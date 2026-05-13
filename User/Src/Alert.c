#include "Alert.h"

Alert_State alert_state = ALERT_IEDE;
Alert_State alert_laststate = ALERT_IEDE;

/**
 * @brief 声光提示
 * 
 */
void Alert_Tip(void)
{
    WR_TASK_PERIODIC(alert_tip, 100)

    static uint32_t alert_starttick = 0;

    if (alert_state == ALERT_OK)
    {
        if (alert_laststate == ALERT_IEDE)
        {
            alert_starttick = WR_GetTick();
        }

        alert_laststate = alert_state;

        DL_GPIO_togglePins(Alert_led_PORT, Alert_led_alert_led_PIN);

        BspBuzzer_ON();

        if (WR_GetTick() - alert_starttick > 500)
        {
            BspBuzzer_OFF();
            DL_GPIO_clearPins(Alert_led_PORT, Alert_led_alert_led_PIN);
            alert_starttick = WR_GetTick();
            alert_state = ALERT_IEDE;
            alert_laststate = alert_state;
        }
    }
}