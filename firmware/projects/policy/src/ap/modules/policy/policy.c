/*
 * policy.c — 학습한 PPO 정책(actor)으로 균형 잡기 (Part 3)
 *
 * 5 ms 마다 관측을 만들어 actor 에 넣고, 나온 행동을 모터 duty 로 낸다.
 * 원본 펌웨어 workspace/software/03-sim-to-real/balance_bot/balance_bot.ino 의 loop() 와 같은 순서.
 *
 *   관측 [pitch, pitch_rate, wheel_vel_left, wheel_vel_right]  (시뮬레이션 _get_obs() 와 같은 순서·단위)
 *     pitch        상보필터, alpha 0.99, dt 는 POLICY_DT 로 고정 (학습 env 와 같은 식)
 *     pitch_rate   자이로 Y [rad/s]
 *     wheel_vel    5 ms 동안의 엔코더 틱 차이 → rad/s
 *   행동 [left, right] → [-1, 1] 로 자른 뒤 x POLICY_CTRL_TO_DUTY_X10 (ctrl 1 = duty 60 %)
 *
 * 축·부호·좌우 (docs/hardware.md §4, §5-6) — 사용자 실측
 *   pitch = -atan2(ax, az), pitch_rate = +gy  (2026-11-17)
 *   모터 A(ch0) = 왼쪽, 모터 B(ch1) = 오른쪽, + duty = 앞으로, 엔코더도 앞으로 + (2026-11-23)
 *
 * 시작 (USB 없이) : SW1 또는 외부 버튼(SW2 핀 P1.09, P4-7)을 누르면 POLICY_BTN_DELAY_MS 뒤
 *   |pitch| < 10° 이면 safety 를 풀고 RUN. RUN 중에 누르면 멈춘다.
 *   RUN 동안 50 ms 마다 RAM 에 기록 → 나중에 `policy dump`
 *   상태 LED (P2.09, P2-23) : RUN 이면 켜짐, 아니면 꺼짐 (safety trip·넘어짐으로 멈춰도 꺼진다)
 *
 * 모드
 *   OFF  스레드가 잠듦
 *   DRY  관측·추론만 하고 모터는 0 (sim ↔ real 관측 비교, 추론 시간 측정용)
 *   RUN  모터 구동. safety trip 이나 |pitch| > POLICY_TIP_RAD 면 모터를 세우고 OFF 로
 *        원본처럼 다시 세우면 저절로 시작하지는 않는다 (`policy start` 로 다시 시작)
 */

#include "policy.h"


#ifdef _USE_HW_MOTOR
#include <math.h>
#include <zephyr/drivers/gpio.h>
#include "policy_actor.h"


#define POLICY_PERIOD_MS          5
#define POLICY_DT                 0.005f      // 학습 MJCF timestep (nu54.xml)
#define POLICY_ALPHA              0.99f       // 학습 env 의 alpha
#define POLICY_ENC_TICKS_PER_REV  1496.0f     // 11 PPR x4 x 34
#define POLICY_CTRL_TO_DUTY_X10   600         // ctrl 1.0 = duty 60.0 % (nu54.xml gear 0.165 의 기준)
#define POLICY_TIP_RAD            0.79f       // 원본 TIP_THRESHOLD (45°), safety 와 같은 기준
#define POLICY_VEL_AVG_MAX        20          // 바퀴 속도 평균 창 최대 (주기 수, 100 ms)

#define POLICY_BTN                _DEF_BUTTON1      // SW1 (P1.13)
#define POLICY_BTN_EXT            _DEF_BUTTON2      // 외부 버튼 (SW2 핀 P1.09, P4-7). COM = GND, NO = P1.09
#define POLICY_BTN_DELAY_MS       2000        // 버튼을 누른 뒤 세워 잡을 시간
#define POLICY_BTN_START_DEG      10.0f       // 시작할 때 허용하는 |pitch| (safety 가속도 각도)

#define POLICY_LOG_DIV            10          // 10 step (50 ms) 마다 기록
#define POLICY_LOG_MAX            1200        // 50 ms x 1200 = 60 s

#define POLICY_MOTOR_LEFT         0           // 모터 A
#define POLICY_MOTOR_RIGHT        1           // 모터 B
#define POLICY_ENC_LEFT           0
#define POLICY_ENC_RIGHT          1

#define DEG_TO_RAD                0.017453293f
#define TWO_PI                    6.283185307f


typedef enum
{
  MODE_OFF = 0,
  MODE_DRY,
  MODE_RUN,
} policy_mode_t;

static const char *mode_str[] = {"off", "dry", "run"};

// 바퀴 속도 관측을 만드는 방법 (떨림 가설 시험, 2026-11-23 세션 15)
//   raw  5 ms 틱 차이 (원본·학습과 같은 주기). 틱 1개 = 0.84 rad/s
//   avg  최근 vel_avg_n 주기의 틱 합 / (n x 5 ms). 계단이 잘게 쪼개지는 대신 늦어진다
//   zero 정책에 0 을 넣는다 (watch 에는 raw 값을 그대로 보인다)
typedef enum
{
  VEL_RAW = 0,
  VEL_AVG,
  VEL_ZERO,
} vel_mode_t;

static const char *vel_str[] = {"raw", "avg", "zero"};

typedef struct
{
  float    obs[POLICY_OBS_SIZE];                // 정책에 넣은 값
  float    vel_raw[2];                        // 5 ms 틱 차이로 구한 바퀴 속도
  float    action[POLICY_ACTION_SIZE];        // 자르기 전 actor 출력
  int32_t  duty_x10[POLICY_ACTION_SIZE];
  float    accel_pitch;
  uint32_t step;
} policy_snap_t;

typedef struct
{
  uint32_t step;
  uint32_t overrun;                           // 5 ms 안에 못 끝나 다음 주기를 놓친 횟수
  uint32_t imu_fail;
  uint32_t period_us_min;
  uint32_t period_us_max;
  uint32_t infer_us_max;
  uint64_t infer_us_sum;
  uint32_t work_us_max;                       // IMU 읽기 + 계산 + 모터 출력
} policy_stat_t;

static volatile policy_mode_t mode      = MODE_OFF;
static volatile uint8_t       model_sel = 2;
static volatile vel_mode_t    vel_mode  = VEL_AVG;      // 세션 15 떨림 시험 뒤 기본값 (raw 는 떨림)
static volatile uint8_t       vel_avg_n = 4;
static const char * volatile  stop_reason = "-";

static policy_snap_t   snap;
static policy_stat_t   stat;
static struct k_spinlock snap_lock;

// RUN 기록 (int16 로 줄여 RAM 17 KB)
typedef struct
{
  int16_t pitch_mrad;
  int16_t rate_mrad;
  int16_t vel_x100[2];        // 정책에 넣은 값
  int16_t raw_x100[2];
  int16_t duty_x10[2];
} policy_log_t;

static policy_log_t log_buf[POLICY_LOG_MAX];
static uint32_t     log_cnt   = 0;              // 지금까지 쓴 개수 (POLICY_LOG_MAX 를 넘으면 앞을 덮음)
static uint32_t     log_start = 0;              // 기록을 시작한 step (dump 의 ms 계산)

static const struct gpio_dt_spec status_led = GPIO_DT_SPEC_GET(DT_PATH(zephyr_user), status_led_gpios);

static K_SEM_DEFINE(start_sem, 0, 1);
static K_THREAD_STACK_DEFINE(btn_stack, 1024);
static struct k_thread btn_thread;
static void policyBtnThread(void *arg1, void *arg2, void *arg3);
static K_THREAD_STACK_DEFINE(policy_stack, _HW_DEF_RTOS_THREAD_MEM_POLICY);
static struct k_thread policy_thread;

static void policyThread(void *arg1, void *arg2, void *arg3);
#ifdef _USE_HW_CLI
static void cliPolicy(cli_args_t *args);
#endif




bool policyInit(void)
{
  k_tid_t tid;

  tid = k_thread_create(&policy_thread, policy_stack, K_THREAD_STACK_SIZEOF(policy_stack),
                        policyThread, NULL, NULL, NULL,
                        _HW_DEF_RTOS_THREAD_PRI_POLICY, K_FP_REGS, K_NO_WAIT);
  k_thread_name_set(tid, "policy");

  tid = k_thread_create(&btn_thread, btn_stack, K_THREAD_STACK_SIZEOF(btn_stack),
                        policyBtnThread, NULL, NULL, NULL,
                        _HW_DEF_RTOS_THREAD_PRI_CLI + 1, 0, K_NO_WAIT);
  k_thread_name_set(tid, "policy_btn");

#ifdef _USE_HW_CLI
  cliAdd("policy", cliPolicy);
#endif

  return tid != NULL;
}

static void actorForward(uint8_t sel, const float *obs, float *action)
{
  if (sel == 1) actorPhase1Forward(obs, action);
  else          actorPhase2Forward(obs, action);
}

static inline uint32_t cycToUs(uint32_t cyc)
{
  return (uint32_t)k_cyc_to_us_floor64(cyc);
}

static float clampf(float v, float lo, float hi)
{
  return v < lo ? lo : (v > hi ? hi : v);
}

static void policyStop(const char *reason)
{
  mode = MODE_OFF;
  motorSetX10(POLICY_MOTOR_LEFT, 0);
  motorSetX10(POLICY_MOTOR_RIGHT, 0);
  stop_reason = reason;
}

static bool policyStart(policy_mode_t new_mode)
{
  if (mode != MODE_OFF) return false;

  log_cnt   = 0;
  log_start = 0;
  memset(&stat, 0, sizeof(stat));
  stat.period_us_min = UINT32_MAX;
  stop_reason = "-";
  mode        = new_mode;
  k_sem_give(&start_sem);
  return true;
}

void policyThread(void *arg1, void *arg2, void *arg3)
{
  moduleWaitReady();

  while (1)
  {
    // 시작할 때마다 시뮬레이션 reset() 과 같이 pitch 를 0 에서 시작한다.
    float    pitch     = 0.0f;
    int32_t  prev_l    = encGetCount(POLICY_ENC_LEFT);
    int32_t  prev_r    = encGetCount(POLICY_ENC_RIGHT);
    uint32_t prev_cyc  = k_cycle_get_32();
    int64_t  next      = k_uptime_get();
    bool     first     = true;
    int32_t  hist_l[POLICY_VEL_AVG_MAX + 1];    // 최근 엔코더 값 (hist[0] = 가장 최근)
    int32_t  hist_r[POLICY_VEL_AVG_MAX + 1];
    uint32_t hist_cnt  = 0;

    k_sem_take(&start_sem, K_FOREVER);

    prev_l   = encGetCount(POLICY_ENC_LEFT);
    prev_r   = encGetCount(POLICY_ENC_RIGHT);
    prev_cyc = k_cycle_get_32();
    next     = k_uptime_get();
    for (int i=0; i<=POLICY_VEL_AVG_MAX; i++)
    {
      hist_l[i] = prev_l;
      hist_r[i] = prev_r;
    }
    hist_cnt = 1;

    while (mode != MODE_OFF)
    {
      uint32_t   t0 = k_cycle_get_32();
      imu_data_t d;
      float      obs[POLICY_OBS_SIZE];
      float      action[POLICY_ACTION_SIZE];
      int32_t    duty_x10[POLICY_ACTION_SIZE] = {0, 0};
      float      accel_pitch;

      // 실제 주기 (학습은 POLICY_DT 로 고정)
      if (!first)
      {
        uint32_t period = cycToUs(t0 - prev_cyc);

        if (period < stat.period_us_min) stat.period_us_min = period;
        if (period > stat.period_us_max) stat.period_us_max = period;
      }
      prev_cyc = t0;
      first    = false;

      if (!(imuUpdate() && imuGetData(&d)))
      {
        stat.imu_fail++;
        if (mode == MODE_RUN)
        {
          motorSetX10(POLICY_MOTOR_LEFT, 0);
          motorSetX10(POLICY_MOTOR_RIGHT, 0);
        }
        goto next_step;
      }

      // 관측 (학습 env _get_obs() 와 같은 식)
      float pitch_rate = ((float)d.gy_mdps / 1000.0f) * DEG_TO_RAD;

      accel_pitch = -atan2f((float)d.ax_mg, (float)d.az_mg);
      pitch       = POLICY_ALPHA * (pitch + pitch_rate * POLICY_DT) + (1.0f - POLICY_ALPHA) * accel_pitch;

      int32_t enc_l = encGetCount(POLICY_ENC_LEFT);
      int32_t enc_r = encGetCount(POLICY_ENC_RIGHT);

      for (int i=POLICY_VEL_AVG_MAX; i>0; i--)
      {
        hist_l[i] = hist_l[i - 1];
        hist_r[i] = hist_r[i - 1];
      }
      hist_l[0] = enc_l;
      hist_r[0] = enc_r;
      if (hist_cnt < POLICY_VEL_AVG_MAX) hist_cnt++;

      float vel_raw_l = (float)(enc_l - prev_l) * (TWO_PI / POLICY_ENC_TICKS_PER_REV) / POLICY_DT;
      float vel_raw_r = (float)(enc_r - prev_r) * (TWO_PI / POLICY_ENC_TICKS_PER_REV) / POLICY_DT;

      prev_l = enc_l;
      prev_r = enc_r;

      obs[0] = pitch;
      obs[1] = pitch_rate;
      if (vel_mode == VEL_AVG)
      {
        // 시작 직후에는 쌓인 만큼만 쓴다
        uint32_t n = vel_avg_n < hist_cnt ? vel_avg_n : hist_cnt;

        obs[2] = (float)(hist_l[0] - hist_l[n]) * (TWO_PI / POLICY_ENC_TICKS_PER_REV) / (POLICY_DT * (float)n);
        obs[3] = (float)(hist_r[0] - hist_r[n]) * (TWO_PI / POLICY_ENC_TICKS_PER_REV) / (POLICY_DT * (float)n);
      }
      else if (vel_mode == VEL_ZERO)
      {
        obs[2] = 0.0f;
        obs[3] = 0.0f;
      }
      else
      {
        obs[2] = vel_raw_l;
        obs[3] = vel_raw_r;
      }

      // 추론
      uint32_t ti = k_cycle_get_32();
      actorForward(model_sel, obs, action);
      uint32_t infer = cycToUs(k_cycle_get_32() - ti);

      stat.infer_us_sum += infer;
      if (infer > stat.infer_us_max) stat.infer_us_max = infer;

      for (int i=0; i<POLICY_ACTION_SIZE; i++)
      {
        duty_x10[i] = (int32_t)lroundf(clampf(action[i], -1.0f, 1.0f) * POLICY_CTRL_TO_DUTY_X10);
      }

      // 모터
      if (mode == MODE_RUN)
      {
        if (safetyIsTrip())
        {
          policyStop("safety trip");
        }
        else if (fabsf(pitch) > POLICY_TIP_RAD)
        {
          policyStop("tip");
        }
        else
        {
          motorSetX10(POLICY_MOTOR_LEFT,  duty_x10[0]);
          motorSetX10(POLICY_MOTOR_RIGHT, duty_x10[1]);
        }
      }

      {
        k_spinlock_key_t key = k_spin_lock(&snap_lock);

        memcpy(snap.obs, obs, sizeof(obs));
        snap.vel_raw[0] = vel_raw_l;
        snap.vel_raw[1] = vel_raw_r;
        memcpy(snap.action, action, sizeof(action));
        memcpy(snap.duty_x10, duty_x10, sizeof(duty_x10));
        snap.accel_pitch = accel_pitch;
        snap.step        = stat.step;
        k_spin_unlock(&snap_lock, key);
      }

      if (mode == MODE_RUN && (stat.step % POLICY_LOG_DIV) == 0)
      {
        policy_log_t *p = &log_buf[log_cnt % POLICY_LOG_MAX];

        if (log_cnt == 0) log_start = stat.step;
        p->pitch_mrad  = (int16_t)lroundf(obs[0] * 1000.0f);
        p->rate_mrad   = (int16_t)clampf(obs[1] * 1000.0f, -32000.0f, 32000.0f);
        p->vel_x100[0] = (int16_t)lroundf(obs[2] * 100.0f);
        p->vel_x100[1] = (int16_t)lroundf(obs[3] * 100.0f);
        p->raw_x100[0] = (int16_t)lroundf(vel_raw_l * 100.0f);
        p->raw_x100[1] = (int16_t)lroundf(vel_raw_r * 100.0f);
        p->duty_x10[0] = (int16_t)duty_x10[0];
        p->duty_x10[1] = (int16_t)duty_x10[1];
        log_cnt++;
      }

next_step:
      stat.step++;
      {
        uint32_t work = cycToUs(k_cycle_get_32() - t0);

        if (work > stat.work_us_max) stat.work_us_max = work;
      }

      next += POLICY_PERIOD_MS;
      if (k_uptime_get() >= next)
      {
        // 늦었다. 밀린 주기를 따라잡으려 연달아 돌지 않고 지금부터 다시 센다.
        stat.overrun++;
        next = k_uptime_get();
      }
      k_sleep(K_TIMEOUT_ABS_MS(next));
    }

    if (mode == MODE_OFF)
    {
      logPrintf("[  ] policy stop : %s\n", stop_reason);
    }
  }
}




// SW1·외부 버튼 : 꺼져 있으면 POLICY_BTN_DELAY_MS 뒤 시작, 돌고 있으면 정지. USB 없이 시험할 때 쓴다.
// 상태 LED 도 여기서 20 ms 마다 mode 에 맞춘다 (어디서 멈췄든 따라간다).
void policyBtnThread(void *arg1, void *arg2, void *arg3)
{
  bool prev = false;
  bool led_ok;

  led_ok = gpio_is_ready_dt(&status_led) &&
           gpio_pin_configure_dt(&status_led, GPIO_OUTPUT_INACTIVE) == 0;
  logPrintf("[%s] policy status led (P2.09)\n", led_ok ? "OK" : "E_");

  moduleWaitReady();

  while (1)
  {
    bool cur = buttonGetPressed(POLICY_BTN) || buttonGetPressed(POLICY_BTN_EXT);

    if (cur && !prev)
    {
      if (mode != MODE_OFF)
      {
        policyStop("button");
      }
      else
      {
        k_msleep(POLICY_BTN_DELAY_MS);
        if (fabsf(safetyGetPitch()) < POLICY_BTN_START_DEG)
        {
          safetyReset();
          k_msleep(50);       // safety 가 다시 trip 하는지 한 번 본다
          if (!safetyIsTrip())
          {
            policyStart(MODE_RUN);
            logPrintf("[  ] policy start : button, phase%d\n", model_sel);
          }
        }
        else
        {
          stop_reason = "button: tilted";
          logPrintf("[  ] policy button : tilted %d deg\n", (int)safetyGetPitch());
        }
      }
    }
    prev = cur;

    if (led_ok)
    {
      gpio_pin_set_dt(&status_led, mode == MODE_RUN);
    }
    k_msleep(20);
  }
}




#ifdef _USE_HW_CLI

// 부호가 남도록 정수 + 소수 자릿수로 찍는다 (printf 에 float 지원 없음).
static const char *fmtF(char *buf, size_t len, float v, int digits)
{
  int32_t scale = 1;

  for (int i=0; i<digits; i++) scale *= 10;

  int32_t t = (int32_t)lroundf(v * (float)scale);
  int32_t a = t < 0 ? -t : t;

  if (digits == 0)
    snprintf(buf, len, "%s%d", t < 0 ? "-" : "", (int)a);
  else
    snprintf(buf, len, "%s%d.%0*d", t < 0 ? "-" : "", (int)(a / scale), digits, (int)(a % scale));
  return buf;
}

static void getSnap(policy_snap_t *p)
{
  k_spinlock_key_t key = k_spin_lock(&snap_lock);

  *p = snap;
  k_spin_unlock(&snap_lock, key);
}

static void printSnapHeader(void)
{
  // vel_l/r : 정책에 넣은 값, raw_l/r : 5 ms 틱 차이
  cliPrintf("%8s %8s %8s %8s %8s %6s %6s %8s %8s %6s %6s\n",
            "ms", "pitch", "rate", "vel_l", "vel_r", "raw_l", "raw_r", "act_l", "act_r", "duty_l", "duty_r");
}

static void printSnap(const policy_snap_t *p)
{
  char b[10][16];

  cliPrintf("%8u %8s %8s %8s %8s %6s %6s %8s %8s %6s %6s\n",
            (unsigned)(p->step * POLICY_PERIOD_MS),
            fmtF(b[0], 16, p->obs[0], 3),
            fmtF(b[1], 16, p->obs[1], 3),
            fmtF(b[2], 16, p->obs[2], 2),
            fmtF(b[3], 16, p->obs[3], 2),
            fmtF(b[8], 16, p->vel_raw[0], 2),
            fmtF(b[9], 16, p->vel_raw[1], 2),
            fmtF(b[4], 16, p->action[0], 3),
            fmtF(b[5], 16, p->action[1], 3),
            fmtF(b[6], 16, (float)p->duty_x10[0] / 10.0f, 1),
            fmtF(b[7], 16, (float)p->duty_x10[1] / 10.0f, 1));
}

static void printInfo(void)
{
  policy_snap_t s;
  char b[16];

  getSnap(&s);
  cliPrintf("model phase%d, mode %s, last stop %s\n", model_sel, mode_str[mode], stop_reason);
  if (vel_mode == VEL_AVG) cliPrintf("vel avg %d (%d ms)\n", vel_avg_n, vel_avg_n * POLICY_PERIOD_MS);
  else                     cliPrintf("vel %s\n", vel_str[vel_mode]);
  cliPrintf("step %u, overrun %u, imu fail %u\n", (unsigned)stat.step, (unsigned)stat.overrun, (unsigned)stat.imu_fail);
  cliPrintf("period %u ~ %u us, work max %u us\n",
            (unsigned)(stat.period_us_min == UINT32_MAX ? 0 : stat.period_us_min),
            (unsigned)stat.period_us_max, (unsigned)stat.work_us_max);
  cliPrintf("infer avg %s us, max %u us\n",
            fmtF(b, sizeof(b), stat.step ? (float)stat.infer_us_sum / (float)stat.step : 0.0f, 1),
            (unsigned)stat.infer_us_max);
  cliPrintf("accel pitch %s rad\n", fmtF(b, sizeof(b), s.accel_pitch, 3));
  printSnapHeader();
  printSnap(&s);
}

// 정지 상태 추론 시험 : ONNX(onnxruntime) 로 구한 값과 비교한다 (2026-11-23, 세션 15).
static void runTest(uint32_t repeat)
{
  static const float test_obs[5][POLICY_OBS_SIZE] = {
    { 0.00f,  0.0f,  0.0f, 0.0f},
    { 0.10f,  0.0f,  0.0f, 0.0f},
    {-0.10f,  0.0f,  0.0f, 0.0f},
    { 0.05f,  0.5f,  1.0f, 1.0f},
    { 0.00f, -0.3f, -2.0f, 3.0f},
  };
  static const float ref[2][5][POLICY_ACTION_SIZE] = {
    {{ 0.04430f,  0.08889f}, { 0.27906f,  0.36005f}, {-0.19263f, -0.18759f},
     { 0.63721f,  0.74041f}, {-0.08843f, -0.04331f}},
    {{ 0.01931f,  0.02575f}, { 0.32094f,  0.29906f}, {-0.27946f, -0.24170f},
     { 0.59209f,  0.59308f}, { 0.41244f, -0.45133f}},
  };

  for (uint8_t sel = 1; sel <= 2; sel++)
  {
    float    max_err = 0.0f;
    char     b[4][16];

    cliPrintf("phase%d : obs -> action (onnx)\n", sel);
    for (int i=0; i<5; i++)
    {
      float a[POLICY_ACTION_SIZE];

      actorForward(sel, test_obs[i], a);
      for (int k=0; k<POLICY_ACTION_SIZE; k++)
      {
        float e = fabsf(a[k] - ref[sel - 1][i][k]);

        if (e > max_err) max_err = e;
      }
      cliPrintf("  %d : %8s %8s  (%8s %8s)\n", i,
                fmtF(b[0], 16, a[0], 5), fmtF(b[1], 16, a[1], 5),
                fmtF(b[2], 16, ref[sel - 1][i][0], 5), fmtF(b[3], 16, ref[sel - 1][i][1], 5));
    }

    float    a[POLICY_ACTION_SIZE];
    uint32_t t0 = k_cycle_get_32();

    for (uint32_t r=0; r<repeat; r++)
    {
      actorForward(sel, test_obs[r % 5], a);
    }
    uint32_t us = cycToUs(k_cycle_get_32() - t0);

    cliPrintf("  max err %s, %u runs %u us -> %s us/run\n",
              fmtF(b[0], 16, max_err, 6), (unsigned)repeat, (unsigned)us,
              fmtF(b[1], 16, (float)us / (float)repeat, 2));
  }
}

void cliPolicy(cli_args_t *args)
{
  bool ret = false;


  if (args->argc == 1 && args->isStr(0, "info") == true)
  {
    printInfo();
    ret = true;
  }

  if (args->argc >= 1 && args->isStr(0, "test") == true)
  {
    runTest(args->argc == 2 ? (uint32_t)args->getData(1) : 1000);
    ret = true;
  }

  if (args->argc == 2 && args->isStr(0, "sel") == true)
  {
    int32_t sel = args->getData(1);

    if (mode != MODE_OFF)
    {
      cliPrintf("stop first\n");
    }
    else if (sel == 1 || sel == 2)
    {
      model_sel = (uint8_t)sel;
      cliPrintf("model phase%d\n", model_sel);
    }
    else
    {
      cliPrintf("sel 1|2\n");
    }
    ret = true;
  }

  // policy vel raw|avg n|zero : 바퀴 속도 관측을 만드는 방법
  if (args->argc >= 2 && args->isStr(0, "vel") == true)
  {
    if (args->isStr(1, "raw") == true)
    {
      vel_mode = VEL_RAW;
    }
    else if (args->isStr(1, "zero") == true)
    {
      vel_mode = VEL_ZERO;
    }
    else if (args->isStr(1, "avg") == true)
    {
      int32_t n = args->argc >= 3 ? args->getData(2) : 4;

      if (n < 1) n = 1;
      if (n > POLICY_VEL_AVG_MAX) n = POLICY_VEL_AVG_MAX;
      vel_avg_n = (uint8_t)n;
      vel_mode  = VEL_AVG;
    }
    cliPrintf("vel %s", vel_str[vel_mode]);
    if (vel_mode == VEL_AVG) cliPrintf(" %d (%d ms)", vel_avg_n, vel_avg_n * POLICY_PERIOD_MS);
    cliPrintf("\n");
    ret = true;
  }

  if (args->argc == 1 && args->isStr(0, "dry") == true)
  {
    cliPrintf(policyStart(MODE_DRY) ? "policy dry (motor off), phase%d\n" : "already running\n", model_sel);
    ret = true;
  }

  if (args->argc == 1 && args->isStr(0, "start") == true)
  {
    if (safetyIsTrip())
    {
      cliPrintf("safety trip, `safety reset` first\n");
    }
    else
    {
      cliPrintf(policyStart(MODE_RUN) ? "policy RUN, phase%d\n" : "already running\n", model_sel);
    }
    ret = true;
  }

  if (args->argc == 1 && args->isStr(0, "stop") == true)
  {
    if (mode != MODE_OFF) policyStop("cli");
    cliPrintf("policy stop\n");
    ret = true;
  }

  // policy dump : 마지막 RUN 기록 (50 ms 간격, 최대 60 s). 버튼으로 USB 없이 돌린 뒤 읽는다
  if (args->argc == 1 && args->isStr(0, "dump") == true)
  {
    uint32_t n     = log_cnt < POLICY_LOG_MAX ? log_cnt : POLICY_LOG_MAX;
    uint32_t first = log_cnt - n;
    char     b[8][16];

    cliPrintf("log %u (of %u), model phase%d, last stop %s\n", (unsigned)n, (unsigned)log_cnt, model_sel, stop_reason);
    cliPrintf("%8s %8s %8s %6s %6s %6s %6s %6s %6s\n", "ms", "pitch", "rate", "vel_l", "vel_r", "raw_l", "raw_r", "duty_l", "duty_r");
    for (uint32_t i=first; i<log_cnt; i++)
    {
      policy_log_t *p = &log_buf[i % POLICY_LOG_MAX];

      cliPrintf("%8u %8s %8s %6s %6s %6s %6s %6s %6s\n",
                (unsigned)(i * POLICY_LOG_DIV * POLICY_PERIOD_MS),
                fmtF(b[0], 16, p->pitch_mrad / 1000.0f, 3),
                fmtF(b[1], 16, p->rate_mrad / 1000.0f, 3),
                fmtF(b[2], 16, p->vel_x100[0] / 100.0f, 2),
                fmtF(b[3], 16, p->vel_x100[1] / 100.0f, 2),
                fmtF(b[4], 16, p->raw_x100[0] / 100.0f, 2),
                fmtF(b[5], 16, p->raw_x100[1] / 100.0f, 2),
                fmtF(b[6], 16, p->duty_x10[0] / 10.0f, 1),
                fmtF(b[7], 16, p->duty_x10[1] / 10.0f, 1));
    }
    cliPrintf("end\n");
    ret = true;
  }

  // policy watch time_ms [interval_ms] : 실행 중 관측·행동을 주기적으로 찍는다 (cli 를 그동안 막음)
  if (args->argc >= 2 && args->isStr(0, "watch") == true)
  {
    uint32_t time_ms  = (uint32_t)args->getData(1);
    uint32_t interval = args->argc >= 3 ? (uint32_t)args->getData(2) : 100;
    int64_t  end      = k_uptime_get() + time_ms;

    if (interval < 10) interval = 10;
    printSnapHeader();
    while (k_uptime_get() < end)
    {
      policy_snap_t s;

      getSnap(&s);
      printSnap(&s);
      if (mode == MODE_OFF) break;
      k_msleep(interval);
    }
    cliPrintf("mode %s, last stop %s, overrun %u\n", mode_str[mode], stop_reason, (unsigned)stat.overrun);
    ret = true;
  }


  if (ret != true)
  {
    cliPrintf("policy info\n");
    cliPrintf("policy test [repeat]\n");
    cliPrintf("policy sel 1|2\n");
    cliPrintf("policy vel raw|avg [n]|zero\n");
    cliPrintf("policy dry|start|stop\n");
    cliPrintf("policy watch time_ms [interval_ms]\n");
    cliPrintf("policy dump\n");
    cliPrintf("SW1 / ext button (P1.09) : start (after 2 s) / stop, LED (P2.09) on = run\n");
  }
}

#endif


MODULE_DEF(policy){
  .name     = "policy",
  .priority = MODULE_PRI_NORMAL,
  .init     = policyInit,
};

#endif
