/*
 * motor.c — TB6612FNG 모터 드라이버 (채널 A, B)
 *
 *   PWM : PWM20 (OUT0 = P1.10 → PWMA, OUT1 = P1.14 → PWMB), 20 kHz
 *   방향 : IN1/IN2 (P2.00~P2.03)
 *     IN1 IN2   동작
 *      1   0    정회전
 *      0   1    역회전
 *      0   0    코스팅 (정지, 공회전)
 *      1   1    브레이크
 *   STBY 는 3.3 V 에 고정한다.
 *
 *   안전 (docs/hardware.md §5) :
 *     duty 상한  : 12 V 스톨 전류(>=1.6 A)가 TB6612 지속 1.2 A 를 넘으므로 HW_MOTOR_DUTY_MAX 로 자른다.
 *                  `motor max n` 으로 바꿀 수 있고 리셋하면 기본값으로 돌아온다.
 *     잠금       : motorLock(true) 이면 모터를 세우고 motorSet() 을 0 으로만 받는다 (safety 모듈이 쓴다).
 */

#include "motor.h"


#ifdef _USE_HW_MOTOR
#include "cli.h"
#include "log.h"
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/pwm.h>


typedef struct
{
  struct pwm_dt_spec  pwm;
  struct gpio_dt_spec in1;
  struct gpio_dt_spec in2;
  int32_t             duty;
} motor_tbl_t;

static int32_t      duty_max = HW_MOTOR_DUTY_MAX;
static volatile bool is_lock = false;

static motor_tbl_t motor_tbl[MOTOR_MAX_CH] = {
  {PWM_DT_SPEC_GET_BY_IDX(DT_PATH(zephyr_user), 0),
   GPIO_DT_SPEC_GET(DT_NODELABEL(ain1), gpios),
   GPIO_DT_SPEC_GET(DT_NODELABEL(ain2), gpios), 0},
  {PWM_DT_SPEC_GET_BY_IDX(DT_PATH(zephyr_user), 1),
   GPIO_DT_SPEC_GET(DT_NODELABEL(bin1), gpios),
   GPIO_DT_SPEC_GET(DT_NODELABEL(bin2), gpios), 0},
};


#ifdef _USE_HW_CLI
static void cliMotor(cli_args_t *args);
#endif


bool motorInit(void)
{
  bool ret = true;

  for (int i=0; i<MOTOR_MAX_CH; i++)
  {
    if (!pwm_is_ready_dt(&motor_tbl[i].pwm)) ret = false;
    if (gpio_pin_configure_dt(&motor_tbl[i].in1, GPIO_OUTPUT_INACTIVE) < 0) ret = false;
    if (gpio_pin_configure_dt(&motor_tbl[i].in2, GPIO_OUTPUT_INACTIVE) < 0) ret = false;
  }

  motorStop();

  logPrintf("[%s] motorInit()\n", ret ? "OK" : "E_");

#ifdef _USE_HW_CLI
  cliAdd("motor", cliMotor);
#endif

  return ret;
}

static int pwm_err[MOTOR_MAX_CH];

static void setPwm(uint8_t ch, uint32_t permille)
{
  motor_tbl_t *p = &motor_tbl[ch];

  pwm_err[ch] = pwm_set_dt(&p->pwm, p->pwm.period, (uint32_t)((uint64_t)p->pwm.period * permille / 1000));
}

// duty_x10 : 0.1 % 단위 (-1000 ~ 1000). 정책 출력(ctrl x 60 %)은 1 % 단위로 자르면
// ctrl 0.017 단위가 되어 작은 행동이 0 으로 사라지므로 0.1 % 단위로 낸다.
bool motorSetX10(uint8_t ch, int32_t duty_x10)
{
  if (ch >= MOTOR_MAX_CH) return false;

  bool ret = true;

  if (is_lock && duty_x10 != 0)
  {
    duty_x10 = 0;
    ret      = false;
  }
  if (duty_x10 >  duty_max * 10) duty_x10 =  duty_max * 10;
  if (duty_x10 < -duty_max * 10) duty_x10 = -duty_max * 10;

  motor_tbl_t *p = &motor_tbl[ch];

  gpio_pin_set_dt(&p->in1, duty_x10 > 0);
  gpio_pin_set_dt(&p->in2, duty_x10 < 0);
  setPwm(ch, (uint32_t)(duty_x10 < 0 ? -duty_x10 : duty_x10));
  p->duty = duty_x10 / 10;

  return ret;
}

bool motorSet(uint8_t ch, int32_t duty)
{
  return motorSetX10(ch, duty * 10);
}

void motorBrake(uint8_t ch)
{
  if (ch >= MOTOR_MAX_CH) return;

  gpio_pin_set_dt(&motor_tbl[ch].in1, 1);
  gpio_pin_set_dt(&motor_tbl[ch].in2, 1);
  setPwm(ch, 1000);
  motor_tbl[ch].duty = 0;
}

void motorStop(void)
{
  for (int i=0; i<MOTOR_MAX_CH; i++)
  {
    motorSet(i, 0);
  }
}

int32_t motorGet(uint8_t ch)
{
  if (ch >= MOTOR_MAX_CH) return 0;

  return motor_tbl[ch].duty;
}

void motorSetDutyMax(int32_t max)
{
  if (max < 0)   max = 0;
  if (max > 100) max = 100;
  duty_max = max;

  // 이미 돌고 있는 모터도 새 상한으로 다시 자른다.
  for (int i=0; i<MOTOR_MAX_CH; i++)
  {
    motorSet(i, motor_tbl[i].duty);
  }
}

int32_t motorGetDutyMax(void)
{
  return duty_max;
}

void motorLock(bool lock)
{
  is_lock = lock;
  if (lock)
  {
    motorStop();
  }
}

bool motorIsLock(void)
{
  return is_lock;
}





#ifdef _USE_HW_CLI

void cliMotor(cli_args_t *args)
{
  bool ret = false;


  if (args->argc == 1 && args->isStr(0, "info") == true)
  {
    for (int i=0; i<MOTOR_MAX_CH; i++)
    {
      cliPrintf("%c : duty %d %%, pwm_err %d, period %u ns\n", 'a' + i, (int)motorGet(i), pwm_err[i], (unsigned)motor_tbl[i].pwm.period);
    }
    cliPrintf("max %d %%, lock %d\n", (int)duty_max, is_lock);
    ret = true;
  }

  if (args->argc == 1 && args->isStr(0, "stop") == true)
  {
    motorStop();
    cliPrintf("stop\n");
    ret = true;
  }

  if (args->argc == 2 && args->isStr(0, "brake") == true)
  {
    uint8_t ch = (args->isStr(1, "b") == true) ? 1 : 0;

    motorBrake(ch);
    cliPrintf("brake %c\n", 'a' + ch);
    ret = true;
  }

  if (args->argc == 1 && args->isStr(0, "max") == true)
  {
    cliPrintf("max %d %%\n", (int)duty_max);
    ret = true;
  }

  if (args->argc == 2 && args->isStr(0, "max") == true)
  {
    motorSetDutyMax(args->getData(1));
    cliPrintf("max %d %%\n", (int)duty_max);
    ret = true;
  }

  // motor a|b duty(-100~100) [time_ms]
  if ((args->argc == 2 || args->argc == 3) && (args->isStr(0, "a") == true || args->isStr(0, "b") == true))
  {
    uint8_t ch = (args->isStr(0, "b") == true) ? 1 : 0;
    int32_t duty = args->getData(1);

    if (motorSet(ch, duty) != true)
    {
      cliPrintf("locked (safety)\n");
    }
    cliPrintf("%c : duty %d %%\n", 'a' + ch, (int)motorGet(ch));

    if (args->argc == 3)
    {
      k_msleep(args->getData(2));
      motorSet(ch, 0);
      cliPrintf("%c : stop\n", 'a' + ch);
    }
    ret = true;
  }


  if (ret != true)
  {
    cliPrintf("motor info\n");
    cliPrintf("motor stop\n");
    cliPrintf("motor brake a|b\n");
    cliPrintf("motor max [0~100]\n");
    cliPrintf("motor a|b duty[-100~100] [time_ms]\n");
  }
}


#endif


#endif
