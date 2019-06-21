#ifndef __XPT2046_H
#define __XPT2046_H
#include "main.h"

#define TP_PRES_DOWN 0x80  //����������	  
#define TP_CATH_PRES 0x40  //�а��������� 
#define CT_MAX_TOUCH  5    //������֧�ֵĵ���,�̶�Ϊ5��

//FLASH��ʼ��ַ
#define STM32_FLASH_BASE_ILI 0x08000000 	//STM32 FLASH����ʼ��ַ
#define FLASH_WAITETIME_ILI  50000          //FLASH�ȴ���ʱʱ��

//FLASH ��������ʼ��ַ
#define ADDR_FLASH_SECTOR_0_ILI     ((uint32_t)0x08000000) 	//����0��ʼ��ַ, 16 Kbytes  
#define ADDR_FLASH_SECTOR_1_ILI     ((uint32_t)0x08004000) 	//����1��ʼ��ַ, 16 Kbytes  
#define ADDR_FLASH_SECTOR_2_ILI     ((uint32_t)0x08008000) 	//����2��ʼ��ַ, 16 Kbytes  
#define ADDR_FLASH_SECTOR_3_ILI     ((uint32_t)0x0800C000) 	//����3��ʼ��ַ, 16 Kbytes  
#define ADDR_FLASH_SECTOR_4_ILI     ((uint32_t)0x08010000) 	//����4��ʼ��ַ, 64 Kbytes  
#define ADDR_FLASH_SECTOR_5_ILI     ((uint32_t)0x08020000) 	//����5��ʼ��ַ, 128 Kbytes  
#define ADDR_FLASH_SECTOR_6_ILI     ((uint32_t)0x08040000) 	//����6��ʼ��ַ, 128 Kbytes  
#define ADDR_FLASH_SECTOR_7_ILI     ((uint32_t)0x08060000) 	//����7��ʼ��ַ, 128 Kbytes  
#define ADDR_FLASH_SECTOR_8_ILI     ((uint32_t)0x08080000) 	//����8��ʼ��ַ, 128 Kbytes  
#define ADDR_FLASH_SECTOR_9_ILI     ((uint32_t)0x080A0000) 	//����9��ʼ��ַ, 128 Kbytes  
#define ADDR_FLASH_SECTOR_10_ILI    ((uint32_t)0x080C0000) 	//����10��ʼ��ַ,128 Kbytes  
#define ADDR_FLASH_SECTOR_11_ILI    ((uint32_t)0x080E0000) 	//����11��ʼ��ַ,128 Kbytes 

//������������
typedef struct
{
	uint16_t x[CT_MAX_TOUCH]; 		//��ǰ����
	uint16_t y[CT_MAX_TOUCH];		//�����������5������,����������x[0],y[0]����:�˴�ɨ��ʱ,����������,��
								//x[4],y[4]�洢��һ�ΰ���ʱ������. 
	uint8_t  sta;					//�ʵ�״̬ 
								//b7:����1/�ɿ�0; 
								//b6:0,û�а�������;1,�а�������. 
								//b5:����
								//b4~b0:���ݴ��������µĵ���(0,��ʾδ����,1��ʾ����)
/////////////////////������У׼����(����������ҪУ׼)//////////////////////								
	float xfac;
	float yfac;
	short xoff;
	short yoff;
	//�����Ĳ���,��������������������ȫ�ߵ�ʱ��Ҫ�õ�.
	//b0:0,����(�ʺ�����ΪX����,����ΪY�����TP)
	//   1,����(�ʺ�����ΪY����,����ΪX�����TP) 
	//b1~6:����.
	//b7:0,������
	//   1,������ 
	uint8_t touchtype;
}_m_tp_dev;

extern _m_tp_dev tp_dev;	 	//������������touch.c���涨��

extern uint16_t IcArr[20][4];

//����������
void TP_Write_Byte(uint8_t num);						//�����оƬд��һ������
uint16_t TP_Read_AD(uint8_t CMD);							//��ȡADת��ֵ
uint16_t TP_Read_XOY(uint8_t xy);							//���˲��������ȡ(X/Y)
uint8_t TP_Read_XY(uint16_t* x, uint16_t* y);					//˫�����ȡ(X+Y)
uint8_t TP_Read_XY2(uint16_t* x, uint16_t* y);					//����ǿ�˲���˫���������ȡ
void TP_Drow_Touch_Point(uint16_t x, uint16_t y, uint16_t color);//��һ������У׼��
void TP_Draw_Big_Point(uint16_t x, uint16_t y, uint16_t color);	//��һ�����
void TP_Save_Adjdata(void);						//����У׼����
uint8_t TP_Get_Adjdata(void);						//��ȡУ׼����
void TP_Adjust(void);							//������У׼
void TP_Adj_Info_Show(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3, uint16_t fac);//��ʾУ׼��Ϣ
//������/������ ���ú���
uint8_t TOUCH_Scan(uint8_t tp);								//ɨ��
uint8_t TOUCH_Init(void);								//��ʼ��

void Load_Drow_Dialog(void);
void Screen_Draw_Line(void);

uint8_t LCD_Feedback_Whole(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, int* temp, uint8_t size);//ȫ������
uint8_t LCD_Feedback(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, float* result, uint8_t size);
uint8_t IC_Feedback(uint8_t num);
uint16_t LCD_ICRec(uint16_t x, uint16_t y, uint16_t xmax, uint8_t str[][20], uint8_t num, uint8_t size);
void LCD_ICSel(uint16_t x, uint16_t y, uint16_t xmax, uint8_t str[][20], uint8_t len, uint8_t cs, uint8_t size);
void LCD_Draw_Keyboard(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t size);
void LCD_ShowFloatnum(uint16_t x, uint16_t y, float num, uint8_t size);

#endif
