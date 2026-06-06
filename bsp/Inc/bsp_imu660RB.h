#ifndef BSP_IMU660RB_H
#define BSP_IMU660RB_H

#include "ti_msp_dl_config.h"
#include "my_Ticks.h"
#include <math.h>
#define M_PI		3.14159265358979323846

#define IMU660RB_ADDRESS            0x6A // AD0默认为低电平的IMU660RB从机地址
#define IMU660RB_GYRO_CONFIG        0x11 // 陀螺仪配置寄存器
#define IMU660RB_ACCEL_CONFIG       0x10 // 加速度计配置寄存器
#define IMU660RB_CTRL6_C            0x15 // IMU660RB硬件配置寄存器(保险起见默认配置0x00)
#define IMU660RB_CTRL3_C            0x12 // IMU660RB软件配置寄存器
#define OUT_TEMP_L                  0x20 // 低八位温度寄存器
#define OUT_TEMP_H                  0x21 // 高八位温度寄存器
#define OUTX_L_G                    0x22 // 低八位X轴陀螺仪寄存器
#define OUTX_H_G                    0x23 // 高八位X轴陀螺仪寄存器
#define OUTY_L_G                    0x24 // 低八位Y轴陀螺仪寄存器
#define OUTY_H_G                    0x25 // 高八位Y轴陀螺仪寄存器
#define OUTZ_L_G                    0x26 // 低八位Z轴陀螺仪寄存器
#define OUTZ_H_G                    0x27 // 高八位Z轴陀螺仪寄存器
#define OUTX_L_A                    0x28 // 低八位X轴加速度计寄存器
#define OUTX_H_A                    0x29 // 高八位X轴加速度计寄存器
#define OUTY_L_A                    0x2A // 低八位Y轴加速度计寄存器
#define OUTY_H_A                    0x2B // 高八位Y轴加速度计寄存器
#define OUTZ_L_A                    0x2C // 低八位Z轴加速度计寄存器
#define OUTZ_H_A                    0x2D // 高八位Z轴加速度计寄存器

// 陀螺仪量程选项(对应的是CTRL2_G中(3bit ~ 4bit)位的FS[1:0]_XL)
typedef enum {
    GYRO_RANGE_250DPS   =    0,  // ±250°/s
    GYRO_RANGE_500DPS   =    1,  // ±500°/s
    GYRO_RANGE_1000DPS  =    2, // ±1000°/s
    GYRO_RANGE_2000DPS  =    3  // ±2000°/s
} GyroRange_t;

// 加速度计量程选项(对应的是CTRL1_XL中(3bit ~ 4bit)位的FS[1:0]_XL)
typedef enum {
    ACCEL_RANGE_2G   =      0, // ±2g
    ACCEL_RANGE_16G  =      1, // ±16g
    ACCEL_RANGE_4G   =      2, // ±4g
    ACCEL_RANGE_8G   =      3  // ±8g
}AccelRange_t;

typedef enum {
    LFP2_DISABLE = 0U,
    LFP2_ENABLE
}Lfp2_State_t;

// 采样速率选项(对应的是CTRL1_XL中(5bit ~ 8bit)位的ODR_XL[3:0])
typedef enum{
    ODR_POWER_DOWN = 0x00,
    ODR_12_5HZ     = 0x10,
    ODR_26HZ       = 0x20,
    ODR_52HZ       = 0x30,
    ODR_104HZ      = 0x40,
    ODR_208HZ      = 0x50,
    ODR_416HZ      = 0x60, 
    ODR_833HZ      = 0x70,
    ODR_1_66kHZ    = 0x80,
    ODR_3_33kHZ    = 0x90,
    ODR_6_66kHZ    = 0xA0,
}ODR_t;

// 陀螺仪配置结构体
typedef struct {
    GyroRange_t gyro_range;             // 陀螺仪量程
    float full_scale_gyro_range;        // 陀螺仪量程对应的满量程值
    float lsb_sensitivity_gyro;         // 陀螺仪量程对应的灵敏度 (dps / LSB)
    ODR_t gyro_odr;                     // 陀螺仪采样速率 (建议是416Hz)
}GyroConfig_t;

// 加速度计配置结构体
typedef struct {
    AccelRange_t accel_range;           // 加速度计量程
    float full_scale_accel_range;       // 加速度计量程对应的满量程值
    float lsb_sensitivity_accel;        // 加速度计量程对应的灵敏度 (g / LSB)
    ODR_t accel_odr;                    // 加速度计采样速率 (建议是416Hz)
    Lfp2_State_t lpf2_en;               // LPF2滤波配置: (0 << 1) -> 一阶滤波 ,    (1 << 1) -> 二阶滤波
}AccelConfig_t;

typedef struct {
    int16_t temp;

    int16_t accel_x;    // 加速度计X轴原始数据
    int16_t accel_y;    // 加速度计Y轴原始数据
    int16_t accel_z;    // 加速度计Z轴原始数据

    int16_t gyro_x;     // 陀螺仪X轴原始数据
    int16_t gyro_y;     // 陀螺仪Y轴原始数据
    int16_t gyro_z;     // 陀螺仪Z轴原始数据
}IMU660RB_RawData_t;

typedef struct {
    float temp;      // 温度(摄氏度) 

    float accel_x;   // 加速度计X轴数据 (m/s^2)
    float accel_y;   // 加速度计Y轴数据 (m/s^2)
    float accel_z;   // 加速度计Z轴数据 (m/s^2)

    float gyro_x;   // 陀螺仪X轴数据 (°/s)
    float gyro_y;   // 陀螺仪Y轴数据 (°/s)
    float gyro_z;   // 陀螺仪Z轴数据 (°/s)
}IMU660RB_Data_t;

typedef struct {
    float yaw;      // 偏航角 (°)
    float pitch;    // 俯仰角 (°)
    float roll;     // 横滚角 (°)
}EulerAngle_t;

extern IMU660RB_RawData_t IMU660RB_RawData;
extern IMU660RB_Data_t IMU660RB_Data;
extern EulerAngle_t EulerAngle;

// 校准数据（由IMU660RB_Calibration填入，外部可读取用于调试）
extern float gyro_bias_x, gyro_bias_y, gyro_bias_z;
extern float angle_offset_pitch, angle_offset_roll;

void IMU660RB_Init(void);
void IMU660RB_Calibration(void);
void IMU660RB_GetRawData(void);
void IMU660RB_EulerAngleCalc(float dt);

#endif // !BSP_IMU660RB_H
