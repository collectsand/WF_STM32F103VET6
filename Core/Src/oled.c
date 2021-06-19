#include "oled.h"

uint8_t GRAM[1024] = {0};
OLED_Poisition_struct OLED_Poisition;
FPS_Struct FPS;

void OLED_Init()
{
    OLED_Reset();
    HAL_Delay(100);

    OLED_WriteCmd(0xAE); //关闭显示

    OLED_WriteCmd(0x20); //寻址模式
    OLED_WriteCmd(0x02); //页寻址

    OLED_WriteCmd(0xAE); //关闭显示
    OLED_WriteCmd(0xD5); //设置时钟分频因子,震荡频率
    OLED_WriteCmd(80);   //[3:0],分频因子;[7:4],震荡频率

    OLED_WriteCmd(0xA8); //设置驱动路数
    OLED_WriteCmd(0X3F); //默认0X3F(1/64)

    OLED_WriteCmd(0xD3); //设置显示偏移
    OLED_WriteCmd(0X00); //默认为0
    OLED_WriteCmd(0x40); //设置显示开始行 [5:0],行数.

    OLED_WriteCmd(0x8D); //电荷泵设置
    OLED_WriteCmd(0x14); //bit2，开启/关闭

    OLED_WriteCmd(0xA1); //段重定义设置,bit0:0,0->0;1,0->127;
    OLED_WriteCmd(0xC0); //设置COM扫描方向;bit3:0,普通模式;1,重定义模式 COM[N-1]->COM0;N:驱动路数
    OLED_WriteCmd(0xDA); //设置COM硬件引脚配置
    OLED_WriteCmd(0x12); //[5:4]配置

    OLED_WriteCmd(0x81); //对比度设置
    OLED_WriteCmd(0xEF); //1~255;默认0X7F (亮度设置,越大越亮)

    OLED_WriteCmd(0xD9); //设置预充电周期
    OLED_WriteCmd(0xf1); //[3:0],PHASE 1;[7:4],PHASE 2;
    OLED_WriteCmd(0xDB); //设置VCOMH 电压倍率
    OLED_WriteCmd(0x30); //[6:4] 000,0.65*vcc;001,0.77*vcc;011,0.83*vcc;

    OLED_WriteCmd(0xA4); //全局显示开启;bit0:1,开启;0,关闭;(白屏/黑屏)
    OLED_WriteCmd(0xA6); //设置显示方式;bit0:1,反相显示;0,正常显示

    OLED_WriteCmd(0xAF); //开启显示

    OLED_Clean();
    OLED_Update();
}

void OLED_Reset()
{
    HAL_GPIO_WritePin(GPIOE, GPIO_PIN_5, GPIO_PIN_RESET);
    HAL_Delay(10);
    HAL_GPIO_WritePin(GPIOE, GPIO_PIN_5, GPIO_PIN_SET);
    HAL_Delay(10);
}

void OLED_WriteCmd(uint8_t cmd)
{
    DC_LOW;
    HAL_SPI_Transmit(&hspi1, &cmd, 1, HAL_MAX_DELAY);
}

void OLED_WriteData(uint8_t *buffer, uint8_t buffersize)
{
    DC_HIGH;
    HAL_SPI_Transmit(&hspi1, buffer, buffersize, HAL_MAX_DELAY);
}

void OLED_Clean()
{
    for (uint32_t i = 0; i < 1024; i++)
    {
        GRAM[i] = 0x00;
    }
}

void OLED_Update()
{
    for (uint8_t i = 0; i < 8; i++)
    {
        OLED_WriteCmd(0xB0 + i);
        OLED_WriteCmd(0x00);
        OLED_WriteCmd(0x10);
        OLED_WriteData(&GRAM[128 * i], 128);
    }
}

void OLED_DrawPixel(uint8_t x, uint8_t y, uint8_t color)
{
    if (color == White)
        GRAM[x + (y / 8) * 128] |= 1 << (y % 8);
    else
        GRAM[x + (y / 8) * 128] &= ~(1 << (y % 8));
}

//8x16
void OLED_WriteChar(const uint8_t *ch, uint8_t x, uint8_t y)
{
    uint8_t temp;
    for (uint8_t i = 0; i < 8; i++)
    {
        temp = ch[i];
        for (uint8_t j = 0; j < 8; j++)
        {
            if ((temp << j) & 0x80)
                OLED_DrawPixel(x + i, y + j, White);
            else
                OLED_DrawPixel(x + i, y + j, Black);
        }
    }

    y -= 8;
    for (uint8_t i = 8; i < 16; i++)
    {
        temp = ch[i];
        for (uint8_t j = 0; j < 8; j++)
        {
            if ((temp << j) & 0x80)
                OLED_DrawPixel(x + i - 8, y + j, White);
            else
                OLED_DrawPixel(x + i - 8, y + j, Black);
        }
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
