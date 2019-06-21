#include "xpt2046.h"
#include "ili9341.h"
#include "delay.h"
#include "stdlib.h"
#include "stdio.h"
#include "math.h"
#include "main.h"
//#include "key.h"

//������FLASH����ĵ�ַ�����ַ,ռ��13���ֽ�(RANGE:SAVE_ADDR_BASE~SAVE_ADDR_BASE+12)
#define SAVE_ADDR_BASE 0x08040000

_m_tp_dev tp_dev =
{
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
};
//Ĭ��Ϊtouchtype=0������.
uint8_t CMD_RDX = 0XD0;
uint8_t CMD_RDY = 0X90;

uint16_t IcArr[20][4];

//��ȡָ����ַ���ֽ�(8λ����) 
//faddr:����ַ 
//����ֵ:��Ӧ����.
uint8_t FLASH_ReadOneByte_ILI(uint32_t faddr)
{
	return *(__IO uint8_t*)faddr;
}

//��ȡĳ����ַ���ڵ�flash����
//addr:flash��ַ
//����ֵ:0~11,��addr���ڵ�����
uint8_t FLASH_GetFlashSector_ILI(uint32_t addr)
{
	if (addr < ADDR_FLASH_SECTOR_1_ILI)return FLASH_SECTOR_0;
	else if (addr < ADDR_FLASH_SECTOR_2_ILI)return FLASH_SECTOR_1;
	else if (addr < ADDR_FLASH_SECTOR_3_ILI)return FLASH_SECTOR_2;
	else if (addr < ADDR_FLASH_SECTOR_4_ILI)return FLASH_SECTOR_3;
	else if (addr < ADDR_FLASH_SECTOR_5_ILI)return FLASH_SECTOR_4;
	else if (addr < ADDR_FLASH_SECTOR_6_ILI)return FLASH_SECTOR_5;
	else if (addr < ADDR_FLASH_SECTOR_7_ILI)return FLASH_SECTOR_6;
	else if (addr < ADDR_FLASH_SECTOR_8_ILI)return FLASH_SECTOR_7;
	else if (addr < ADDR_FLASH_SECTOR_9_ILI)return FLASH_SECTOR_8;
	else if (addr < ADDR_FLASH_SECTOR_10_ILI)return FLASH_SECTOR_9;
	else if (addr < ADDR_FLASH_SECTOR_11_ILI)return FLASH_SECTOR_10;
	return FLASH_SECTOR_11;
}

//��ָ����ַ��ʼд��ָ�����ȵ�����
//�ر�ע��:��ΪSTM32F4������ʵ��̫��,û�취���ر�����������,���Ա�����
//         д��ַ�����0XFF,��ô���Ȳ������������Ҳ�������������.����
//         д��0XFF�ĵ�ַ,�����������������ݶ�ʧ.����д֮ǰȷ��������
//         û����Ҫ����,��������������Ȳ�����,Ȼ����������д. 
//�ú�����OTP����Ҳ��Ч!��������дOTP��!
//OTP�����ַ��Χ:0X1FFF7800~0X1FFF7A0F(ע�⣺���16�ֽڣ�����OTP���ݿ�����������д����)
//WriteAddr:��ʼ��ַ(�˵�ַ����Ϊ4�ı���!!)
//pBuffer:����ָ��
//NumToWrite:�ֽ�(8λ)��(����Ҫд���8λ���ݵĸ���.) 
void FLASH_WritexByte_ILI(uint32_t WriteAddr, uint8_t* pBuffer, uint32_t NumToWrite)
{
	FLASH_EraseInitTypeDef FlashEraseInit;
	HAL_StatusTypeDef FlashStatus = HAL_OK;
	uint32_t SectorError = 0;
	uint32_t addrx = 0;
	uint32_t endaddr = 0;

	if (WriteAddr < STM32_FLASH_BASE_ILI)return;	//�Ƿ���ַ

	HAL_FLASH_Unlock();             //����	
	addrx = WriteAddr;				//д�����ʼ��ַ
	endaddr = WriteAddr + NumToWrite;	//д��Ľ�����ַ

	FlashStatus = FLASH_WaitForLastOperation(FLASH_WAITETIME_ILI);            //�ȴ��ϴβ������
	if (FlashStatus == HAL_OK)
	{
		while (WriteAddr < endaddr)//д����
		{
			if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, WriteAddr, *pBuffer) != HAL_OK)//д������
			{
				break;	//д���쳣
			}
			WriteAddr++;
			pBuffer++;
		}
	}
	HAL_FLASH_Lock();           //����
}

//��ָ����ַ��ʼ����ָ�����ȵ�����
//ReadAddr:��ʼ��ַ
//pBuffer:����ָ��
//NumToRead:�ֽ�(8λ)��
void FLASH_ReadxByte_ILI(uint32_t ReadAddr, uint8_t* pBuffer, uint32_t NumToRead)
{
	for (uint32_t i = 0; i < NumToRead; i++)
	{
		pBuffer[i] = FLASH_ReadOneByte_ILI(ReadAddr);//��ȡ1���ֽ�.
		ReadAddr++;//ƫ��1���ֽ�.	
	}
}

float float_abs(float x)
{
	if (x < 0)
		return -x;
	return x;
}

//SPIд����
//������ICд��1byte����
//num:Ҫд�������
void TP_Write_Byte(uint8_t num)
{
	uint8_t count = 0;
	for (count = 0; count < 8; count++)
	{
		if (num & 0x80)
			HAL_GPIO_WritePin(T_MOSI_GPIO_Port, T_MOSI_Pin, GPIO_PIN_SET);
		else
			HAL_GPIO_WritePin(T_MOSI_GPIO_Port, T_MOSI_Pin, GPIO_PIN_RESET);
		num <<= 1;
		HAL_GPIO_WritePin(T_SCK_GPIO_Port, T_SCK_Pin, GPIO_PIN_RESET);
		delay_us(1);
		HAL_GPIO_WritePin(T_SCK_GPIO_Port, T_SCK_Pin, GPIO_PIN_SET); //��������Ч
	}
}
//SPI������
//�Ӵ�����IC��ȡadcֵ
//CMD:ָ��
//����ֵ:����������
uint16_t TP_Read_AD(uint8_t CMD)
{
	uint8_t count = 0;
	uint16_t Num = 0;
	HAL_GPIO_WritePin(T_SCK_GPIO_Port, T_SCK_Pin, GPIO_PIN_RESET);   //������ʱ��
	HAL_GPIO_WritePin(T_MOSI_GPIO_Port, T_MOSI_Pin, GPIO_PIN_RESET); //����������
	HAL_GPIO_WritePin(T_CS_GPIO_Port, T_CS_Pin, GPIO_PIN_RESET);     //ѡ�д�����IC
	TP_Write_Byte(CMD);                                              //����������
	delay_us(6);                                                     //ADS7846��ת��ʱ���Ϊ6us
	HAL_GPIO_WritePin(T_SCK_GPIO_Port, T_SCK_Pin, GPIO_PIN_RESET);
	delay_us(1);
	HAL_GPIO_WritePin(T_SCK_GPIO_Port, T_SCK_Pin, GPIO_PIN_SET); //��1��ʱ�ӣ����BUSY
	delay_us(1);
	HAL_GPIO_WritePin(T_SCK_GPIO_Port, T_SCK_Pin, GPIO_PIN_RESET);
	for (count = 0; count < 16; count++) //����16λ����,ֻ�и�12λ��Ч
	{
		Num <<= 1;
		HAL_GPIO_WritePin(T_SCK_GPIO_Port, T_SCK_Pin, GPIO_PIN_RESET); //�½�����Ч
		delay_us(1);
		HAL_GPIO_WritePin(T_SCK_GPIO_Port, T_SCK_Pin, GPIO_PIN_SET);
		if (HAL_GPIO_ReadPin(T_MISO_GPIO_Port, T_MISO_Pin) == GPIO_PIN_SET)
			Num++;
	}
	Num >>= 4;                                                 //ֻ�и�12λ��Ч.
	HAL_GPIO_WritePin(T_CS_GPIO_Port, T_CS_Pin, GPIO_PIN_SET); //�ͷ�Ƭѡ
	return (Num);
}
//��ȡһ������ֵ(x����y)
//������ȡREAD_TIMES������,����Щ������������,
//Ȼ��ȥ����ͺ����LOST_VAL����,ȡƽ��ֵ
//xy:ָ�CMD_RDX/CMD_RDY��
//����ֵ:����������
#define READ_TIMES 5 //��ȡ����
#define LOST_VAL 1   //����ֵ
uint16_t TP_Read_XOY(uint8_t xy)
{
	uint16_t i, j;
	uint16_t buf[READ_TIMES];
	uint16_t sum = 0;
	uint16_t temp;
	for (i = 0; i < READ_TIMES; i++)
		buf[i] = TP_Read_AD(xy);
	for (i = 0; i < READ_TIMES - 1; i++) //����
	{
		for (j = i + 1; j < READ_TIMES; j++)
		{
			if (buf[i] > buf[j]) //��������
			{
				temp = buf[i];
				buf[i] = buf[j];
				buf[j] = temp;
			}
		}
	}
	sum = 0;
	for (i = LOST_VAL; i < READ_TIMES - LOST_VAL; i++)
		sum += buf[i];
	temp = sum / (READ_TIMES - 2 * LOST_VAL);
	return temp;
}
//��ȡx,y����
//��Сֵ��������100.
//x,y:��ȡ��������ֵ
//����ֵ:0,ʧ��;1,�ɹ���
uint8_t TP_Read_XY(uint16_t * x, uint16_t * y)
{
	uint16_t xtemp, ytemp;
	xtemp = TP_Read_XOY(CMD_RDX);
	ytemp = TP_Read_XOY(CMD_RDY);
	//if(xtemp<100||ytemp<100)return 0;//����ʧ��
	*x = xtemp;
	*y = ytemp;
	return 1; //�����ɹ�
}
//����2�ζ�ȡ������IC,�������ε�ƫ��ܳ���
//ERR_RANGE,��������,����Ϊ������ȷ,�����������.
//�ú����ܴ�����׼ȷ��
//x,y:��ȡ��������ֵ
//����ֵ:0,ʧ��;1,�ɹ���
#define ERR_RANGE 50 //��Χ
uint8_t TP_Read_XY2(uint16_t * x, uint16_t * y)
{
	uint16_t x1, y1;
	uint16_t x2, y2;
	uint8_t flag;
	flag = TP_Read_XY(&x1, &y1);
	if (flag == 0)
		return (0);
	flag = TP_Read_XY(&x2, &y2);
	if (flag == 0)
		return (0);
	if (((x2 <= x1 && x1 < x2 + ERR_RANGE) || (x1 <= x2 && x2 < x1 + ERR_RANGE)) //ǰ�����β�����+-50��
		&& ((y2 <= y1 && y1 < y2 + ERR_RANGE) || (y1 <= y2 && y2 < y1 + ERR_RANGE)))
	{
		*x = (x1 + x2) / 2;
		*y = (y1 + y2) / 2;
		return 1;
	}
	else
		return 0;
}
//////////////////////////////////////////////////////////////////////////////////
//��LCD�����йصĺ���
//��һ��������
//����У׼�õ�
//x,y:����
//color:��ɫ
void TP_Drow_Touch_Point(uint16_t x, uint16_t y, uint16_t color)
{
	POINT_COLOR = color;
	LCD_DrawLine(x - 12, y, x + 13, y); //����
	LCD_DrawLine(x, y - 12, x, y + 13); //����
	LCD_DrawPoint(x + 1, y + 1);
	LCD_DrawPoint(x - 1, y + 1);
	LCD_DrawPoint(x + 1, y - 1);
	LCD_DrawPoint(x - 1, y - 1);
	LCD_Draw_Circle(x, y, 6); //������Ȧ
}
//��һ�����(2*2�ĵ�)
//x,y:����
//color:��ɫ
void TP_Draw_Big_Point(uint16_t x, uint16_t y, uint16_t color)
{
	POINT_COLOR = color;
	LCD_DrawPoint(x, y); //���ĵ�
	LCD_DrawPoint(x + 1, y);
	LCD_DrawPoint(x, y + 1);
	LCD_DrawPoint(x + 1, y + 1);
}
//////////////////////////////////////////////////////////////////////////////////
//��������ɨ��
//tp:0,��Ļ����;1,��������(У׼�����ⳡ����)
//����ֵ:��ǰ����״̬.
//0,�����޴���;1,�����д���
uint8_t TOUCH_Scan(uint8_t tp)
{
	if (HAL_GPIO_ReadPin(T_PEN_GPIO_Port, T_PEN_Pin) == GPIO_PIN_RESET) //�а�������
	{
		if (tp)
			TP_Read_XY2(&tp_dev.x[0], &tp_dev.y[0]);        //��ȡ��������
		else if (TP_Read_XY2(&tp_dev.x[0], &tp_dev.y[0])) //��ȡ��Ļ����
		{
			tp_dev.x[0] = tp_dev.xfac * tp_dev.x[0] + tp_dev.xoff; //�����ת��Ϊ��Ļ����
			tp_dev.y[0] = tp_dev.yfac * tp_dev.y[0] + tp_dev.yoff;
		}
		if ((tp_dev.sta & TP_PRES_DOWN) == 0) //֮ǰû�б�����
		{
			tp_dev.sta = TP_PRES_DOWN | TP_CATH_PRES; //��������
			tp_dev.x[4] = tp_dev.x[0];                //��¼��һ�ΰ���ʱ������
			tp_dev.y[4] = tp_dev.y[0];
		}
	}
	else
	{
		if (tp_dev.sta & TP_PRES_DOWN) //֮ǰ�Ǳ����µ�
		{
			tp_dev.sta &= ~(1 << 7); //��ǰ����ɿ�
		}
		else //֮ǰ��û�б�����
		{
			tp_dev.x[4] = 0;
			tp_dev.y[4] = 0;
			tp_dev.x[0] = 0xffff;
			tp_dev.y[0] = 0xffff;
		}
	}
	return tp_dev.sta& TP_PRES_DOWN; //���ص�ǰ�Ĵ���״̬
}
//////////////////////////////////////////////////////////////////////////
//����У׼����
void TP_Save_Adjdata(void)
{
	int32_t temp;
	//����У�����!
	temp = tp_dev.xfac * 100000000; //����xУ������
	FLASH_WritexByte_ILI(SAVE_ADDR_BASE, (uint8_t*)& temp, 4);
	temp = tp_dev.yfac * 100000000; //����yУ������
	FLASH_WritexByte_ILI(SAVE_ADDR_BASE + 4, (uint8_t*)& temp, 4);
	//����xƫ����
	FLASH_WritexByte_ILI(SAVE_ADDR_BASE + 8, (uint8_t*)& tp_dev.xoff, 2);
	//����yƫ����
	FLASH_WritexByte_ILI(SAVE_ADDR_BASE + 10, (uint8_t*)& tp_dev.yoff, 2);
	//���津������
	FLASH_WritexByte_ILI(SAVE_ADDR_BASE + 12, (uint8_t*)& tp_dev.touchtype, 1);
	temp = 0X0A; //���У׼����
	FLASH_WritexByte_ILI(SAVE_ADDR_BASE + 13, (uint8_t*)& temp, 1);
}
//�õ�������EEPROM�����У׼ֵ
//����ֵ��1���ɹ���ȡ����
//        0����ȡʧ�ܣ�Ҫ����У׼
uint8_t TP_Get_Adjdata(void)
{
	int32_t tempfac;
	tempfac = 0;
	FLASH_ReadxByte_ILI(SAVE_ADDR_BASE + 13, (uint8_t*)& tempfac, 1); //��ȡ�����,���Ƿ�У׼����
	if (tempfac == 0X0A)                                          //�������Ѿ�У׼����
	{
		FLASH_ReadxByte_ILI(SAVE_ADDR_BASE, (uint8_t*)& tempfac, 4);
		tp_dev.xfac = (float)tempfac / 100000000; //�õ�xУ׼����
		FLASH_ReadxByte_ILI(SAVE_ADDR_BASE + 4, (uint8_t*)& tempfac, 4);
		tp_dev.yfac = (float)tempfac / 100000000; //�õ�yУ׼����
			//�õ�xƫ����
		FLASH_ReadxByte_ILI(SAVE_ADDR_BASE + 8, (uint8_t*)& tp_dev.xoff, 2);
		//�õ�yƫ����
		FLASH_ReadxByte_ILI(SAVE_ADDR_BASE + 10, (uint8_t*)& tp_dev.yoff, 2);
		FLASH_ReadxByte_ILI(SAVE_ADDR_BASE + 12, (uint8_t*)& tp_dev.touchtype, 1); //��ȡ�������ͱ��
		if (tp_dev.touchtype)                                                  //X,Y��������Ļ�෴
		{
			CMD_RDX = 0X90;
			CMD_RDY = 0XD0;
		}
		else //X,Y��������Ļ��ͬ
		{
			CMD_RDX = 0XD0;
			CMD_RDY = 0X90;
		}
		return 1;
	}
	return 0;
}
//��ʾ�ַ���
uint8_t* const TP_REMIND_MSG_TBL = "Please use the stylus click the cross on the screen.The cross will always move until the screen adjustment is completed.";

//��ʾУ׼���(��������)
void TP_Adj_Info_Show(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3, uint16_t fac)
{
	POINT_COLOR = RED;
	LCD_ShowString(40, 160, lcddev.width, lcddev.height, 16, "x1:");
	LCD_ShowString(40 + 80, 160, lcddev.width, lcddev.height, 16, "y1:");
	LCD_ShowString(40, 180, lcddev.width, lcddev.height, 16, "x2:");
	LCD_ShowString(40 + 80, 180, lcddev.width, lcddev.height, 16, "y2:");
	LCD_ShowString(40, 200, lcddev.width, lcddev.height, 16, "x3:");
	LCD_ShowString(40 + 80, 200, lcddev.width, lcddev.height, 16, "y3:");
	LCD_ShowString(40, 220, lcddev.width, lcddev.height, 16, "x4:");
	LCD_ShowString(40 + 80, 220, lcddev.width, lcddev.height, 16, "y4:");
	LCD_ShowString(40, 240, lcddev.width, lcddev.height, 16, "fac is:");
	LCD_ShowNum(40 + 24, 160, x0, 4, 16);      //��ʾ��ֵ
	LCD_ShowNum(40 + 24 + 80, 160, y0, 4, 16); //��ʾ��ֵ
	LCD_ShowNum(40 + 24, 180, x1, 4, 16);      //��ʾ��ֵ
	LCD_ShowNum(40 + 24 + 80, 180, y1, 4, 16); //��ʾ��ֵ
	LCD_ShowNum(40 + 24, 200, x2, 4, 16);      //��ʾ��ֵ
	LCD_ShowNum(40 + 24 + 80, 200, y2, 4, 16); //��ʾ��ֵ
	LCD_ShowNum(40 + 24, 220, x3, 4, 16);      //��ʾ��ֵ
	LCD_ShowNum(40 + 24 + 80, 220, y3, 4, 16); //��ʾ��ֵ
	LCD_ShowNum(40 + 56, 240, fac, 3, 16);     //��ʾ��ֵ,����ֵ������95~105��Χ֮��.
}

//������У׼����
//�õ��ĸ�У׼����
void TP_Adjust(void)
{
	uint16_t pos_temp[4][2]; //���껺��ֵ
	uint8_t cnt = 0;
	uint16_t d1, d2;
	uint32_t tem1, tem2;
	double fac;
	uint16_t outtime = 0;
	cnt = 0;
	POINT_COLOR = BLUE;
	BACK_COLOR = WHITE;
	LCD_Clear(WHITE);  //����
	POINT_COLOR = RED; //��ɫ
	LCD_Clear(WHITE);  //����
	POINT_COLOR = BLACK;
	LCD_ShowString(40, 40, 160, 100, 16, (uint8_t*)TP_REMIND_MSG_TBL); //��ʾ��ʾ��Ϣ
	TP_Drow_Touch_Point(20, 20, RED);                                   //����1
	tp_dev.sta = 0;                                                     //���������ź�
	tp_dev.xfac = 0;                                                    //xfac��������Ƿ�У׼��,����У׼֮ǰ�������!�������
	while (1)                                                           //�������10����û�а���,���Զ��˳�
	{
		TOUCH_Scan(1);                              //ɨ����������
		if ((tp_dev.sta & 0xc0) == TP_CATH_PRES) //����������һ��(��ʱ�����ɿ���.)
		{
			outtime = 0;
			tp_dev.sta &= ~(1 << 6); //��ǰ����Ѿ����������.

			pos_temp[cnt][0] = tp_dev.x[0];
			pos_temp[cnt][1] = tp_dev.y[0];
			cnt++;
			switch (cnt)
			{
			case 1:
				TP_Drow_Touch_Point(20, 20, WHITE);              //�����1
				TP_Drow_Touch_Point(lcddev.width - 20, 20, RED); //����2
				break;
			case 2:
				TP_Drow_Touch_Point(lcddev.width - 20, 20, WHITE); //�����2
				TP_Drow_Touch_Point(20, lcddev.height - 20, RED);  //����3
				break;
			case 3:
				TP_Drow_Touch_Point(20, lcddev.height - 20, WHITE);              //�����3
				TP_Drow_Touch_Point(lcddev.width - 20, lcddev.height - 20, RED); //����4
				break;
			case 4:                                        //ȫ���ĸ����Ѿ��õ�
														   //�Ա����
				tem1 = abs(pos_temp[0][0] - pos_temp[1][0]); //x1-x2
				tem2 = abs(pos_temp[0][1] - pos_temp[1][1]); //y1-y2
				tem1 *= tem1;
				tem2 *= tem2;
				d1 = sqrt(tem1 + tem2); //�õ�1,2�ľ���

				tem1 = abs(pos_temp[2][0] - pos_temp[3][0]); //x3-x4
				tem2 = abs(pos_temp[2][1] - pos_temp[3][1]); //y3-y4
				tem1 *= tem1;
				tem2 *= tem2;
				d2 = sqrt(tem1 + tem2); //�õ�3,4�ľ���
				fac = (float)d1 / d2;
				if (fac < 0.95 || fac > 1.05 || d1 == 0 || d2 == 0) //���ϸ�
				{
					cnt = 0;
					TP_Drow_Touch_Point(lcddev.width - 20, lcddev.height - 20, WHITE);                                                                                           //�����4
					TP_Drow_Touch_Point(20, 20, RED);                                                                                                                            //����1
					TP_Adj_Info_Show(pos_temp[0][0], pos_temp[0][1], pos_temp[1][0], pos_temp[1][1], pos_temp[2][0], pos_temp[2][1], pos_temp[3][0], pos_temp[3][1], fac * 100); //��ʾ����
					continue;
				}
				tem1 = abs(pos_temp[0][0] - pos_temp[2][0]); //x1-x3
				tem2 = abs(pos_temp[0][1] - pos_temp[2][1]); //y1-y3
				tem1 *= tem1;
				tem2 *= tem2;
				d1 = sqrt(tem1 + tem2); //�õ�1,3�ľ���

				tem1 = abs(pos_temp[1][0] - pos_temp[3][0]); //x2-x4
				tem2 = abs(pos_temp[1][1] - pos_temp[3][1]); //y2-y4
				tem1 *= tem1;
				tem2 *= tem2;
				d2 = sqrt(tem1 + tem2); //�õ�2,4�ľ���
				fac = (float)d1 / d2;
				if (fac < 0.95 || fac > 1.05) //���ϸ�
				{
					cnt = 0;
					TP_Drow_Touch_Point(lcddev.width - 20, lcddev.height - 20, WHITE);                                                                                           //�����4
					TP_Drow_Touch_Point(20, 20, RED);                                                                                                                            //����1
					TP_Adj_Info_Show(pos_temp[0][0], pos_temp[0][1], pos_temp[1][0], pos_temp[1][1], pos_temp[2][0], pos_temp[2][1], pos_temp[3][0], pos_temp[3][1], fac * 100); //��ʾ����
					continue;
				} //��ȷ��

				//�Խ������
				tem1 = abs(pos_temp[1][0] - pos_temp[2][0]); //x1-x3
				tem2 = abs(pos_temp[1][1] - pos_temp[2][1]); //y1-y3
				tem1 *= tem1;
				tem2 *= tem2;
				d1 = sqrt(tem1 + tem2); //�õ�1,4�ľ���

				tem1 = abs(pos_temp[0][0] - pos_temp[3][0]); //x2-x4
				tem2 = abs(pos_temp[0][1] - pos_temp[3][1]); //y2-y4
				tem1 *= tem1;
				tem2 *= tem2;
				d2 = sqrt(tem1 + tem2); //�õ�2,3�ľ���
				fac = (float)d1 / d2;
				if (fac < 0.95 || fac > 1.05) //���ϸ�
				{
					cnt = 0;
					TP_Drow_Touch_Point(lcddev.width - 20, lcddev.height - 20, WHITE);                                                                                           //�����4
					TP_Drow_Touch_Point(20, 20, RED);                                                                                                                            //����1
					TP_Adj_Info_Show(pos_temp[0][0], pos_temp[0][1], pos_temp[1][0], pos_temp[1][1], pos_temp[2][0], pos_temp[2][1], pos_temp[3][0], pos_temp[3][1], fac * 100); //��ʾ����
					continue;
				} //��ȷ��
				//������
				tp_dev.xfac = (float)(lcddev.width - 40) / (pos_temp[1][0] - pos_temp[0][0]);       //�õ�xfac
				tp_dev.xoff = (lcddev.width - tp_dev.xfac * (pos_temp[1][0] + pos_temp[0][0])) / 2; //�õ�xoff

				tp_dev.yfac = (float)(lcddev.height - 40) / (pos_temp[2][1] - pos_temp[0][1]);       //�õ�yfac
				tp_dev.yoff = (lcddev.height - tp_dev.yfac * (pos_temp[2][1] + pos_temp[0][1])) / 2; //�õ�yoff
				if (float_abs(tp_dev.xfac) > 2 || float_abs(tp_dev.yfac) > 2)                                    //������Ԥ����෴��.
				{
					cnt = 0;
					TP_Drow_Touch_Point(lcddev.width - 20, lcddev.height - 20, WHITE); //�����4
					TP_Drow_Touch_Point(20, 20, RED);                                  //����1
					LCD_ShowString(40, 26, lcddev.width, lcddev.height, 16, "TP Need readjust!");
					tp_dev.touchtype = !tp_dev.touchtype; //�޸Ĵ�������.
					if (tp_dev.touchtype)                 //X,Y��������Ļ�෴
					{
						CMD_RDX = 0X90;
						CMD_RDY = 0XD0;
					}
					else //X,Y��������Ļ��ͬ
					{
						CMD_RDX = 0XD0;
						CMD_RDY = 0X90;
					}
					continue;
				}
				POINT_COLOR = BLUE;
				LCD_Clear(WHITE);                                                                    //����
				LCD_ShowString(35, 110, lcddev.width, lcddev.height, 16, "Touch Screen Adjust OK!"); //У�����
				delay_ms(1000);
				TP_Save_Adjdata();
				LCD_Clear(WHITE); //����
				return;           //У�����
			}
		}
		delay_ms(10);
		outtime++;
		if (outtime > 1000)
		{
			TP_Get_Adjdata();
			break;
		}
	}
}
//��������ʼ��
//����ֵ:0,û�н���У׼
//       1,���й�У׼
uint8_t TOUCH_Init(void)
{
	if (TP_Get_Adjdata())
		return 0; //�Ѿ�У׼
	else        //δУ׼
	{
		LCD_Clear(WHITE); //����
		TP_Adjust();      //��ĻУ׼
		TP_Save_Adjdata();
	}
	TP_Get_Adjdata();

	return 1;
}

//�����Ļ�������Ͻ���ʾ"RST"
void Load_Drow_Dialog(void)
{
	LCD_Clear(WHITE);                                         //����
	POINT_COLOR = RED;                                        //���û�����ɫ
}

void Screen_Draw_Line(void)
{
	TOUCH_Scan(0);
	if (tp_dev.sta & TP_PRES_DOWN) //������������
	{
		if (tp_dev.x[0] < lcddev.width && tp_dev.y[0] < lcddev.height)
		{
			if (tp_dev.x[0] > (lcddev.width - 24) && tp_dev.y[0] < 16)
				Load_Drow_Dialog(); //���
			else
				TP_Draw_Big_Point(tp_dev.x[0], tp_dev.y[0], RED); //��ͼ
		}
	}
	else
		delay_ms(10);       //û�а������µ�ʱ��
}

//��ƴ�������
uint8_t LCD_Feedback(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, float* result, uint8_t size)
{
	static uint8_t touch_up = 1;
	static uint8_t i = 0;              //i��¼������Ԫ�ظ���
	uint8_t j;
	static uint8_t str[20] = { '\0' }; //�ַ�������

	uint16_t x = x2 - x1; //���εĿ�ͳ�
	uint16_t y = y2 - y1;
	TOUCH_Scan(0);
	/*��������*/
	if (touch_up && (tp_dev.x[0] != 0xFFFF))
	{
		delay_ms(10);
		touch_up = 0;

		if (tp_dev.x[0] > (lcddev.width - 24) && tp_dev.y[0] < 20)
		{
			LCD_Clear(WHITE);
			LCD_Draw_Keyboard(x1, y1, x2, y2, size);
			i = 0;
		}
		//�жϼ�ֵ
		if (tp_dev.x[0] > x1 && tp_dev.x[0] < x2 && tp_dev.y[0] < y2 && tp_dev.y[0] > y1)
		{
			i++;
			//����1
			if (tp_dev.x[0] > x1 && tp_dev.x[0] < (x1 + x / 3) && tp_dev.y[0] > y1 && tp_dev.y[0] < (y1 + y / 5))
			{
				str[i - 1] = '1';
				LCD_ShowChar(x1 + 5 + size * i / 2 + x / 3, 9 * y / 10 - size / 2 + y1, '1', size, 0);
			}
			//����2
			else if (tp_dev.x[0] > (x1 + x / 3) && tp_dev.x[0] < (x1 + 2 * x / 3) && tp_dev.y[0] > y1 && tp_dev.y[0] < (y1 + y / 5))
			{
				str[i - 1] = '2';
				LCD_ShowChar(x1 + 5 + size * i / 2 + x / 3, 9 * y / 10 - size / 2 + y1, '2', size, 0);
			}
			//����3
			else if (tp_dev.x[0] > (x1 + 2 * x / 3) && tp_dev.x[0] < (x1 + x) && tp_dev.y[0] > y1 && tp_dev.y[0] < (y1 + y / 5))
			{
				str[i - 1] = '3';
				LCD_ShowChar(x1 + 5 + size * i / 2 + x / 3, 9 * y / 10 - size / 2 + y1, '3', size, 0);
			}
			//����4
			else if (tp_dev.x[0] > (x1) && tp_dev.x[0] < (x1 + x / 3) && tp_dev.y[0] > (y1 + y / 5) && tp_dev.y[0] < (y1 + 2 * y / 5))
			{
				str[i - 1] = '4';
				LCD_ShowChar(x1 + 5 + size * i / 2 + x / 3, 9 * y / 10 - size / 2 + y1, '4', size, 0);
			}
			//����5
			else if (tp_dev.x[0] > (x1 + x / 3) && tp_dev.x[0] < (x1 + 2 * x / 3) && tp_dev.y[0] > (y1 + y / 5) && tp_dev.y[0] < (y1 + 2 * y / 5))
			{
				str[i - 1] = '5';
				LCD_ShowChar(x1 + 5 + size * i / 2 + x / 3, 9 * y / 10 - size / 2 + y1, '5', size, 0);
			}
			//����6
			else if (tp_dev.x[0] > (x1 + 2 * x / 3) && tp_dev.x[0] < (x1 + x) && tp_dev.y[0] > (y1 + y / 5) && tp_dev.y[0] < (y1 + 2 * y / 5))
			{
				str[i - 1] = '6';
				LCD_ShowChar(x1 + 5 + size * i / 2 + x / 3, 9 * y / 10 - size / 2 + y1, '6', size, 0);
			}
			//����7
			else if (tp_dev.x[0] > (x1) && tp_dev.x[0] < (x1 + x / 3) && tp_dev.y[0] > (y1 + 2 * y / 5) && tp_dev.y[0] < (y1 + 3 * y / 5))
			{
				str[i - 1] = '7';
				LCD_ShowChar(x1 + 5 + size * i / 2 + x / 3, 9 * y / 10 - size / 2 + y1, '7', size, 0);
			}
			//����8
			else if (tp_dev.x[0] > (x1 + x / 3) && tp_dev.x[0] < (x1 + 2 * x / 3) && tp_dev.y[0] > (y1 + 2 * y / 5) && tp_dev.y[0] < (y1 + 3 * y / 5))
			{
				str[i - 1] = '8';
				LCD_ShowChar(x1 + 5 + size * i / 2 + x / 3, 9 * y / 10 - size / 2 + y1, '8', size, 0);
			}
			//����9
			else if (tp_dev.x[0] > (x1 + 2 * x / 3) && tp_dev.x[0] < (x1 + x) && tp_dev.y[0] > (y1 + 2 * y / 5) && tp_dev.y[0] < (y1 + 3 * y / 5))
			{
				str[i - 1] = '9';
				LCD_ShowChar(x1 + 5 + size * i / 2 + x / 3, 9 * y / 10 - size / 2 + y1, '9', size, 0);
			}
			//����.
			else if (tp_dev.x[0] > (x1) && tp_dev.x[0] < (x1 + x / 3) && tp_dev.y[0] > (y1 + 3 * y / 5) && tp_dev.y[0] < (y1 + 4 * y / 5))
			{
				str[i - 1] = '.';
				LCD_ShowChar(x1 + 5 + size * i / 2 + x / 3, 9 * y / 10 - size / 2 + y1, '.', size, 0);
			}
			//����0
			else if (tp_dev.x[0] > (x1 + x / 3) && tp_dev.x[0] < (x1 + 2 * x / 3) && tp_dev.y[0] > (y1 + 3 * y / 5) && tp_dev.y[0] < (y1 + 4 * y / 5))
			{
				str[i - 1] = '0';
				LCD_ShowChar(x1 + 5 + size * i / 2 + x / 3, 9 * y / 10 - size / 2 + y1, '0', size, 0);
			}
			//����back
			else if (tp_dev.x[0] > (x1 + 2 * x / 3) && tp_dev.x[0] < (x1 + x) && tp_dev.y[0] > (y1 + 3 * y / 5) && tp_dev.y[0] < (y1 + 4 * y / 5))
			{
				LCD_ShowChar(x1 + 5 + size * (i - 1) / 2 + x / 3, 9 * y / 10 - size / 2 + y1, ' ', size, 0);
				if (i >= 2)
					i -= 2;
				else
					i -= 1;
				str[i] = '\0';
			}
			//����-		
			else if (tp_dev.x[0] > (x1) && tp_dev.x[0]<(x1 + x / 3) && tp_dev.y[0]>(y1 + 4 * y / 5) && tp_dev.y[0] < (y1 + 5 * y / 5))
			{
				str[i - 1] = '-';
				LCD_ShowChar(x1 + 5 + size * i / 2 + x / 3, 9 * y / 10 - size / 2 + y1, '-', size, 0);

			}
			//����enter
			//����enter������������ĸ���������ʾ�������������Ļ����
			else if (tp_dev.x[0] > (x1 + x / 3) && tp_dev.x[0] < (x1 + x) && tp_dev.y[0] > (y1 + 4 * y / 5) && tp_dev.y[0] < (y1 + 5 * y / 5))
			{
				*result = atof(str);
				for (j = 1; j < i + 1; j++)
				{
					LCD_ShowChar(x1 + 5 + size * (j - 1) / 2 + x / 3, 9 * y / 10 - size / 2 + y1, ' ', size, 0);
				}
				for (j = 0; j < 20; j++)
					str[j] = '\0';
				//״̬�жϱ�����λ
				i = 0;
				return 1;
			}

		}
	}
	else if (tp_dev.x[0] == 0xFFFF)touch_up = 1;

	return 0;
}

uint8_t IC_Feedback(uint8_t num)
{
	static uint8_t cs = -1;
	TOUCH_Scan(0);
	for (uint8_t i = 0; i < num; i++)
		if (tp_dev.x[0] > IcArr[i][0] && tp_dev.x[0] < IcArr[i][2] && tp_dev.y[0] > IcArr[i][1] && tp_dev.y[0] < IcArr[i][3] && cs != i)
			cs = i;
	return cs;
}

//����Ƭѡ����
/*
�����ַ�������
*/

uint16_t LCD_ICRec(uint16_t x, uint16_t y, uint16_t xmax, uint8_t str[][20], uint8_t num, uint8_t size)
{
	uint8_t* p;
	uint8_t count = 0;
	uint16_t len;
	p = str[num];
	while (*p != '\0')
	{
		count++;
		p++;
	}
	len = size * count / 2;
	if (x + len + size + 5 > xmax)
		return 0xFFFF;
	LCD_DrawRectangle(x, y, x + len + size + 5, y + 2 * size + 5);
	IcArr[num][0] = x;
	IcArr[num][1] = y;
	IcArr[num][2] = x + len + size + 5;
	IcArr[num][3] = y + 2 * size + 5;
	LCD_ShowString(x + (size + 5) / 2, y + (size + 5) / 2, len, size, size, str[num]);
	return (x + len + size + 5);
}
/*
��ڲ���:
x,y:LCD����
str:��ʾ���ַ���
len:�ַ�������
*/
void LCD_ICSel(uint16_t x, uint16_t y, uint16_t xmax, uint8_t str[][20], uint8_t len, uint8_t cs, uint8_t size)
{
	uint8_t icnum = 0;
	uint16_t x0 = x;
	for (uint8_t i = 0; i < len; i++)
	{
		if (cs == i)
			POINT_COLOR = RED;
		x = LCD_ICRec(x, y, xmax, str, i, size);
		if (x == 0xFFFF) {
			y += 2 * size + 5;
			x = x0;
			x = LCD_ICRec(x, y, xmax, str, i, size);
		}
		if (cs == i)
			POINT_COLOR = BLACK;
	}
}
//����ͼ�ν���
void LCD_Draw_Keyboard(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t size)
{
	uint8_t buff[] = { '1','2','3','4','5','6','7','8','9','.','0',' ','-',' ',' ' };
	POINT_COLOR = BLACK;
	uint16_t x = x2 - x1;
	uint16_t y = y2 - y1;

	LCD_DrawRectangle(x1, y1, x2, y2);
	for (uint8_t i = 1; i < 5; i++)
		LCD_DrawLine(x1, i * y / 5 + y1, x2, i * y / 5 + y1);
	//������
	for (uint8_t i = 1; i < 2; i++)
		LCD_DrawLine(i * x / 3 + x1, y1, i * x / 3 + x1, y + y1);
	//�������
	for (uint8_t i = 2; i < 3; i++)
		LCD_DrawLine(i * x / 3 + x1, y1, i * x / 3 + x1, 4 * y / 5 + y1);

	uint8_t k = 0;
	for (uint8_t i = 0; i < 5; i++)
	{
		for (uint8_t j = 0; j < 3; j++)
		{
    		LCD_ShowChar(j * x / 3 + x / 6 - size / 4 + x1, i * y / 5 + y / 10 - size / 2 + y1, buff[k], size, 0);
			k++;
		}
	}
    LCD_ShowString(5 * x / 6 - size + x1, 7 * y / 10 - size / 2 + y1, 2 * size, size, size, "back");
}

void LCD_ShowFloatnum(uint16_t x, uint16_t y, float num, uint8_t size)
{
	char buff[20];
	sprintf(buff, "%.5f", num);
	LCD_ShowString(x, y, 200, size, size, buff);
}
//��ƴ���ȫ������
uint8_t LCD_Feedback_Whole(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, int* temp, uint8_t size)
{
    TOUCH_Scan(0);
    /*��������*/
    //˫����
    if (tp_dev.x[0] > x1 && tp_dev.x[0] < x2 && tp_dev.y[0] > y1 && tp_dev.y[0] < y2)
    {
        (*temp)++; 
        tp_dev.x[0] = 0xFFFF;
    }
    if (tp_dev.x[0] > (x1+75) && tp_dev.x[0] < (x2+75) && tp_dev.y[0] > y1 && tp_dev.y[0] < y2)
    {
        (*temp)--;
        tp_dev.x[0] = 0xFFFF;
    }
    return 0;
}
