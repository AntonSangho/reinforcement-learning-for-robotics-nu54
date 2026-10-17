#ifndef HW_H_
#define HW_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "hw_def.h"


#include "uart.h"
#include "cli.h"
#include "log.h"
#include "button.h"
#include "i2c.h"
#include "motor.h"
#include "enc.h"
#include "imu.h"


bool hwInit(void);


#ifdef __cplusplus
}
#endif

#endif
