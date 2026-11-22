/*
 * safety.c — 넘어짐 감지 모듈
 *
 * 10 ms 마다 IMU 를 읽어 가속도로 pitch 를 구한다. |pitch| 가 SAFETY_TIP_DEG 를
 * SAFETY_TIP_COUNT 번 연속으로 넘거나, IMU 를 SAFETY_IMU_FAIL_MAX 번 연속으로 못 읽으면
 * 모터를 세우고 잠근다 (trip). 잠금은 `safety reset` 으로만 푼다 (다시 세운 뒤 사람이 푼다).
 *
 *   pitch = -atan2(ax, az)  (docs/hardware.md §4, 2026-11-17 섀시 장착 측정, 시뮬레이션 식과 같음)
 *   가속도만 쓰므로 바퀴가 가속하면 값이 흔들린다. 45° 는 정책이 쓸 각도보다 훨씬 커서
 *   넘어짐 판정에는 충분하다. 균형 제어용 pitch 는 Part 3 에서 자이로와 합친다.
 *   원본 펌웨어(03-sim-to-real/balance_bot.ino)의 TIP_THRESHOLD 0.79 rad 와 같은 기준.
 */

#include "safety.h"


#ifdef _USE_HW_MOTOR
#include <math.h>


#define SAFETY_PERIOD_MS        10
#define SAFETY_TIP_DEG          45.0f
#define SAFETY_TIP_COUNT        3           // 3 x 10 ms 연속 (한 번 튀는 값으로 서지 않도록)
#define SAFETY_IMU_FAIL_MAX     10          // 100 ms 동안 못 읽으면 넘어진 것과 같이 다룬다

#define RAD_TO_DEG              57.29578f


typedef enum
{
  TRIP_NONE = 0,
  TRIP_TIP,
  TRIP_IMU,
} trip_reason_t;

static const char *trip_str[] = {"none", "tip", "imu"};

static volatile bool          is_enable   = true;
static volatile bool          is_trip     = false;
static volatile trip_reason_t trip_reason = TRIP_NONE;
static volatile float         pitch_deg   = 0.0f;
static volatile float         trip_pitch  = 0.0f;
static volatile uint32_t      trip_count  = 0;

static void safetyThread(void *arg1, void *arg2, void *arg3);
static void safetyTrip(trip_reason_t reason);
#ifdef _USE_HW_CLI
static void cliSafety(cli_args_t *args);
#endif

static K_THREAD_STACK_DEFINE(safety_stack, _HW_DEF_RTOS_THREAD_MEM_SAFETY);
static struct k_thread safety_thread;




bool safetyInit(void)
{
  k_tid_t tid;

  tid = k_thread_create(&safety_thread, safety_stack, K_THREAD_STACK_SIZEOF(safety_stack),
                        safetyThread, NULL, NULL, NULL,
                        _HW_DEF_RTOS_THREAD_PRI_SAFETY, 0, K_NO_WAIT);
  k_thread_name_set(tid, "safety");

#ifdef _USE_HW_CLI
  cliAdd("safety", cliSafety);
#endif

  return tid != NULL;
}

bool safetyIsTrip(void)
{
  return is_trip;
}

float safetyGetPitch(void)
{
  return pitch_deg;
}

void safetyTrip(trip_reason_t reason)
{
  motorLock(true);

  if (!is_trip)
  {
    is_trip     = true;
    trip_reason = reason;
    trip_pitch  = pitch_deg;
    trip_count++;
    logPrintf("[!!] safety trip : %s, pitch %d deg\n", trip_str[reason], (int)trip_pitch);
  }
}

void safetyThread(void *arg1, void *arg2, void *arg3)
{
  uint32_t tip_cnt  = 0;
  uint32_t fail_cnt = 0;
  int64_t  next     = k_uptime_get();

  moduleWaitReady();

  while (1)
  {
    imu_data_t d;

    if (imuUpdate() && imuGetData(&d))
    {
      fail_cnt  = 0;
      pitch_deg = -atan2f((float)d.ax_mg, (float)d.az_mg) * RAD_TO_DEG;
      tip_cnt   = (fabsf(pitch_deg) > SAFETY_TIP_DEG) ? tip_cnt + 1 : 0;
    }
    else
    {
      fail_cnt++;
    }

    if (is_enable)
    {
      if (tip_cnt  >= SAFETY_TIP_COUNT)    safetyTrip(TRIP_TIP);
      if (fail_cnt >= SAFETY_IMU_FAIL_MAX) safetyTrip(TRIP_IMU);
    }

    next += SAFETY_PERIOD_MS;
    k_sleep(K_TIMEOUT_ABS_MS(next));
  }
}




#ifdef _USE_HW_CLI

// -0.5 같은 값도 부호가 남도록 0.1 deg 단위 정수로 바꿔 찍는다.
static void fmtDeg(char *buf, size_t len, float deg)
{
  int t = (int)lroundf(deg * 10.0f);

  snprintf(buf, len, "%s%d.%d", t < 0 ? "-" : "", abs(t) / 10, abs(t) % 10);
}

static void printState(void)
{
  char p_str[16], t_str[16];

  fmtDeg(p_str, sizeof(p_str), pitch_deg);
  fmtDeg(t_str, sizeof(t_str), trip_pitch);
  cliPrintf("enable %d, trip %d (%s), count %u\n", is_enable, is_trip, trip_str[trip_reason], (unsigned)trip_count);
  cliPrintf("pitch %s deg (trip at %s), limit %d deg, motor max %d %%, lock %d\n",
            p_str, t_str, (int)SAFETY_TIP_DEG, (int)motorGetDutyMax(), motorIsLock());
}

void cliSafety(cli_args_t *args)
{
  bool ret = false;


  if (args->argc == 1 && args->isStr(0, "info") == true)
  {
    printState();
    ret = true;
  }

  // 다시 세운 뒤 잠금을 푼다. 아직 기울어 있으면 30 ms 안에 다시 trip 된다.
  if (args->argc == 1 && args->isStr(0, "reset") == true)
  {
    is_trip     = false;
    trip_reason = TRIP_NONE;
    motorLock(false);
    printState();
    ret = true;
  }

  // 바퀴를 띄운 벤치 시험용. 끄면 넘어져도 모터가 서지 않는다. 리셋하면 다시 켜진다.
  if (args->argc == 1 && args->isStr(0, "off") == true)
  {
    is_enable   = false;
    is_trip     = false;
    trip_reason = TRIP_NONE;
    motorLock(false);
    cliPrintf("safety OFF (bench test only)\n");
    ret = true;
  }

  if (args->argc == 1 && args->isStr(0, "on") == true)
  {
    is_enable = true;
    cliPrintf("safety ON\n");
    ret = true;
  }


  if (ret != true)
  {
    cliPrintf("safety info\n");
    cliPrintf("safety reset\n");
    cliPrintf("safety on|off\n");
  }
}

#endif


MODULE_DEF(safety){
  .name     = "safety",
  .priority = MODULE_PRI_HIGH,
  .init     = safetyInit,
};

#endif
