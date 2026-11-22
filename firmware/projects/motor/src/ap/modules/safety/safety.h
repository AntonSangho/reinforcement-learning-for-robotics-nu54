#ifndef SAFETY_H_
#define SAFETY_H_


#include "ap_def.h"

#ifdef __cplusplus
extern "C"
{
#endif

  bool  safetyInit(void);
  bool  safetyIsTrip(void);
  float safetyGetPitch(void);         // 마지막으로 계산한 pitch [deg], 앞으로 기울면 +

#ifdef __cplusplus
}
#endif

#endif
