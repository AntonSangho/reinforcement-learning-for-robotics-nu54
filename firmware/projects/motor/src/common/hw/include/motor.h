#ifndef MOTOR_H_
#define MOTOR_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "hw_def.h"


#ifdef _USE_HW_MOTOR

#define MOTOR_MAX_CH  HW_MOTOR_MAX_CH


bool motorInit(void);
bool motorSet(uint8_t ch, int32_t duty);    // duty : -100 ~ 100 (%), 부호가 방향
void motorBrake(uint8_t ch);
void motorStop(void);                       // 모든 모터를 코스팅(공회전)으로
int32_t motorGet(uint8_t ch);

#endif

#ifdef __cplusplus
}
#endif

#endif
