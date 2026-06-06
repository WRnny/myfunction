#include "bsp_oledi2c.h"

static void WR_OLEDI2C_SendData(I2C_WriteType type, uint8_t data)
{

    uint8_t SendData[2] = { 0 };

    // 0x40为写入数据; 0x00为写入命令
    SendData[0] = type ? 0x40 : 0x00;
    SendData[1] = data;

    // 等待I2C总线空闲
    while ( !((DL_I2C_getControllerStatus(I2C1)) & DL_I2C_CONTROLLER_STATUS_IDLE) );

    // 将传入数据放到FIFO里
    DL_I2C_fillControllerTXFIFO(I2C1, SendData, 2);

    // 传输数据
    DL_I2C_startControllerTransfer(I2C1, OLED_ADRESS, DL_I2C_CONTROLLER_DIRECTION_TX, 2);

    // 等待总线开始工作
    while ( !((DL_I2C_getControllerStatus(I2C1)) & DL_I2C_CONTROLLER_STATUS_BUSY_BUS) );

    // 等待总线传输完毕
    while ( !((DL_I2C_getControllerStatus(I2C1)) & DL_I2C_CONTROLLER_STATUS_IDLE) );

    // 清理TXFIFO数据
    DL_I2C_flushControllerTXFIFO(I2C1);
}

static void OLED_SetCursor(uint8_t X, uint8_t Page)
{
    WR_OLEDI2C_SendData(WRITE_CMD, 0x00 | (X & 0x0F) );
    WR_OLEDI2C_SendData(WRITE_CMD, 0x10 | (X & 0xF0) >> 4 );

    WR_OLEDI2C_SendData(WRITE_CMD, 0xB0 | Page);
}

void OLED_ClearScreen(void)
{
    for (int j = 0; j < 8; j++)
    {
        OLED_SetCursor(0, j);
        for (int i = 0; i < 128; i++)
        {
            WR_OLEDI2C_SendData(WRITE_DATA, 0x00);
        }
    }
}

void OLED_Init(void)
{
    WR_Delay(100);

    WR_OLEDI2C_SendData(WRITE_CMD, 0xAE);

    WR_OLEDI2C_SendData(WRITE_CMD, 0xD5);
    WR_OLEDI2C_SendData(WRITE_CMD, 0x80);

    WR_OLEDI2C_SendData(WRITE_CMD, 0xA8);
    WR_OLEDI2C_SendData(WRITE_CMD, 0x3F);

    WR_OLEDI2C_SendData(WRITE_CMD, 0xD3);
    WR_OLEDI2C_SendData(WRITE_CMD, 0x00);
    
    WR_OLEDI2C_SendData(WRITE_CMD, 0x40);

    WR_OLEDI2C_SendData(WRITE_CMD, 0xA1);

    WR_OLEDI2C_SendData(WRITE_CMD, 0xC8);

    WR_OLEDI2C_SendData(WRITE_CMD, 0xDA);
    WR_OLEDI2C_SendData(WRITE_CMD, 0x12);

    WR_OLEDI2C_SendData(WRITE_CMD, 0x81);
    WR_OLEDI2C_SendData(WRITE_CMD, 0xCF);

    WR_OLEDI2C_SendData(WRITE_CMD, 0xD9);
    WR_OLEDI2C_SendData(WRITE_CMD, 0xF1);

    WR_OLEDI2C_SendData(WRITE_CMD, 0xD8);
    WR_OLEDI2C_SendData(WRITE_CMD, 0x30);

    WR_OLEDI2C_SendData(WRITE_CMD, 0xA4);

    WR_OLEDI2C_SendData(WRITE_CMD, 0xA6);

    WR_OLEDI2C_SendData(WRITE_CMD, 0x8D);
    WR_OLEDI2C_SendData(WRITE_CMD, 0x14);

    WR_OLEDI2C_SendData(WRITE_CMD, 0xAF);

    WR_Delay(100);

    OLED_ClearScreen();
}

static void OLED_ShowChar_Hope(uint8_t X, uint8_t Page, char Char, uint8_t FontSize)
{
    if (FontSize == 6)
    {
        OLED_SetCursor(X, Page);
        for (int i = 0; i < 6; i++)
        {
            WR_OLEDI2C_SendData(WRITE_DATA, OLED_F6x8_Hope[Char - ' '][i]);
        }
    }else if (FontSize == 8){
        OLED_SetCursor(X, Page);
        for (int i = 0; i < 8; i++)
        {
            WR_OLEDI2C_SendData(WRITE_DATA, OLED_F8x16_Hope[Char - ' '][i]);
        }
        OLED_SetCursor(X, Page + 1);
        for (int i = 0; i < 8; i++)
        {
            WR_OLEDI2C_SendData(WRITE_DATA, OLED_F8x16_Hope[Char - ' '][i + 8]);
        }
    }
}

static void OLED_ShowChar_Regret(uint8_t X, uint8_t Page, char Char, uint8_t FontSize)
{
    if (FontSize == 6)
    {
        OLED_SetCursor(X, Page);
        for (int i = 0; i < 6; i++)
        {
            WR_OLEDI2C_SendData(WRITE_DATA, OLED_F6x8_Regret[Char - ' '][i]);
        }
    }else if (FontSize == 8){
        OLED_SetCursor(X, Page);
        for (int i = 0; i < 8; i++)
        {
            WR_OLEDI2C_SendData(WRITE_DATA, OLED_F8x16_Regret[Char - ' '][i]);
        }
        OLED_SetCursor(X, Page + 1);
        for (int i = 0; i < 8; i++)
        {
            WR_OLEDI2C_SendData(WRITE_DATA, OLED_F8x16_Regret[Char - ' '][i + 8]);
        }
    }
}

static void OLED_ShowString_Hope(uint8_t X, uint8_t Page, char *string, uint8_t FontSize)
{
    uint8_t index = 0;
    while (string[index])
    {
        OLED_ShowChar_Hope(X + index * FontSize, Page, string[index], FontSize);
        index++;
    }
}

static void OLED_ShowString_Regret(uint8_t X, uint8_t Page, char *string, uint8_t FontSize)
{
    uint8_t index = 0;
    while (string[index])
    {
        OLED_ShowChar_Regret(X + index * FontSize, Page, string[index], FontSize);
        index++;
    }
}


void OLED_ShowImage (uint8_t X, uint8_t Page, uint8_t Width, uint8_t Height, uint8_t *Image)
{
    for (uint8_t i = 0; i < Height; i++)
    {
        OLED_SetCursor(X, Page + i);
        for(uint8_t j = 0; j < Width; j++)\
        {
            WR_OLEDI2C_SendData(WRITE_DATA, Image[Width * i + j]);
        }
    }
}

void OLED_Printf_Hope(int16_t X, int16_t Y, uint8_t FontSize, char *format, ...)
{
	char String[256];
	va_list arg;
	va_start(arg, format);
	vsprintf(String, format, arg);
	va_end(arg);
	OLED_ShowString_Hope(X, Y, String, FontSize);
}

void OLED_Printf_Regret(int16_t X, int16_t Y, uint8_t FontSize, char *format, ...)
{
	char String[256];
	va_list arg;
	va_start(arg, format);
	vsprintf(String, format, arg);
	va_end(arg);
	OLED_ShowString_Regret(X, Y, String, FontSize);
}

