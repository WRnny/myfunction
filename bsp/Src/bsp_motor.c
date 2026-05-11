#include "bsp_motor.h"

/****************** 电机DIR引脚配置表 **************************/

/**
 * @note 这里默认IN1高电平IN2低电平是电机正转，IN1低电平IN2高电平是电机反转
 */
bsp_motor_dir_param_t moror_dir_param[] = {
    {Motor_DIR_PORT, Motor_DIR_AIN1_PIN, Motor_DIR_PORT, Motor_DIR_AIN2_PIN}, //电机A的DIR引脚配置
    {Motor_DIR_PORT, Motor_DIR_BIN1_PIN, Motor_DIR_PORT, Motor_DIR_BIN2_PIN}, //电机B的DIR引脚配置
};

/****************** 电机配置表 ***********************/

bsp_motor_param_t motor_param[] = {
    {Motor_A_INST, GPIO_Motor_A_C1_IDX, {0}},   // 电机A配置
    {Motor_B_INST, GPIO_Motor_B_C1_IDX, {0}},   // 电机B配置
};

/****************** 电机索引 ***********************/

/**
 * 
 * @note 索引的话主要是提高代码可阅读性
 * @note 一定要记得加索引，不然代码初始化和电机设置速度都无法进行
 * 
 * @note 索引记得与配置表的顺序相同不然全乱掉， 其他部分也是这样不然全乱光光
 * 
 * @note 枚举的话放在了"bsp_motor.h"里面记得去调整 
 */


 /****************** 函数逻辑部分 ***********************/

 /**
 * @brief 电机初始化函数
 * 
 */
void BspMotor_Init(void)
{

    /**
     * @param 把电机初始表填全
     * 
     * @note 这部是必须的不是闲的没事干
     * 
     * @note 上面的电机初始化表并不可以直接把DIR引脚配置放进去
     *       全局变量或全局数组的初始值必须在编译期前面确定
     *       当如果把引脚配置表直接传给电机配置表的话，编译器就会报错，所以这一步要放在这里
     * 
     */
    for(int i = 0; i < BSP_MOTOR_NUM; i++)
    {
        motor_param[i].dir = moror_dir_param[i];
    }

    // nothing to do here

    /**
     * @note 在DL库中电机初始化有俩种形式
     * 
     * 1. 在配置的时候图形化界面勾选(Start Timer)的时候 --> 当前的选择
     *    这种情况下在定时器初始化之后就开始计数，所以不需要单独的进行电机定时器通道开启操作
     *    关于DIR引脚也可以在配置的时候设置为默认高电平状态,如果不想这么做自己设置即可
     * 
     * 2. 在配置的时候图形化界面不勾选(Start Timer)的时候 --> 非当前选择
     *    这种情况下需要单独的进行电机定时器通道开启操作
     *    需要用户主动的调用" DL_TimerX_startCounter() "这个函数开启定时器计数
     *    关于DIR引脚也可以在配置的时候设置为默认高电平状态,如果不想这么做自己设置即可
     */


     /**
      * @brief 第二种情况下的电机初始化示例代码如下:
    for(int i = 0; i < BSP_MOTOR_NUM; i++)
    {
        DL_TimerX_startCounter(motor_param[i].timer); // 开启定时器计数
    }
      */

}

/**
 * @brief 配置电机速度的函数
 * 
 * @param motor_id 指定电机ID
 * @param speed 设置电机pwm通道的占空比, 范围-10000 ~ 10000
 * 
 * @note 暂时没有写PID逻辑，后面打算写个PID的逻辑去控制电机的转速
 */
void BspMotor_SetSpeed(motor_index_t motor_id, int16_t dutyCycle)
{
    // 确保电机ID合法
    if(motor_id >= BSP_MOTOR_NUM)
    {
        return;
    }

    // 电机占空比处理
    if(dutyCycle > 0 && dutyCycle < 10000)
    {
        DL_GPIO_setPins(motor_param[motor_id].dir.gpio_in1, motor_param[motor_id].dir.pins_in1);
        DL_GPIO_clearPins(motor_param[motor_id].dir.gpio_in2, motor_param[motor_id].dir.pins_in2);

        DL_Timer_setCaptureCompareValue(motor_param[motor_id].timer, (uint32_t)(dutyCycle), motor_param[motor_id].ccIndex);
    }else if(dutyCycle >= 10000){
        DL_GPIO_setPins(motor_param[motor_id].dir.gpio_in1, motor_param[motor_id].dir.pins_in1);
        DL_GPIO_clearPins(motor_param[motor_id].dir.gpio_in2, motor_param[motor_id].dir.pins_in2);

        DL_Timer_setCaptureCompareValue(motor_param[motor_id].timer, 10000U, motor_param[motor_id].ccIndex);
    }else if(dutyCycle < 0 && dutyCycle > -10000)
    {
        DL_GPIO_setPins(motor_param[motor_id].dir.gpio_in2, motor_param[motor_id].dir.pins_in2);
        DL_GPIO_clearPins(motor_param[motor_id].dir.gpio_in1, motor_param[motor_id].dir.pins_in1);

        DL_Timer_setCaptureCompareValue(motor_param[motor_id].timer, -(uint32_t)(dutyCycle), motor_param[motor_id].ccIndex);
    }else if(dutyCycle <= -10000){
        DL_GPIO_setPins(motor_param[motor_id].dir.gpio_in2, motor_param[motor_id].dir.pins_in2);
        DL_GPIO_clearPins(motor_param[motor_id].dir.gpio_in1, motor_param[motor_id].dir.pins_in1);

        DL_Timer_setCaptureCompareValue(motor_param[motor_id].timer, 10000U, motor_param[motor_id].ccIndex);
    }else{
        DL_GPIO_setPins(motor_param[motor_id].dir.gpio_in1, motor_param[motor_id].dir.pins_in1);
        DL_GPIO_setPins(motor_param[motor_id].dir.gpio_in2, motor_param[motor_id].dir.pins_in2);

        DL_Timer_setCaptureCompareValue(motor_param[motor_id].timer, 0U, motor_param[motor_id].ccIndex);
    }

}

