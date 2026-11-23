#ifndef MOTOR_H_
#define MOTOR_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "hw_def.h"


#ifdef _USE_HW_MOTOR

#define MOTOR_MAX_CH  HW_MOTOR_MAX_CH


bool motorInit(void);
bool motorSet(uint8_t ch, int32_t duty);    // duty : -100 ~ 100 (%), 부호가 방향. 잠겨 있으면 false
bool motorSetX10(uint8_t ch, int32_t duty_x10);  // duty : -1000 ~ 1000 (0.1 %)
void motorBrake(uint8_t ch);
void motorStop(void);                       // 모든 모터를 코스팅(공회전)으로
int32_t motorGet(uint8_t ch);
void motorSetDutyMax(int32_t max);          // 0 ~ 100 (%), 기본값 HW_MOTOR_DUTY_MAX
int32_t motorGetDutyMax(void);
void motorLock(bool lock);                  // true : 모터를 세우고 0 이 아닌 duty 를 거부한다
bool motorIsLock(void);

#endif

#ifdef __cplusplus
}
#endif

#endif
