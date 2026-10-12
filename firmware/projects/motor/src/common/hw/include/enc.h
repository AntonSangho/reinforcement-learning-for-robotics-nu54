#ifndef ENC_H_
#define ENC_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "hw_def.h"


#ifdef _USE_HW_ENC

#define ENC_MAX_CH  HW_ENC_MAX_CH


bool    encInit(void);
int32_t encGetCount(uint8_t ch);            // A·B 양쪽 엣지 (x4) 누적 틱
void    encClear(uint8_t ch);

#endif

#ifdef __cplusplus
}
#endif

#endif
