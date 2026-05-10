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

typedef void(*TaskFunc)(void);

extern bsp_key_param_t bsp_key_param[];

void BSP_KeyTask(void);
void WR_KeyControlTask(TaskFunc task, bool* key_flag);

#endif /* BSP_KEY_H */
