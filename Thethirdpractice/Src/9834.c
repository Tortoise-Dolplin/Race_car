#include"9834.h"
#include"gpio.h"
#include"spi.h"
#include"main.h"
/****************************************************************
��������: AD9834_Write_16Bits
��    ��: ��AD9834д��16Ϊ����
��    ��: data --  Ҫд���16λ����
����ֵ  : ��
*****************************************************************/
void AD9834_Write_16Bits(unsigned int data)
{
    unsigned char i = 0;
    AD9834_SCLK_SET;
    AD9834_FSYNC_CLR;
    for (i = 0; i < 16; i++)
    {
        if (data & 0x8000)
            AD9834_SDATA_SET;
        else
            AD9834_SDATA_CLR;

        AD9834_SCLK_CLR;
        data <<= 1;
        AD9834_SCLK_SET;
    }

    AD9834_FSYNC_SET;
}
/***********************************************************************************
�������ƣ�AD9834_Select_Wave
��    �ܣ�����Ϊ���ƣ�
    --------------------------------------------------
    IOUT���Ҳ� ��SIGNBITOUT���� ��дFREQREG0 ��дPHASE0
    ad9834_write_16bit(0x2028)   һ����дFREQREG0
    ad9834_write_16bit(0x0038)   ������дFREQREG0��LSB
    ad9834_write_16bit(0x1038)   ������дFREQREG0��MSB
    --------------------------------------------------
    IOUT���ǲ� ��дPHASE0
    ad9834_write_16bit(0x2002)   һ����дFREQREG0
    ad9834_write_16bit(0x0002)   ������дFREQREG0��LSB
    ad9834_write_16bit(0x1008)   ������дFREQREG0��MSB
��    ����initdata -- Ҫ���������
����ֵ  ����
************************************************************************************/
void AD9834_Select_Wave(unsigned int initdata)
{
    AD9834_FSYNC_SET;
    AD9834_SCLK_SET;

    AD9834_RESET_SET;
    AD9834_RESET_SET;
    AD9834_RESET_CLR;

    AD9834_Write_16Bits(initdata);
}
/****************************************************************
��������: Init_AD9834
��    ��: ��ʼ��AD9834��������
��    ��: ��
����ֵ  : ��
*****************************************************************/
void Init_AD9834()
{
    GPIO_InitTypeDef GPIO_InitStructure;
    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitStructure.Pin = AD9834_FSYNC | AD9834_SCLK | AD9834_SDATA | AD9834_RESET;
    GPIO_InitStructure.Speed = GPIO_SPEED_FAST;
    GPIO_InitStructure.Pull = GPIO_PULLUP;
    GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
    HAL_GPIO_Init(AD9834_Control_Port, &GPIO_InitStructure);

    //MX_SPI1_Init();
    //SPI1_SetSpeed(SPI_BAUDRATEPRESCALER_2); //����Ϊ42Mʱ��,����ģʽ

}
/****************************************************************
��������: AD9834_Set_Freq
��    ��: ����Ƶ��ֵ
��    ��: freq_number -- Ҫд���Ƶ�ʼĴ���(FREQ_0��FREQ_1)
          freq -- Ƶ��ֵ (Freq_value(value)=Freq_data(data)*FCLK/2^28)
����ֵ  : ��
*****************************************************************/
void AD9834_Set_Freq(unsigned char freq_number, unsigned long freq)
{
    unsigned long FREQREG = (unsigned long)(268435456.0 / AD9834_SYSTEM_COLCK * freq);

    unsigned int FREQREG_LSB_14BIT = (unsigned int)FREQREG;
    unsigned int FREQREG_MSB_14BIT = (unsigned int)(FREQREG >> 14);

    if (freq_number == FREQ_0)
    {
        FREQREG_LSB_14BIT &= ~(1U << 15);
        FREQREG_LSB_14BIT |= 1 << 14;
        FREQREG_MSB_14BIT &= ~(1U << 15);
        FREQREG_MSB_14BIT |= 1 << 14;
    }
    else
    {
        FREQREG_LSB_14BIT &= ~(1 << 14);
        FREQREG_LSB_14BIT |= 1U << 15;
        FREQREG_MSB_14BIT &= ~(1 << 14);
        FREQREG_MSB_14BIT |= 1U << 15;
    }

    AD9834_Write_16Bits(FREQREG_LSB_14BIT);
    AD9834_Write_16Bits(FREQREG_MSB_14BIT);

}
//============================================================//