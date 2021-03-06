/*
 __________________
|__________________ P7                    _______________________________________  HSB       
|__________________ P6
|__________________ P5                   
|__________________ P4                                                              ^      
|__________________ P3                              P0                              ^
|__________________ P2                                                              ^
|__________________ P1                 
|__________________ P0 -------->          ______________________________________   LSB   

*/

#include "oled.h"
#include "spi.h"
#include "oledfonts.h"
#include "math.h"

uint8_t GRAM[1024] = {0};
uint16_t OLED_FPS;
uint8_t OLED_Display = 0;

void OLED_Init()
{
    OLED_Reset();
    HAL_Delay(100);

    OLED_WriteCmd(0xAE); //关闭显示

    OLED_WriteCmd(0x20); //寻址模式
    OLED_WriteCmd(0x00);

    OLED_WriteCmd(0xB0);
    OLED_WriteCmd(0x00);
    OLED_WriteCmd(0x10);
    OLED_WriteCmd(0x40); //设置显示开始行 [5:0],行数.

    OLED_WriteCmd(0xAE); //关闭显示
    OLED_WriteCmd(0xD5); //设置时钟分频因子,震荡频率
    OLED_WriteCmd(0xf0); //[3:0],分频因子;[7:4],震荡频率

    OLED_WriteCmd(0xA8); //设置驱动路数
    OLED_WriteCmd(0X3F); //默认0X3F(1/64)

    OLED_WriteCmd(0xD3); //设置显示偏移
    OLED_WriteCmd(0X00); //默认为0

    OLED_WriteCmd(0x8D); //电荷泵设置
    OLED_WriteCmd(0x14); //bit2，开启/关闭

    OLED_WriteCmd(0xA1); //段重定义设置,bit0:0,0->0;1,0->127;
    OLED_WriteCmd(0xC0); //设置COM扫描方向;bit3:0,普通模式;1,重定义模式 COM[N-1]->COM0;N:驱动路数

    OLED_WriteCmd(0xDA); //设置COM硬件引脚配置
    OLED_WriteCmd(0x12); //[5:4]配置

    OLED_WriteCmd(0x81); //对比度设置
    OLED_WriteCmd(0xFF); //1~255;默认0X7F (亮度设置,越大越亮)

    OLED_WriteCmd(0xD9); //设置预充电周期
    OLED_WriteCmd(0x22); //[3:0],PHASE 1;[7:4],PHASE 2;

    OLED_WriteCmd(0xDB); //设置VCOMH 电压倍率
    OLED_WriteCmd(0x20); //[6:4] 000,0.65*vcc;001,0.77*vcc;011,0.83*vcc;

    OLED_WriteCmd(0xA6); //设置显示方式;bit0:1,反相显示;0,正常显示
    OLED_WriteCmd(0xA4); //全局显示开启;bit0:1,开启;0,关闭;(白屏/黑屏)

    OLED_WriteCmd(0xAF); //开启显示

    OLED_Clean();
    OLED_Update();
}

void OLED_Reset()
{
    RST_LOW;
    HAL_Delay(10);
    RST_HIGH;
    HAL_Delay(10);
}

void OLED_WriteCmd(uint8_t cmd)
{
    CS_LOW;
    DC_LOW;
    HAL_SPI_Transmit(&OLED_SPI_HANDLE, &cmd, 1, HAL_MAX_DELAY);
    CS_HIGH;
}

void OLED_WriteData(uint8_t *buffer, uint16_t buffersize)
{
    CS_LOW;
    DC_HIGH;
    HAL_SPI_Transmit(&OLED_SPI_HANDLE, buffer, buffersize, HAL_MAX_DELAY);
    CS_HIGH;
}

void OLED_Clean()
{
    for (uint16_t i = 0; i < 1024; i++)
    {
        GRAM[i] = 0x00;
    }
}

void OLED_Update()
{
    //一定要等所有数据全部写完再进行Update
    OLED_WriteCmd(0xB0);
    OLED_WriteCmd(0x00);
    OLED_WriteCmd(0x10);
    OLED_WriteData(GRAM, 1024);
    // uint8_t i;
    // for (i = 0; i < 8; i++)
    // {
    //     OLED_WriteCmd(0xB0 + i);
    //     OLED_WriteCmd(0x00);
    //     OLED_WriteCmd(0x10);
    //     OLED_WriteData(GRAM, 128);
    // }
}

//x:0-127
//y:0-63
void OLED_DrawPixel(uint8_t x, uint8_t y, uint8_t color)
{
    if (color == White)
        GRAM[x + (y / 8) * 128] |= 1 << (y % 8);
    else
        GRAM[x + (y / 8) * 128] &= ~(1 << (y % 8));
}

//8x16
void OLED_WriteChar(uint8_t ch, uint8_t x, uint8_t y)
{
    uint8_t i, temp;
    ch = ch - ' ';
    for (i = 0; i < 8; i++)
    {
        temp = oled_ascll[16 * ch + i];
        for (uint8_t j = 0; j < 8; j++)
        {
            if ((temp << j) & 0x80)
                OLED_DrawPixel(x + i, y + j, White);
            else
                OLED_DrawPixel(x + i, y + j, Black);
        }
    }

    y -= 8;
    for (i = 8; i < 16; i++)
    {
        temp = oled_ascll[16 * ch + i];
        for (uint8_t j = 0; j < 8; j++)
        {
            if ((temp << j) & 0x80)
                OLED_DrawPixel(x + i - 8, y + j, White);
            else
                OLED_DrawPixel(x + i - 8, y + j, Black);
        }
    }
}

void OLED_WriteString(char *str, uint8_t x, uint8_t y)
{
    while (*str != '\0')
    {
        OLED_WriteChar(*str, x, y);
        str++;
        x += 8;
    }
}

void OLED_WriteNumber(float number, uint8_t x, uint8_t y)
{
    uint16_t n4, n3, n2, n1;

    n4 = (uint16_t)(fabs(number) / 100);
    n3 = ((uint16_t)(fabs(number)) / 10) % 10;
    n2 = ((uint16_t)fabs(number)) % 10;
    n1 = ((uint16_t)(fabs(number) * 10)) % 10;

    if (number > 0)
    {
        OLED_WriteChar(' ', x, y);
        OLED_WriteChar(n4 + 16 + ' ', x + 8, y);
        OLED_WriteChar(n3 + 16 + ' ', x + 16, y);
        OLED_WriteChar(n2 + 16 + ' ', x + 24, y);
        OLED_WriteChar('.', x + 32, y);
        OLED_WriteChar(n1 + 16 + ' ', x + 40, y);
    }
    else
    {
        OLED_WriteChar('-', x, y);
        OLED_WriteChar(n4 + 16 + ' ', x + 8, y);
        OLED_WriteChar(n3 + 16 + ' ', x + 16, y);
        OLED_WriteChar(n2 + 16 + ' ', x + 24, y);
        OLED_WriteChar('.', x + 32, y);
        OLED_WriteChar(n1 + 16 + ' ', x + 40, y);
    }
}

//16x16
void OLED_WriteChinese(const uint8_t *chinese, uint8_t x, uint8_t y)
{
    uint8_t temp;
    for (uint8_t i = 0; i < 16; i++)
    {
        temp = chinese[i];
        for (uint8_t j = 0; j < 8; j++)
        {
            if ((temp << j) & 0x80)
                OLED_DrawPixel(x + i, y + j, White);
            else
                OLED_DrawPixel(x + i, y + j, Black);
        }
    }

    y -= 8;
    for (uint8_t i = 16; i < 32; i++)
    {
        temp = chinese[i];
        for (uint8_t j = 0; j < 8; j++)
        {
            if ((temp << j) & 0x80)
                OLED_DrawPixel(x + i - 16, y + j, White);
            else
                OLED_DrawPixel(x + i - 16, y + j, Black);
        }
    }
}
