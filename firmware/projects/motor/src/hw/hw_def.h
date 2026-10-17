#ifndef HW_DEF_H_
#define HW_DEF_H_


#include "bsp.h"


#define _DEF_FIRMWATRE_VERSION      "V261012R1"
#define _DEF_BOARD_NAME             "NU54-DK-MOTOR"



#define _HW_DEF_RTOS_THREAD_PRI_CLI           5
#define _HW_DEF_RTOS_THREAD_MEM_CLI           (4*1024)


#define _USE_HW_RTOS


// LED2(P1.10), LED4(P1.14) 핀은 모터 PWM 이 쓰므로 led 모듈을 넣지 않는다.

/* 하드웨어 채널은 VCOM1(uart20), VCOM0(uart30). 이후 가상 채널(BLE 등)은 uart_driver_t 를 등록해 붙인다.
 * 채널 번호는 기능을 켜고 끄는 것과 상관없이 고정한다 (docs/05_uart.md 채널 표). */
#define _USE_HW_UART
#define      HW_UART_CH_LOG         _DEF_UART1        // uart20 (VCOM1)
#define      HW_UART_CH_CLI         HW_UART_CH_LOG
#define      HW_UART_MAX_CH         1

#define _USE_HW_LOG
#define      HW_LOG_CH              HW_UART_CH_LOG
#define      HW_LOG_BOOT_BUF_MAX    2048
#define      HW_LOG_LIST_BUF_MAX    4096

#define _USE_HW_I2C
#define      HW_I2C_MAX_CH          1                 // _DEF_I2C1 : i2c21 (Qwiic J5, PMIC)

#define _USE_HW_IMU                           // MPU-6050 (0x68), i2c 채널은 imu.c 에서 지정

#define _USE_HW_ENC
#define      HW_ENC_MAX_CH          1                 // 모터 A 엔코더 (A상 P2.04, B상 P2.05)

#define _USE_HW_MOTOR
#define      HW_MOTOR_MAX_CH        2                 // A : 왼쪽, B : 오른쪽
#define      HW_MOTOR_DUTY_MAX      100               // % (전원 전압에 따라 낮춘다)

#define _USE_HW_BUTTON
#define      HW_BUTTON_MAX_CH       BUTTON_PIN_MAX

#define _USE_HW_CLI
#define      HW_CLI_CMD_LIST_MAX    32
#define      HW_CLI_CMD_NAME_MAX    16
#define      HW_CLI_LINE_HIS_MAX    8
#define      HW_CLI_LINE_BUF_MAX    64

//-- CLI
//
#define _USE_CLI_HW_UART            1
#define _USE_CLI_HW_LOG             1
#define _USE_CLI_HW_BUTTON          1
#define _USE_CLI_HW_I2C             1
#define _USE_CLI_HW_MOTOR           1
#define _USE_CLI_HW_ENC             1
#define _USE_CLI_HW_IMU             1


// 보드 스위치 SW1~SW4 (DTS button0~button3)
//
typedef enum
{
  BTN1,
  BTN2,
  BTN3,
  BTN4,
  BUTTON_PIN_MAX,
} ButtonPinName_t;


#endif
