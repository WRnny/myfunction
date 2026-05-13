#ifndef ALERT_H
#define ALERT_H

#include "ti_msp_dl_config.h"
#include "my_Ticks.h"

#include "bsp_buzzer.h"

typedef enum{
    ALERT_IEDE = 0,
    ALERT_OK
}Alert_State;

extern Alert_State alert_state;

/**
 * @brief 声光提示
 * 
 */
void Alert_Tip(void);

#endif /* ALERT_H */
