#ifndef BSP_KEY_H
#define BSP_KEY_H

#include "ti_msp_dl_config.h"
#include "string.h"
#include "my_Ticks.h"

#define key_busy true
#define key_idle false

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

typedef void(*TaskFunc)(void);

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

#endif /* BSP_KEY_H */
