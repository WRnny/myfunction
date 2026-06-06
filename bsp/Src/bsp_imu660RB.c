#include "bsp_imu660RB.h"

/* ===================================================================
 * 配置宏
 * =================================================================== */
#define CALIB_SAMPLES      500       // 开机校准采样次数
#define CALIB_INTERVAL     5         // 采样间隔 (ms)，总计约2.5秒
#define DEG2RAD            (0.01745329252f)  // PI/180
#define RAD2DEG            (57.2957795131f)  // 180/PI

// Mahony AHRS 滤波器参数
#define MAHONY_KP          2.0f      // 姿态修正比例增益
#define MAHONY_KI_FAST     0.5f      // 启动阶段零偏追踪增益
#define MAHONY_KI_SLOW     0.02f     // 静止时零偏追踪增益
#define MAHONY_STARTUP_CNT 200       // 快速收敛持续周期数（≈2秒）
#define BIAS_STILL_THR     0.0087f   // 静止判定阈值 rad/s（≈0.5°/s）
#define YAW_DEAD_ZONE      0.0087f   // yaw死区 rad/s（≈0.5°/s，加速度计无法观测yaw）

/* ===================================================================
 * 配置表
 * =================================================================== */
static const GyroConfig_t gyro_config_table[] = {
    {GYRO_RANGE_250DPS,  250.0f,  114.286f,  ODR_416HZ},
    {GYRO_RANGE_500DPS,  500.0f,  57.143f,   ODR_416HZ},
    {GYRO_RANGE_1000DPS, 1000.0f, 28.571f,   ODR_416HZ},
    {GYRO_RANGE_2000DPS, 2000.0f, 14.286f,   ODR_416HZ}
};

static const AccelConfig_t accel_config_table[] = {
    {ACCEL_RANGE_2G,  2.0f,  16393.443f, ODR_416HZ, LFP2_ENABLE},
    {ACCEL_RANGE_4G,  4.0f,  8196.721f,  ODR_416HZ, LFP2_ENABLE},
    {ACCEL_RANGE_8G,  8.0f,  4098.361f,  ODR_416HZ, LFP2_ENABLE},
    {ACCEL_RANGE_16G, 16.0f, 2049.180f,  ODR_416HZ, LFP2_ENABLE}
};

/* ===================================================================
 * 全局数据
 * =================================================================== */
IMU660RB_RawData_t IMU660RB_RawData = {0};
IMU660RB_Data_t    IMU660RB_Data    = {0};
EulerAngle_t       EulerAngle       = {0};

// 校准结果（IMU660RB_Calibration 填入，外部可读）
float gyro_bias_x = 0.0f, gyro_bias_y = 0.0f, gyro_bias_z = 0.0f;
float angle_offset_pitch = 0.0f, angle_offset_roll = 0.0f;

/* ===================================================================
 * I2C 底层通信
 * =================================================================== */
static void IMU660RB_WriteRegister(uint8_t reg, uint8_t data)
{
    uint8_t buf[2] = {reg, data};
    while (!(DL_I2C_getControllerStatus(I2C0) & DL_I2C_CONTROLLER_STATUS_IDLE));
    DL_I2C_fillControllerTXFIFO(I2C0, buf, 2);
    DL_I2C_startControllerTransfer(I2C0, IMU660RB_ADDRESS, DL_I2C_CONTROLLER_DIRECTION_TX, 2);
    while (!(DL_I2C_getControllerStatus(I2C0) & DL_I2C_CONTROLLER_STATUS_BUSY_BUS));
    while (!(DL_I2C_getControllerStatus(I2C0) & DL_I2C_CONTROLLER_STATUS_IDLE));
    DL_I2C_flushControllerTXFIFO(I2C0);
}

static uint8_t IMU660RB_ReadRegister_Byte(uint8_t reg)
{
    while (!(DL_I2C_getControllerStatus(I2C0) & DL_I2C_CONTROLLER_STATUS_IDLE));
    DL_I2C_fillControllerTXFIFO(I2C0, &reg, 1U);
    DL_I2C_startControllerTransfer(I2C0, IMU660RB_ADDRESS, DL_I2C_CONTROLLER_DIRECTION_TX, 1U);
    while (!(DL_I2C_getControllerStatus(I2C0) & DL_I2C_CONTROLLER_STATUS_BUSY_BUS));
    while (!(DL_I2C_getControllerStatus(I2C0) & DL_I2C_CONTROLLER_STATUS_IDLE));
    DL_I2C_flushControllerTXFIFO(I2C0);

    DL_I2C_startControllerTransfer(I2C0, IMU660RB_ADDRESS, DL_I2C_CONTROLLER_DIRECTION_RX, 1U);
    while (!(DL_I2C_getControllerStatus(I2C0) & DL_I2C_CONTROLLER_STATUS_BUSY_BUS));
    while (!(DL_I2C_getControllerStatus(I2C0) & DL_I2C_CONTROLLER_STATUS_IDLE));
    return DL_I2C_receiveControllerData(I2C0);
}

static void IMU660RB_ReadRegister(uint8_t reg, uint8_t *pData, uint16_t size)
{
    for (uint16_t i = 0; i < size; i++) {
        pData[i] = IMU660RB_ReadRegister_Byte(reg + i);
    }
}

/* ===================================================================
 * IMU660RB_Init — 初始化IMU寄存器
 * =================================================================== */
void IMU660RB_Init(void)
{
    IMU660RB_WriteRegister(IMU660RB_CTRL3_C, 0x01);  // 软复位
    WR_Delay(20);
    IMU660RB_WriteRegister(IMU660RB_CTRL3_C, 0x04);  // 软件配置
    IMU660RB_WriteRegister(IMU660RB_CTRL6_C, 0x00);  // 硬件配置

    // 陀螺仪：1000dps + 416Hz
    IMU660RB_WriteRegister(IMU660RB_GYRO_CONFIG,
        (gyro_config_table[2].gyro_range << 2) | ODR_416HZ);

    // 加速度计：2g + 416Hz + LPF2
    IMU660RB_WriteRegister(IMU660RB_ACCEL_CONFIG,
        (accel_config_table[0].accel_range << 2) | ODR_416HZ |
        (accel_config_table[0].lpf2_en << 1));
}

/* ===================================================================
 * IMU660RB_Calibration — 开机静止校准（约2.5秒）
 * =================================================================== */
void IMU660RB_Calibration(void)
{
    float sgx = 0, sgy = 0, sgz = 0;
    float sax = 0, say = 0, saz = 0;

    for (uint16_t i = 0; i < CALIB_SAMPLES; i++) {
        IMU660RB_GetRawData();
        sgx += IMU660RB_Data.gyro_x;  sgy += IMU660RB_Data.gyro_y;
        sgz += IMU660RB_Data.gyro_z;
        sax += IMU660RB_Data.accel_x; say += IMU660RB_Data.accel_y;
        saz += IMU660RB_Data.accel_z;
        WR_Delay(CALIB_INTERVAL);
    }

    gyro_bias_x = sgx / CALIB_SAMPLES;
    gyro_bias_y = sgy / CALIB_SAMPLES;
    gyro_bias_z = sgz / CALIB_SAMPLES;

    float ax = sax / CALIB_SAMPLES;
    float ay = say / CALIB_SAMPLES;
    float az = saz / CALIB_SAMPLES;

    angle_offset_pitch = atan2f(-ax, sqrtf(ay*ay + az*az)) * RAD2DEG;
    angle_offset_roll  = atan2f(az, -ay) * RAD2DEG;
}

/* ===================================================================
 * IMU660RB_GetRawData — 读取原始数据并转换为物理量
 * =================================================================== */
void IMU660RB_GetRawData(void)
{
    uint8_t raw[14];
    IMU660RB_ReadRegister(OUT_TEMP_L, raw, 14);

    IMU660RB_RawData.temp    = (int16_t)((raw[1]  << 8) | raw[0]);
    IMU660RB_RawData.gyro_x  = (int16_t)((raw[3]  << 8) | raw[2]);
    IMU660RB_RawData.gyro_y  = (int16_t)((raw[5]  << 8) | raw[4]);
    IMU660RB_RawData.gyro_z  = (int16_t)((raw[7]  << 8) | raw[6]);
    IMU660RB_RawData.accel_x = (int16_t)((raw[9]  << 8) | raw[8]);
    IMU660RB_RawData.accel_y = (int16_t)((raw[11] << 8) | raw[10]);
    IMU660RB_RawData.accel_z = (int16_t)((raw[13] << 8) | raw[12]);

    // 物理量转换
    float gyro_scale  = gyro_config_table[2].lsb_sensitivity_gyro;
    float accel_scale = accel_config_table[0].full_scale_accel_range / 32768.0f;

    IMU660RB_Data.temp    = IMU660RB_RawData.temp * 0.0625f + 25.0f;
    IMU660RB_Data.gyro_x  = IMU660RB_RawData.gyro_x  / gyro_scale;
    IMU660RB_Data.gyro_y  = IMU660RB_RawData.gyro_y  / gyro_scale;
    IMU660RB_Data.gyro_z  = IMU660RB_RawData.gyro_z  / gyro_scale;
    IMU660RB_Data.accel_x = IMU660RB_RawData.accel_x * accel_scale;
    IMU660RB_Data.accel_y = IMU660RB_RawData.accel_y * accel_scale;
    IMU660RB_Data.accel_z = IMU660RB_RawData.accel_z * accel_scale;
}

/* ===================================================================
 * Mahony AHRS 滤波器（带陀螺零偏估计）
 *
 * pitch/roll：加速度计观测 → PI修正零偏 → 自动收敛，无漂移
 * yaw：      加速度计不可观测 → 校准零偏 + 死区斩残余
 * =================================================================== */

// 滤波器状态
static float m_q0 = 1, m_q1 = 0, m_q2 = 0, m_q3 = 0;   // 姿态四元数
static float m_bx = 0, m_by = 0, m_bz = 0;               // 陀螺零偏估计 (rad/s)
static uint8_t  m_init = 0;
static uint16_t m_startup = 0;

// 快速反平方根
static inline float m_rsqrt(float x) { return 1.0f / sqrtf(x); }

// 欧拉角 → 四元数（roll, pitch 为弧度，yaw 固定为 0）
static void m_Euler2Quat(float roll, float pitch,
                          float *q0, float *q1, float *q2, float *q3)
{
    float cr = cosf(roll * 0.5f),  sr = sinf(roll * 0.5f);
    float cp = cosf(pitch * 0.5f), sp = sinf(pitch * 0.5f);
    *q0 = cr * cp;  *q1 = sr * cp;
    *q2 = cr * sp;  *q3 = -sr * sp;
}

// Mahony 滤波器核心更新（gx/gy/gz: rad/s, ax/ay/az: 归一化, dt: 秒）
static void m_MahonyUpdate(float gx, float gy, float gz,
                            float ax, float ay, float az, float dt)
{
    // 归一化加速度
    float r = m_rsqrt(ax*ax + ay*ay + az*az);
    ax *= r; ay *= r; az *= r;

    // 预估重力方向
    float vx = 2*(m_q1*m_q3 - m_q0*m_q2);
    float vy = 2*(m_q0*m_q1 + m_q2*m_q3);
    float vz = m_q0*m_q0 - m_q1*m_q1 - m_q2*m_q2 + m_q3*m_q3;

    // 重力误差（叉积）
    float ex = ay*vz - az*vy;
    float ey = az*vx - ax*vz;
    float ez = ax*vy - ay*vx;

    // 自适应 KI：启动快 / 静止慢 / 运动冻
    float gm = fabsf(gx - m_bx) + fabsf(gy - m_by) + fabsf(gz - m_bz);
    float ki;
    if (m_startup > 0)      { ki = MAHONY_KI_FAST; m_startup--; }
    else if (gm < BIAS_STILL_THR) { ki = MAHONY_KI_SLOW; }
    else                    { ki = 0; }

    m_bx += ki * ex * dt;
    m_by += ki * ey * dt;
    m_bz += ki * ez * dt;

    // 修正角速度
    float gxc = gx + MAHONY_KP * ex - m_bx;
    float gyc = gy + MAHONY_KP * ey - m_by;
    float gzc = gz + MAHONY_KP * ez - m_bz;

    // yaw 死区（加速度计无法观测 yaw 误差）
    if (fabsf(gzc) < YAW_DEAD_ZONE) gzc = 0;

    // 四元数积分
    float qd0 = 0.5f*(-m_q1*gxc - m_q2*gyc - m_q3*gzc);
    float qd1 = 0.5f*( m_q0*gxc + m_q2*gzc - m_q3*gyc);
    float qd2 = 0.5f*( m_q0*gyc - m_q1*gzc + m_q3*gxc);
    float qd3 = 0.5f*( m_q0*gzc + m_q1*gyc - m_q2*gxc);

    m_q0 += qd0*dt; m_q1 += qd1*dt;
    m_q2 += qd2*dt; m_q3 += qd3*dt;

    r = m_rsqrt(m_q0*m_q0 + m_q1*m_q1 + m_q2*m_q2 + m_q3*m_q3);
    m_q0 *= r; m_q1 *= r; m_q2 *= r; m_q3 *= r;
}

// 四元数 → 欧拉角（ZYX 序列，度）
static void m_Quat2Euler(float *roll, float *pitch, float *yaw)
{
    *roll  = atan2f(2*(m_q0*m_q1 + m_q2*m_q3), 1 - 2*(m_q1*m_q1 + m_q2*m_q2)) * RAD2DEG;
    *pitch = asinf(2*(m_q0*m_q2 - m_q3*m_q1)) * RAD2DEG;
    *yaw   = atan2f(2*(m_q0*m_q3 + m_q1*m_q2), 1 - 2*(m_q2*m_q2 + m_q3*m_q3)) * RAD2DEG;
}

/* ===================================================================
 * IMU660RB_EulerAngleCalc — 主入口：读取IMU → Mahony滤波 → 输出欧拉角
 * =================================================================== */
void IMU660RB_EulerAngleCalc(float dt)
{
    IMU660RB_GetRawData();
    WR_TASK_PERIODIC(euleranglecalc_task_id, dt)

    // 实际 dt（秒）
    static uint32_t last_tick = 0;
    uint32_t now = WR_GetTick();
    float dt_s;
    if (last_tick == 0) {
        dt_s = dt * 0.001f;
    } else {
        dt_s = (float)(now - last_tick) * 0.001f;
        if (dt_s > 0.1f) dt_s = dt * 0.001f;
    }
    last_tick = now;

    // ---- 芯片 → 板子坐标系映射 ----
    // 板子 X=芯片X, Y=芯片Z, Z=-芯片Y
    float gx =  IMU660RB_Data.gyro_x * DEG2RAD;
    float gy =  IMU660RB_Data.gyro_z * DEG2RAD;
    float gz = -IMU660RB_Data.gyro_y * DEG2RAD;

    float ax =  IMU660RB_Data.accel_x;
    float ay =  IMU660RB_Data.accel_z;
    float az = -IMU660RB_Data.accel_y;

    // ---- 首次初始化 ----
    if (!m_init) {
        float init_r = atan2f(ay, az);
        float init_p = atan2f(-ax, sqrtf(ay*ay + az*az));
        m_Euler2Quat(init_r, init_p, &m_q0, &m_q1, &m_q2, &m_q3);

        m_bx =  gyro_bias_x * DEG2RAD;
        m_by =  gyro_bias_z * DEG2RAD;
        m_bz = -gyro_bias_y * DEG2RAD;

        m_init = 1;
        m_startup = MAHONY_STARTUP_CNT;
    }

    // ---- Mahony 滤波 + 输出 ----
    m_MahonyUpdate(gx, gy, gz, ax, ay, az, dt_s);

    float raw_roll, raw_pitch, raw_yaw;
    m_Quat2Euler(&raw_roll, &raw_pitch, &raw_yaw);

    EulerAngle.roll  = raw_pitch - angle_offset_pitch;
    EulerAngle.pitch = raw_roll  - angle_offset_roll;
    EulerAngle.yaw   = raw_yaw;
}
