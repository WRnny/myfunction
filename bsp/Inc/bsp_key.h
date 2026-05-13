#ifndef BSP_KEY_H
#define BSP_KEY_H

#include "ti_msp_dl_config.h"
#include "string.h"
#include "my_Ticks.h"

#define key_busy true
#define key_idle false

// 计算配置表成员个数
#define BSP_KEY_NUMBER sizeof(bsp_key_param) / sizeof(bsp_key_param_t)

typedef struct{
    GPIO_Regs* gpio;
    uint32_t pins;
    uint32_t starttick;
    uint32_t endtick;
    bool last_state;

    bool key_pressflag;
    bool key_releaseflag;
    bool key_holdflag;
    bool key_longpressflag;
    bool key_shortpressflag;

    bool is_busy;
}bsp_key_param_t;

typedef enum{
    Key_center = 0,
    Key_right = 1,
    Key_left = 2,
    Key_up = 3,
    Key_down = 4
}key_index_t;

// 无参数函数定义的函数类型
typedef void(*TaskFunc)(void);
// 有参数函数的定义类型
typedef void(*TaskFuncParam)(void *param);

// 有参数函数参数类型转化辅助函数
#define WR_TASK_PARAM(param) ((void*) (uintptr_t)(param))

// 配置表外用声明
extern bsp_key_param_t bsp_key_param[];


 /**
  * @brief 按键任务初始化
  * 
  * @note 主要是初始化定时器
  * 
  */
void BspKey_Init();

/**
 * @brief 非阻塞式按键检测状态
 * 
 * @note 检测逻辑要一致
 * @note 如果引脚配置上拉电阻需要把invert打开
 * 
 */

void BSP_KeyTask(void);

/**
 * @brief 执行简单的按键触发逻辑
 * 
 * @param task 执行的任务逻辑
 * @param key_flag 指定按键检测方式
 * 
 */
void WR_KeyControlTask(TaskFunc task, bool* key_flag);

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
void WR_KeyControlTaskParam(TaskFuncParam task, bool* key_flag, void *task_param);

#endif /* BSP_KEY_H */
