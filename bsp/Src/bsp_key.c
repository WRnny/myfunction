#include "bsp_key.h"

/****************** 按键配置表 ***********************/

bsp_key_param_t bsp_key_param[] = {

    {Debug_key_Key_center_PORT, Debug_key_Key_center_PIN, .starttick = 0, .endtick = 0, false},
    {Debug_key_Key_right_PORT, Debug_key_Key_right_PIN, .starttick = 0, .endtick = 0, false},
    {Debug_key_Key_left_PORT, Debug_key_Key_left_PIN, .starttick = 0, .endtick = 0, false},
    {Debug_key_Key_up_PORT, Debug_key_Key_up_PIN, .starttick = 0, .endtick = 0, false},
    {Debug_key_Key_down_PORT, Debug_key_Key_down_PIN, .starttick = 0, .endtick = 0, false},

};

/****************** 按键索引 ***********************/

/**
 * 
 * @note 索引的话主要是提高代码可阅读性
 * @note 一定要记得加索引，不然代码逻辑会混乱
 * 
 * @note 索引记得与配置表的顺序相同不然全乱掉， 其他部分也是这样不然全乱光光
 * 
 * @note 枚举的话放在了"bsp_key.h"里面记得去调整 
 */

 /****************** 函数逻辑部分 ***********************/

 /**
  * @brief 按键任务初始化
  * 
  * @note 主要是初始化定时器
  * 
  */
void BspKey_Init()
{
    // 清除定时器中断标志位
    NVIC_ClearPendingIRQ(Key_task_INST_INT_IRQN);

    // 使能定时器中断
    NVIC_EnableIRQ(Key_task_INST_INT_IRQN);
}

/**
 * @brief 非阻塞式按键检测状态
 * 
 * @note 检测逻辑要一致
 * @note 如果引脚配置上拉电阻需要把invert打开
 * 
 */

void BSP_KeyTask(void)
{

    for(int i = 0; i < BSP_KEY_NUMBER; i++)
    {
        bool current_state = (0 < DL_GPIO_readPins(bsp_key_param[i].gpio, bsp_key_param[i].pins));

        // 按键空闲状态
        if( (bsp_key_param[i].last_state == false && current_state == false && bsp_key_param[i].is_busy == key_busy) 
            ||  bsp_key_param[i].key_shortpressflag == true || bsp_key_param[i].key_longpressflag == true)
        {
            memset(&bsp_key_param[i].last_state, 0, sizeof(bsp_key_param_t) - offsetof(bsp_key_param_t, last_state));
        }

        // 检测按键下
        if(bsp_key_param[i].last_state == false && current_state == true)
        {
            bsp_key_param[i].is_busy = key_busy;
            bsp_key_param[i].key_pressflag = true;
            bsp_key_param[i].key_holdflag = true;
            bsp_key_param[i].starttick = WR_GetTick();
        }

        // 检测按键释放
        if(bsp_key_param[i].last_state == true && current_state == false)
        {
            bsp_key_param[i].key_releaseflag = true;
            bsp_key_param[i].endtick = WR_GetTick();
        }

        // 按键长按短按判断
        if(bsp_key_param[i].is_busy == key_idle)
        {
            // 按键空闲状态
            if(bsp_key_param[i].endtick == bsp_key_param[i].starttick);
            // 短按按键判断时间小于300ms
            else if(bsp_key_param[i].endtick - bsp_key_param[i].starttick < 300)
            {
                bsp_key_param[i].key_shortpressflag = true;
                bsp_key_param[i].endtick = bsp_key_param[i].starttick;
            }else{
                bsp_key_param[i].key_longpressflag = true;
                bsp_key_param[i].endtick = bsp_key_param[i].starttick;
            }
        }
        bsp_key_param[i].last_state = current_state;
    }
    
}

/**
 * @brief 执行简单的按键触发逻辑
 * 
 * @param task 执行的任务逻辑
 * @param key_flag 指定按键检测方式
 * 
 */
void WR_KeyControlTask(TaskFunc task, bool* key_flag)
{
    // 错误处理
    if(task == NULL)
    {
        return;
    }

    // 判断按键状态
    if(*key_flag == true)
    {
        task();
        *key_flag = false;
    }
}

/**
 * @brief 执行简单的按键触发逻辑
 * 
 * @param task 带参数的执行的任务逻辑
 * @param key_flag 指定按键检测方式
 * @param task_param 任务的参数
 * 
 * @note 这个属于如果函数是带参数函数的写法，用起来比较麻烦可以不用
 *       我是为了代码的简介性去写的
 * 
 * @note 具体的使用方法就是，我需要把这个传入的参数的函数命名为"void Task_Init(void *param)"这种形式
 *       之后需要对函数的参数进行一些处理"WR_KeyControlTaskParam(Task_Init, &bsp_key_param[Key_up].key_shortpressflag, WR_TASK_PARAM(TASK_INDEX_ONE));"
 *       可以看出来我暂时用到的需要是把我定义的枚举变量传进去，我在头文件搞了一个宏定义函数"#define WR_TASK_PARAM(param) ((void*) (uintptr_t)(param))"
 *       稍微解释一下这个代码的作用"(uintprt_t)"是一个无符号整数类型大小和指针相同，把参数先转成这样的类型为了安全的转成(void*)指针变量
 *       之后还需要再这个"Task_Init(void *param)"里写"Task_Index task_index = (Task_Index)(uintptr_t)param;"把指针变量转换成原来你要的参数的格式
 *       这样就可以完整的调用这些函数了
 * 
 * @note 不带参数的函数不建议用这个很麻烦得不偿失，之前的那个版本就可以用
 *       如果硬要调用的话空函数的命名也要"Example_Func(void *param)"之后需要在函数里生命"(void)param"不然可能会弹警告
 * 
 * @note 多变量的函数也可以这么搞不过需要把变量放到结构体里面，之后直接把结构体的地址传进去
 *       传进去之后还需要操作就是"example_struct *func_struct = (example_struct*)param"
 *       之后把变量再用结构体"func_struct -> param"用对应的类型引出来类似于"uint8_t one_param = func_struct -> param_one"
 * 
 */
void WR_KeyControlTaskParam(TaskFuncParam task, bool* key_flag, void *task_param)
{
    // 错误处理
    if(task == NULL)
    {
        return;
    }

    // 判断按键状态
    if(*key_flag == true)
    {
        task(task_param);
        *key_flag = false;
    }
}


/**
 * @brief 定时器中断每20ms检测按键状态
 * 
 * @note 优先级 --> Level3 - Lowest
 * 
 */
void Key_task_INST_IRQHandler(void)
{
    switch (DL_Timer_getPendingInterrupt(Key_task_INST))
    {
    case DL_TIMER_IIDX_ZERO:
        BSP_KeyTask();
        DL_Timer_clearInterruptStatus(Key_task_INST, DL_TIMER_IIDX_ZERO);
        break;
    
    default:
        break;
    }
}
