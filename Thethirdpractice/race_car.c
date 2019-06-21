#include"race_car.h"
#include"game.h"
#include"ili9341.h"
#include"xpt2046.h"
#include"time.h"
#include"main.h"
#include"tim.h"

uint32_t while_round = 0;

char char_game_type[15] = "GAME:RACE CAR";
char char_score[12] = "Your score:";
char char_turn_left[5] = "LEFT";
char char_turn_right[6] = "RIGHT";

extern Moving_car car1, car2, car3, car4, car5, car6, player_car;

//����̬���
void Draw_Still_Frame(void)
{
    //��������
    //һ������
    LCD_DrawLine(60, 0, 60, 17);
    LCD_DrawLine(60, 34, 60, 68);
    LCD_DrawLine(60, 85, 60, 119);
    LCD_DrawLine(60, 136, 60, 170);
    LCD_DrawLine(60, 187, 60, 221);
    LCD_DrawLine(60, 238, 60, 272);
    LCD_DrawLine(60, 289, 60, 320);
    //��������
    LCD_DrawLine(120, 0, 120, 17);
    LCD_DrawLine(120, 34, 120, 68);
    LCD_DrawLine(120, 85, 120, 119);
    LCD_DrawLine(120, 136, 120, 170);
    LCD_DrawLine(120, 187, 120, 221);
    LCD_DrawLine(120, 238, 120, 272);
    LCD_DrawLine(120, 289, 120, 320);
    //��������
    LCD_DrawLine(180, 0, 180, 320);
    LCD_DrawLine(0, 260, 180, 260);                  
    //д��Ϸ���ͼ��÷�
    POINT_COLOR = RED;
    LCD_ShowString(185, 60, 45, 40, 16, char_game_type);
    LCD_ShowString(185, 120, 45, 40, 16, char_score);
    LCD_DrawRectangle(0, 260, 60, 320);
    LCD_DrawRectangle(120, 260, 180, 320);
    //������
    LCD_Fill(0, 260, 60, 320, BLACK);
    LCD_Fill(120, 260, 180, 320, BLACK);
    POINT_COLOR = RED;
    LCD_ShowString(5, 280, 50, 20, 16, char_turn_left);
    LCD_ShowString(125, 280, 50, 20, 16, char_turn_right);
}
//����̬����
extern uint32_t race_score;
void Draw_score(void)
{
    LCD_ShowNum(185, 160, race_score, 4, 24);
}
//��һ����̬С��
void Draw_crash_car(Moving_car* car)
{
    if (car->condition == out_screen || car->color == BLACK)
    {
        int a = rand() % 5;//���������ɫ��ʾ��ͬ��ɫ��С��������һ�»�ɫ
        switch (a)
        {
        case 0://������ɫС��
            car->color = RED;
            break;
        case 1://������ɫС��
            car->color = BLUE;
            break;
        case 2://������ɫС��
            car->color = GREEN;
            break;
        case 3://������ɫС��
            car->color = BRRED;
            break;
        case 4://������ɫС��
            car->color = YELLOW;
            break;
        default:
            break;
        }
    }
    LCD_Fill(car->left_up_x, car->left_up_y, car->left_up_x + car->width, car->left_up_y + car->height, car->color);
    LCD_Fill(car->left_up_x + 0.1*car->width, car->left_up_y + 0.7*car->height, car->left_up_x + 0.4*car->width, car->left_up_y + 0.9*car->height, GRAY);
    LCD_Fill(car->left_up_x + 0.6*car->width, car->left_up_y + 0.7*car->height, car->left_up_x + 0.9*car->width, car->left_up_y + 0.9*car->height, GRAY);
}
//���һ����̬С��
void Clean_crash_car(Moving_car* car)
{
    LCD_Fill(car->left_up_x, car->left_up_y, car->left_up_x + car->width, car->left_up_y + car->height, WHITE);
    LCD_Fill(car->left_up_x + 0.1*car->width, car->left_up_y + 0.7*car->height, car->left_up_x + 0.4*car->width, car->left_up_y + 0.9*car->height, WHITE);
    LCD_Fill(car->left_up_x + 0.6*car->width, car->left_up_y + 0.7*car->height, car->left_up_x + 0.9*car->width, car->left_up_y + 0.9*car->height, WHITE);
}
void Clean_all_car(void)
{
    Clean_crash_car(&car1);
    Clean_crash_car(&car2);
    Clean_crash_car(&car3);
    Clean_crash_car(&car4);
    Clean_crash_car(&car5);
    Clean_crash_car(&car6);
}
//�����С��
void Draw_player_car(Moving_car* car)
{
    LCD_Fill(car->left_up_x, car->left_up_y, car->left_up_x + car->width, car->left_up_y + car->height, BLUE);
    LCD_Fill(car->left_up_x + 0.1*car->width, car->left_up_y + 0.1*car->height, car->left_up_x + 0.4*car->width, car->left_up_y + 0.3*car->height, GRAY);
    LCD_Fill(car->left_up_x + 0.6*car->width, car->left_up_y + 0.1*car->height, car->left_up_x + 0.9*car->width, car->left_up_y + 0.3*car->height, GRAY);
}
//���߸���̬С��
void Draw_all_car(void)
{
    if (car1.condition == in_screen)
    {
        Draw_crash_car(&car1);
    }
    if (car2.condition == in_screen)
    {
        Draw_crash_car(&car2);
    }
    if (car3.condition == in_screen)
    {
        Draw_crash_car(&car3);
    }
    if (car4.condition == in_screen)
    {
        Draw_crash_car(&car4);
    }
    if (car5.condition == in_screen)
    {
        Draw_crash_car(&car5);
    }
    if (car6.condition == in_screen)
    {
        Draw_crash_car(&car6);
    }
    Draw_player_car(&player_car);
}
extern int car_location;
//���������ײ
void Detect_all_crash(Moving_car* game_car1, Moving_car* game_car2, Moving_car* game_car3, 
                      Moving_car* game_car4, Moving_car* game_car5, Moving_car* game_car6, Moving_car* play_car)
{
    switch (car_location)
    {
          case line1:
              Detect_one_crash(game_car1, play_car);
              Detect_one_crash(game_car2, play_car);
              break;
          case line2:
              Detect_one_crash(game_car3, play_car);
              Detect_one_crash(game_car4, play_car);
              break;
          case line3:
              Detect_one_crash(game_car5, play_car);
              Detect_one_crash(game_car6, play_car);
              break;
          default:
              break;
    }
}
//��ⵥ����ײ
extern int car_condition;

void Detect_one_crash(Moving_car* game_car, Moving_car* player_car)
{
    if (player_car->left_up_x < game_car->left_up_x && (player_car->left_up_x + player_car->width) > game_car->left_up_x &&
        player_car->left_up_y > game_car->left_up_y && player_car->left_up_y < (game_car->left_up_y + game_car->height))
    {
        player_car->condition = car_dead;
    }
    else if (player_car->left_up_x < game_car->left_up_x && (player_car->left_up_x + player_car->width) > game_car->left_up_x &&
        player_car->left_up_y < game_car->left_up_y && (player_car->left_up_y + player_car->height) > game_car->left_up_y)
    {
        player_car->condition = car_dead;
    }
    else if (player_car->left_up_x < (game_car->left_up_x + game_car->width) && (player_car->left_up_x + player_car->width) > (game_car->left_up_x + game_car->width) &&
        player_car->left_up_y > game_car->left_up_y && player_car->left_up_y < (game_car->left_up_y + game_car->height))
    {
        player_car->condition = car_dead;
    }
    else if (player_car->left_up_x < (game_car->left_up_x + game_car->width) && (player_car->left_up_x + player_car->width) > (game_car->left_up_x + game_car->width) &&
        player_car->left_up_y < game_car->left_up_y && (player_car->left_up_y + player_car->height) > game_car->left_up_y)
    {
        player_car->condition = car_dead;
    }
    else if (player_car->left_up_y < (game_car->left_up_y + game_car->height))
    {
        player_car->condition = car_dead;
    }
}
//�����Ļ�Ƿ���
extern uint16_t key1_x;
extern uint16_t key1_y;
extern uint16_t key2_x;
extern uint16_t key2_y;
extern uint16_t key_width_height;
//extern struct Moving_car car;
//extern uint16_t car_location;
void Touch_screen(Moving_car* car)
{
    static uint8_t key_up = 1;
    key_width_height = 60;
    TOUCH_Scan(0);
    if (key_up && tp_dev.x[0] > key1_x&&tp_dev.x[0] < (key1_x + key_width_height) && tp_dev.y[0]>key1_y&&tp_dev.y[0] < (key1_y + key_width_height))
    {
        delay_ms(10);
        key_up = 0;
        switch (car_location)
        {
             case line1:
                 break;
             case line2:
                 Clean_crash_car(&player_car);
                 player_car.left_up_x = 10;
                 car_location = line1;
                 break;
             case line3:
                 Clean_crash_car(&player_car);
                 player_car.left_up_x = 70;
                 car_location = line2;
             default:
                 break;
        }
    }
    else if (key_up && tp_dev.x[0] > key2_x&&tp_dev.x[0] < (key2_x + key_width_height) && tp_dev.y[0]>key2_y&&tp_dev.y[0] < (key2_y + key_width_height))
    {
        delay_ms(10);
        key_up = 0;
        switch (car_location)
        {
        case line1:
            Clean_crash_car(&player_car);
            player_car.left_up_x = 70;
            car_location = line2;
            break;
        case line2:
            Clean_crash_car(&player_car);
            player_car.left_up_x = 130;
            car_location = line2;
            break;
        case line3:
            break;
        default:
            break; 
        }
    }
    if (tp_dev.x[0] == 0xffff)
    {
        key_up = 1;
    }
    tp_dev.x[0] = 0xffff;
    tp_dev.y[0] = 0xffff;

}
//��ʱ���ж�ʹС�������ƶ�
extern int car_speed;
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    car_speed = 3;
    while_round++;
    if (htim == (&htim4))
    {
        Clean_all_car();
        if (car1.condition == in_screen)
        {
            car1.left_up_y += car_speed;
        }
        if (car2.condition == in_screen)
        {
            car2.left_up_y += car_speed;
        }
        if (car3.condition == in_screen)
        {
            car3.left_up_y += car_speed;
        }
        if (car4.condition == in_screen)
        {
            car4.left_up_y += car_speed;
        }
        if (car5.condition == in_screen)
        {
            car5.left_up_y += car_speed;
        }
        if (car6.condition == in_screen)
        {
            car6.left_up_y += car_speed;
        }
        if (car1.condition == in_screen && car2.condition == out_screen || (car1.condition == in_screen && car2.condition == in_screen && car1.left_up_y <= 120))
        {
            if ((car1.left_up_y - car2.left_up_y) <= 160 && car1.left_up_y <= 120)
            {
                car2.left_up_y = -40;
            }
        }
        else if (car2.condition == in_screen && car1.condition == out_screen)
        {
            if ((car2.left_up_y - car1.left_up_y) <= 160 && car2.left_up_y <= 120)
            {
                car1.left_up_y = -40;
            }
        }
        if (car3.condition == in_screen && car4.condition == out_screen || (car3.condition == in_screen && car4.condition == in_screen && car3.left_up_y <= 120))
        {
            if ((car3.left_up_y - car4.left_up_y) <= 160 && car3.left_up_y <= 120)
            {
                car4.left_up_y = -40;
            }
        }
        else if (car4.condition == in_screen && car3.condition == out_screen)
        {
            if ((car4.left_up_y - car3.left_up_y) <= 160 && car4.left_up_y <= 120)
            {
                car3.left_up_y = -40;
            }
        }
        if (car5.condition == in_screen && car6.condition == out_screen || (car5.condition == in_screen && car6.condition == in_screen && car5.left_up_y <= 120))
        {
            if ((car5.left_up_y - car6.left_up_y) <= 160 && car5.left_up_y <= 120)
            {
                car6.left_up_y = -40;
            }
        }
        else if (car6.condition == in_screen && car5.condition == out_screen)
        {
            if ((car6.left_up_y - car5.left_up_y) <= 160 && car6.left_up_y <= 120)
            {
                car5.left_up_y = -40;
            }
        }
        Draw_all_car();
    }
}
//������������
uint16_t width = 30;
uint16_t height = 40;
extern int car_location;
void race_car_main()
{
    srand((unsigned)time(NULL));
    //��ʼ��С��x���꼰����
    car1.left_up_x = 10; car1.left_up_y = -40;
    car2.left_up_x = 10; car2.left_up_y = -40;
    car3.left_up_x = 70; car3.left_up_y = -40;
    car4.left_up_x = 70; car4.left_up_y = -40;
    car5.left_up_x = 130; car5.left_up_y = -40;
    car6.left_up_x = 130; car6.left_up_y = -40;
    player_car.left_up_y = 178;
    car1.width = width, car1.height = height;
    car2.width = width, car2.height = height;
    car3.width = width, car3.height = height;
    car4.width = width, car4.height = height;
    car5.width = width, car5.height = height;
    car6.width = width, car6.height = height;
    car1.condition = out_screen;
    car2.condition = out_screen;
    car3.condition = out_screen;
    car4.condition = out_screen;
    car5.condition = out_screen;
    car6.condition = out_screen;
    player_car.width = width, player_car.height = height;
    player_car.condition = 1;
    //����̬���
    Draw_Still_Frame();
    int car_location = rand() % 3;//����������С�����
    switch (car_location)
    {
    case line1:
        player_car.left_up_x = 10;
        break;
    case line2:
        player_car.left_up_x = 70;
        break;
    case line3:
        player_car.left_up_x = 130;
        break;
    }
    HAL_TIM_Base_Start_IT(&htim4);
//    a = rand() % 2;
//    b = rand() % 2;
//    c = rand() % 2;
//    while (a == creat && b == creat && c == creat)
//{
//        a = rand() % 2;
//        b = rand() % 2;
//        c = rand() % 2;
//    }
    while (1)
    {
        //���������ʱ��Ϸ��ת����������
        if (player_car.condition == car_dead)
        {
            break;
        }
        else
        {
            uint16_t x = while_round % 200;
            if (x == 199)
            {
                while_round = 0;
                race_score++;
                Draw_score();//����̬����
            }

            //Draw_all_car();//������С��
            //�����Ļ�Ƿ񱻰���
            Touch_screen(&player_car);
            //��ⰴ�º��Ƿ�����ײ
            Detect_all_crash(&car1, &car2, &car3, &car4, &car5, &car6, &player_car);
            if (car1.left_up_y >= 219) 
                car1.left_up_y = 1000;
            if (car2.left_up_y >= 219) 
                car2.left_up_y = 1000;
            if (car3.left_up_y >= 219) 
                car3.left_up_y = 1000;
            if (car4.left_up_y >= 219) 
                car4.left_up_y = 1000;
            if (car5.left_up_y >= 219) 
                car5.left_up_y = 1000;
            if (car6.left_up_y >= 219) 
                car6.left_up_y = 1000;

            //a = rand() % 2;
            //b = rand() % 2;
            //c = rand() % 2;
            //while (a == creat && b == creat && c == creat)
            //{
            //    a = rand() % 2;
            //    b = rand() % 2;
            //    c = rand() % 2;
            //}
            int a, b, c;
            uint16_t y = while_round % 200;
            if (y == 199)
            {
                a = rand() % 2;
                b = rand() % 2;
                c = rand() % 2;
                while (a == creat && b == creat && c == creat)
                {
                    a = rand() % 2;
                    b = rand() % 2;
                    c = rand() % 2;
                }
            }
            //�����Ļ�Ƿ񱻰���
            //Touch_screen(&player_car);
            switch (a)
            {
                case creat:
                    if (car1.left_up_y >= 220)
                    {
                        LCD_Fill(10, 219, 40, 259, WHITE);
                        car1.condition = out_screen;
                        car1.left_up_y = -40;
                    }
                    else
                    {
                        car1.condition = in_screen;
                    }
                    if (car2.left_up_y >= 220)
                    {
                        LCD_Fill(10, 219, 40, 259, WHITE);
                        car2.condition = out_screen;
                        car2.left_up_y = -40;
                    }
                    else
                    {
                        car2.condition = in_screen;
                    }
                    if (car1.condition == in_screen && car2.condition == out_screen || (car1.condition == in_screen && car2.condition == in_screen && car1.left_up_y <= 120))
                    {
                        if((car1.left_up_y - car2.left_up_y) <= 160 && car1.left_up_y <= 120)
                        {
                            car2.left_up_y = -40;
                        }
                    }
                    else if (car2.condition == in_screen && car1.condition == out_screen)
                    {
                        if((car2.left_up_y - car1.left_up_y) <= 160 && car2.left_up_y <= 120)
                        {
                            car1.left_up_y = -40;
                        }
                    }
            }
            //�����Ļ�Ƿ񱻰���
            //Touch_screen(&player_car);
            switch (b)
            {
                case creat:
                    if (car3.left_up_y >= 220)
                    {
                        LCD_Fill(70, 219, 100, 259, WHITE);
                        car3.condition = out_screen;
                        car3.left_up_y = -40;
                    }
                    else
                    {
                        car3.condition = in_screen;
                    }
                    if (car4.left_up_y >= 220)
                    {
                        LCD_Fill(70, 219, 100, 259, WHITE);
                        car4.condition = out_screen;
                        car4.left_up_y = -40;
                    }
                    else
                    {
                        car4.condition = in_screen;
                    }
                    if (car3.condition == in_screen && car4.condition == out_screen || (car3.condition == in_screen && car4.condition == in_screen && car3.left_up_y <= 120))
                    {
                        if((car3.left_up_y - car4.left_up_y) <= 160 && car3.left_up_y <= 120)
                        {
                            car4.left_up_y = -40;
                        }
                    }
                    else if (car4.condition == in_screen && car3.condition == out_screen)
                    {
                        if((car4.left_up_y - car3.left_up_y) <= 160 && car4.left_up_y <= 120)
                        {
                            car3.left_up_y = -40;
                        }
                    }
            }
            //�����Ļ�Ƿ񱻰���
            //Touch_screen(&player_car);
            switch (c)
            {
                case creat:
                    if (car5.left_up_y >= 220)
                    {
                        LCD_Fill(130, 219, 160, 259, WHITE);
                        car5.condition = out_screen;
                        car5.left_up_y = -40;
                    }
                    else
                    {
                        car5.condition = in_screen;
                    }
                    if (car6.left_up_y >= 220)
                    {
                        LCD_Fill(130, 219, 160, 259, WHITE);
                        car6.condition = out_screen;
                        car6.left_up_y = -40;
                    }
                    else
                    {
                        car6.condition = in_screen;
                    }
                    if (car5.condition == in_screen && car6.condition == out_screen || (car5.condition == in_screen && car6.condition == in_screen && car5.left_up_y <= 120))
                    {
                        if((car5.left_up_y - car6.left_up_y) <= 160 && car5.left_up_y <= 120)
                        {
                            car6.left_up_y = -40;
                        }
                    }
                    else if (car6.condition == in_screen && car5.condition == out_screen)
                    {
                        if((car6.left_up_y - car5.left_up_y) <= 160 && car6.left_up_y <= 120)
                        {
                            car5.left_up_y = -40;
                        }
                    }
            }
        }
    }
    HAL_TIM_Base_Stop_IT(&htim4);
    LCD_Clear(BLUE);
    char final_score[20] = "Your final score is:";
    LCD_ShowString(60, 110, 120, 30, 24, final_score);
    LCD_ShowNum(100, 180, race_score, 4, 24);
    while (1);
}