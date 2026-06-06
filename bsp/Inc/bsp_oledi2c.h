#ifndef BSP_OLEDI2C_H
#define BSP_OLEDI2C_H

#include "ti_msp_dl_config.h"
#include "my_Ticks.h"
#include "oled_font.h"
#include "string.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>


#define OLED_ADRESS 0x3C

typedef enum {
    WRITE_CMD,
    WRITE_DATA
}I2C_WriteType;

void OLED_Init(void);
void OLED_ClearScreen(void);
void OLED_ShowImage (uint8_t X, uint8_t Page, uint8_t Width, uint8_t Height, uint8_t *Image);
void OLED_Printf_Hope(int16_t X, int16_t Y, uint8_t FontSize, char *format, ...);
void OLED_Printf_Regret(int16_t X, int16_t Y, uint8_t FontSize, char *format, ...);

#endif // !BSP_OLEDI2C_H
