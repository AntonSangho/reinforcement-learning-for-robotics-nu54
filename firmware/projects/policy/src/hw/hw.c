#include "hw.h"
#include <zephyr/drivers/hwinfo.h>


// 직전 리셋 원인과 부팅 횟수를 부팅 로그에 남긴다 (세션 16, USB 없이 시험하다 재부팅된 원인 추적).
//   POR/BOR = 전원이 끊겼거나 떨어짐, PIN = 리셋 핀, SOFTWARE = sys_reboot·J-Link, LOCKUP/WATCHDOG = 펌웨어 오류
//   읽은 뒤 지워서 다음 부팅에는 그 사이의 원인만 보이게 한다. `log list` 로 다시 볼 수 있다.
static void hwLogResetCause(void)
{
  static const struct { uint32_t bit; const char *name; } tbl[] =
  {
    {RESET_PIN, "PIN"}, {RESET_SOFTWARE, "SOFTWARE"}, {RESET_BROWNOUT, "BROWNOUT"},
    {RESET_POR, "POR"}, {RESET_WATCHDOG, "WATCHDOG"}, {RESET_DEBUG, "DEBUG"},
    {RESET_SECURITY, "SECURITY"}, {RESET_LOW_POWER_WAKE, "LOW_POWER_WAKE"},
    {RESET_CPU_LOCKUP, "CPU_LOCKUP"}, {RESET_PARITY, "PARITY"}, {RESET_PLL, "PLL"},
    {RESET_CLOCK, "CLOCK"}, {RESET_HARDWARE, "HARDWARE"}, {RESET_USER, "USER"},
    {RESET_TEMPERATURE, "TEMPERATURE"},
  };
  uint32_t cause = 0;
  uint32_t boot_cnt = 0;

  if (hwinfo_get_reset_cause(&cause) == 0)
  {
    hwinfo_clear_reset_cause();
  }
  logPrintf("Reset cause   \t\t: 0x%X", (unsigned)cause);
  if (cause == 0) logPrintf(" (none = power on)");
  for (int i=0; i<(int)ARRAY_SIZE(tbl); i++)
  {
    if (cause & tbl[i].bit) logPrintf(" %s", tbl[i].name);
  }
  logPrintf("\r\n");

  nvsGet("boot_cnt", &boot_cnt, sizeof(boot_cnt));
  boot_cnt++;
  nvsSet("boot_cnt", &boot_cnt, sizeof(boot_cnt));
  logPrintf("Boot count    \t\t: %u\r\n", (unsigned)boot_cnt);
}



bool hwInit(void)
{
  bspInit();

  cliInit();
  logInit();
  uartInit();
  for (int i=0; i<HW_UART_MAX_CH; i++)
  {
    uartOpen(i, 115200);
  }

  logOpen(HW_LOG_CH, 115200);
  logPrintf("\r\n[ Firmware Begin... ]\r\n");
  logPrintf("Booting..Name \t\t: %s\r\n", _DEF_BOARD_NAME);
  logPrintf("Booting..Ver  \t\t: %s\r\n", _DEF_FIRMWATRE_VERSION);
  logPrintf("Booting..Date \t\t: %s\r\n", __DATE__);
  logPrintf("Booting..Time \t\t: %s\r\n", __TIME__);
  logPrintf("Board         \t\t: %s\r\n", CONFIG_BOARD_TARGET);
  logPrintf("\n");

  buttonInit();
  nvsInit();
  hwLogResetCause();
  i2cInit();
  motorInit();
  encInit();
  imuInit();

  return true;
}
