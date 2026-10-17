#ifndef IMU_H_
#define IMU_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "hw_def.h"


#ifdef _USE_HW_IMU

typedef struct
{
  int32_t ax_mg, ay_mg, az_mg;          // 가속도 [mg]
  int32_t gx_mdps, gy_mdps, gz_mdps;    // 자이로 (바이어스 보정 후) [mdps]
  int32_t temp_cdeg;                    // 온도 [0.01 degC]
} imu_data_t;


bool imuInit(void);
bool imuIsInit(void);
bool imuUpdate(void);                   // 14바이트를 한 번에 읽어 최신 값을 갱신
bool imuGetData(imu_data_t *p_data);    // imuUpdate() 가 읽어 둔 값
bool imuCalibrateGyro(uint32_t samples);// 정지 상태 평균으로 자이로 바이어스를 구한다 (RAM)
void imuClearBias(void);

#endif

#ifdef __cplusplus
}
#endif

#endif
