/*
 * motor.c — TB6612FNG 모터 드라이버 (채널 A, B)
 *
 *   PWM : PWM20 (OUT0 = P1.12 → PWMA, OUT1 = P1.14 → PWMB), 20 kHz
 *   방향 : IN1/IN2 (P2.00~P2.03)
 *     IN1 IN2   동작
 *      1   0    정회전
 *      0   1    역회전
 *      0   0    코스팅 (정지, 공회전)
 *      1   1    브레이크
 *   STBY 는 3.3 V 에 고정한다.
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

static void setPwm(uint8_t ch, uint32_t percent)
{
  motor_tbl_t *p = &motor_tbl[ch];

  pwm_err[ch] = pwm_set_dt(&p->pwm, p->pwm.period, (uint32_t)((uint64_t)p->pwm.period * percent / 100));
}

bool motorSet(uint8_t ch, int32_t duty)
{
  if (ch >= MOTOR_MAX_CH) return false;

  if (duty >  HW_MOTOR_DUTY_MAX) duty =  HW_MOTOR_DUTY_MAX;
  if (duty < -HW_MOTOR_DUTY_MAX) duty = -HW_MOTOR_DUTY_MAX;

  motor_tbl_t *p = &motor_tbl[ch];

  gpio_pin_set_dt(&p->in1, duty > 0);
  gpio_pin_set_dt(&p->in2, duty < 0);
  setPwm(ch, (uint32_t)(duty < 0 ? -duty : duty));
  p->duty = duty;

  return true;
}

void motorBrake(uint8_t ch)
{
  if (ch >= MOTOR_MAX_CH) return;

  gpio_pin_set_dt(&motor_tbl[ch].in1, 1);
  gpio_pin_set_dt(&motor_tbl[ch].in2, 1);
  setPwm(ch, 100);
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

  // motor a|b duty(-100~100) [time_ms]
  if ((args->argc == 2 || args->argc == 3) && (args->isStr(0, "a") == true || args->isStr(0, "b") == true))
  {
    uint8_t ch = (args->isStr(0, "b") == true) ? 1 : 0;
    int32_t duty = args->getData(1);

    motorSet(ch, duty);
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
    cliPrintf("motor a|b duty[-100~100] [time_ms]\n");
  }
}


#endif


#endif
