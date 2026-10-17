/*
 * imu.c — MPU-6050 (GY-521), I2C 0x68
 *
 *   I2C 는 i2c 모듈의 채널 _DEF_I2C1 (i2c21, 헤더 P1.2/P1.3) 을 쓴다.
 *   레지스터 0x3B 부터 14바이트(가속도 XYZ, 온도, 자이로 XYZ)를 한 번에 읽는다.
 *   설정값은 아래 define 에서 바꾼다. 값을 바꾸면 변환 상수도 같이 바꾼다.
 *     DLPF 3        : 자이로 대역폭 42 Hz, 내부 샘플 1 kHz (지연 약 4.8 ms)
 *     SMPLRT_DIV 4  : 1 kHz / (1 + 4) = 200 Hz (출력 레지스터 갱신 주기)
 *     자이로 +-500 dps (65.5 LSB/dps), 가속도 +-2 g (16384 LSB/g)
 *   축 방향과 pitch 부호는 보드를 기울여서 확인한다 (docs/hardware.md §4).
 */

#include "imu.h"


#ifdef _USE_HW_IMU
#include "cli.h"
#include "i2c.h"
#include "log.h"
#include <zephyr/kernel.h>


#define IMU_I2C_CH            _DEF_I2C1
#define IMU_I2C_ADDR          0x68
#define IMU_I2C_TIMEOUT       10

#define REG_SMPLRT_DIV        0x19
#define REG_CONFIG            0x1A
#define REG_GYRO_CONFIG       0x1B
#define REG_ACCEL_CONFIG      0x1C
#define REG_ACCEL_XOUT_H      0x3B
#define REG_PWR_MGMT_1        0x6B
#define REG_WHO_AM_I          0x75

#define IMU_WHO_AM_I          0x68

#define IMU_DLPF_CFG          3
#define IMU_SMPLRT_DIV        4
#define IMU_GYRO_FS_SEL       1                 // 0:250 1:500 2:1000 3:2000 dps
#define IMU_ACCEL_FS_SEL      0                 // 0:2 1:4 2:8 3:16 g

#define IMU_GYRO_LSB_X10      655               // 65.5 LSB/dps 를 10배 한 값
#define IMU_ACCEL_LSB_PER_G   16384


static bool    is_init = false;
static int16_t raw[7];                          // ax ay az temp gx gy gz
static int32_t gyro_bias[3];                    // raw 단위

#ifdef _USE_HW_CLI
static void cliImu(cli_args_t *args);
#endif


bool imuInit(void)
{
  bool    ret;
  uint8_t id = 0;

  ret = i2cBegin(IMU_I2C_CH, 400);

  if (ret) ret = i2cReadByte(IMU_I2C_CH, IMU_I2C_ADDR, REG_WHO_AM_I, &id, IMU_I2C_TIMEOUT);
  if (ret && id != IMU_WHO_AM_I)
  {
    logPrintf("imu WHO_AM_I 0x%02X (expected 0x%02X)\n", id, IMU_WHO_AM_I);
    ret = false;
  }

  if (ret) ret = i2cWriteByte(IMU_I2C_CH, IMU_I2C_ADDR, REG_PWR_MGMT_1, 0x01, IMU_I2C_TIMEOUT);   // 깨우기, 클럭은 자이로 X PLL
  if (ret)
  {
    k_msleep(10);
    ret = i2cWriteByte(IMU_I2C_CH, IMU_I2C_ADDR, REG_CONFIG,       IMU_DLPF_CFG,       IMU_I2C_TIMEOUT);
  }
  if (ret) ret = i2cWriteByte(IMU_I2C_CH, IMU_I2C_ADDR, REG_SMPLRT_DIV,   IMU_SMPLRT_DIV,     IMU_I2C_TIMEOUT);
  if (ret) ret = i2cWriteByte(IMU_I2C_CH, IMU_I2C_ADDR, REG_GYRO_CONFIG,  IMU_GYRO_FS_SEL << 3,  IMU_I2C_TIMEOUT);
  if (ret) ret = i2cWriteByte(IMU_I2C_CH, IMU_I2C_ADDR, REG_ACCEL_CONFIG, IMU_ACCEL_FS_SEL << 3, IMU_I2C_TIMEOUT);

  is_init = ret;
  logPrintf("[%s] imuInit()\n", ret ? "OK" : "E_");

#ifdef _USE_HW_CLI
  cliAdd("imu", cliImu);
#endif

  return ret;
}

bool imuIsInit(void)
{
  return is_init;
}

bool imuUpdate(void)
{
  uint8_t buf[14];

  if (!is_init) return false;

  if (!i2cReadBytes(IMU_I2C_CH, IMU_I2C_ADDR, REG_ACCEL_XOUT_H, buf, 14, IMU_I2C_TIMEOUT))
  {
    return false;
  }

  for (int i=0; i<7; i++)
  {
    raw[i] = (int16_t)((buf[i*2] << 8) | buf[i*2 + 1]);     // 상위 바이트가 먼저
  }
  return true;
}

bool imuGetData(imu_data_t *p_data)
{
  if (!is_init) return false;

  p_data->ax_mg = (int32_t)raw[0] * 1000 / IMU_ACCEL_LSB_PER_G;
  p_data->ay_mg = (int32_t)raw[1] * 1000 / IMU_ACCEL_LSB_PER_G;
  p_data->az_mg = (int32_t)raw[2] * 1000 / IMU_ACCEL_LSB_PER_G;

  // 온도: raw / 340 + 36.53 degC
  p_data->temp_cdeg = (int32_t)raw[3] * 100 / 340 + 3653;

  p_data->gx_mdps = ((int32_t)raw[4] - gyro_bias[0]) * 10000 / IMU_GYRO_LSB_X10;
  p_data->gy_mdps = ((int32_t)raw[5] - gyro_bias[1]) * 10000 / IMU_GYRO_LSB_X10;
  p_data->gz_mdps = ((int32_t)raw[6] - gyro_bias[2]) * 10000 / IMU_GYRO_LSB_X10;
  return true;
}

bool imuCalibrateGyro(uint32_t samples)
{
  int64_t sum[3] = {0, 0, 0};
  uint32_t n = 0;

  if (!is_init || samples == 0) return false;

  for (uint32_t i=0; i<samples; i++)
  {
    if (imuUpdate())
    {
      sum[0] += raw[4];
      sum[1] += raw[5];
      sum[2] += raw[6];
      n++;
    }
    k_msleep(5);                                // 출력 주기 200 Hz 에 맞춤
  }
  if (n == 0) return false;

  for (int i=0; i<3; i++)
  {
    gyro_bias[i] = (int32_t)(sum[i] / n);
  }
  return true;
}

void imuClearBias(void)
{
  for (int i=0; i<3; i++)
  {
    gyro_bias[i] = 0;
  }
}





#ifdef _USE_HW_CLI

static void printData(void)
{
  imu_data_t d;

  imuGetData(&d);
  cliPrintf("acc mg  %6d %6d %6d | gyro mdps %7d %7d %7d\n",
            (int)d.ax_mg, (int)d.ay_mg, (int)d.az_mg,
            (int)d.gx_mdps, (int)d.gy_mdps, (int)d.gz_mdps);
}

void cliImu(cli_args_t *args)
{
  bool ret = false;


  if (args->argc == 1 && args->isStr(0, "info") == true)
  {
    imu_data_t d;

    cliPrintf("init %d, addr 0x%02X, i2c err %u\n", is_init, IMU_I2C_ADDR, (unsigned)i2cGetErrCount(IMU_I2C_CH));
    cliPrintf("dlpf %d, div %d (%d Hz), gyro fs %d, accel fs %d\n",
              IMU_DLPF_CFG, IMU_SMPLRT_DIV, 1000 / (1 + IMU_SMPLRT_DIV), IMU_GYRO_FS_SEL, IMU_ACCEL_FS_SEL);
    cliPrintf("gyro bias raw %d %d %d\n", (int)gyro_bias[0], (int)gyro_bias[1], (int)gyro_bias[2]);
    if (imuUpdate())
    {
      imuGetData(&d);
      cliPrintf("temp %d.%02d degC\n", (int)(d.temp_cdeg / 100), (int)(d.temp_cdeg % 100));
    }
    ret = true;
  }

  if (args->argc == 1 && args->isStr(0, "read") == true)
  {
    if (imuUpdate())
      printData();
    else
      cliPrintf("read fail\n");
    ret = true;
  }

  if (args->argc == 2 && args->isStr(0, "watch") == true)
  {
    uint32_t pre_time = millis();

    while (cliKeepLoop())
    {
      if (millis() - pre_time >= (uint32_t)args->getData(1))
      {
        pre_time = millis();
        if (imuUpdate())
          printData();
        else
          cliPrintf("read fail\n");
      }
    }
    ret = true;
  }

  if (args->argc == 1 && args->isStr(0, "cal") == true)
  {
    cliPrintf("keep still... (1 s)\n");
    if (imuCalibrateGyro(200))
      cliPrintf("cal OK, bias raw %d %d %d\n", (int)gyro_bias[0], (int)gyro_bias[1], (int)gyro_bias[2]);
    else
      cliPrintf("cal fail\n");
    ret = true;
  }

  if (args->argc == 1 && args->isStr(0, "clear") == true)
  {
    imuClearBias();
    cliPrintf("bias cleared\n");
    ret = true;
  }


  if (ret != true)
  {
    cliPrintf("imu info\n");
    cliPrintf("imu read\n");
    cliPrintf("imu watch time_ms\n");
    cliPrintf("imu cal\n");
    cliPrintf("imu clear\n");
  }
}


#endif


#endif
