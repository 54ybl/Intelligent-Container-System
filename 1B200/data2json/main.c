
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include "ls1b.h"
#include "mips.h"
#include "ls1b_gpio.h"

#include "src\press\press.h"
#include "src\i2c\i2c.h"
#include "src\link\link.h"
#include "src\uart\uart.h"
#include "ls1x_rtc.h"
#include "src\lux\lux.h"
#include "src\other\other.h"

#include "bsp.h"

// RTCʱ��
struct tm tmp, now = {
                   .tm_sec = 30,
                   .tm_min = 35,
                   .tm_hour = 10,
                   .tm_mday = 14,
                   .tm_mon = 5,
                   .tm_year = 2023,
};

//-------------------------------------------------------------------------------------------------
// ������
//-------------------------------------------------------------------------------------------------

//��ʼ��GPIO����ӦLoRaM0��M1
static void ls1b_set_gpio_regs(void)
{
    gpio_enable(40, DIR_OUT);    // Pin: CAN0_SDA2
    gpio_enable(41, DIR_OUT);    // Pin: CAN0_SCL2
}

// ����MAC: 02:02:17:00:01:01

int main(void)
{
    int cnt = 0, i = 0,tmn = 10;
    float temp = 0, hum = 0, press = 0, eleva = 0, lux = 0;
    char fire = 0;
    char loc[50];
    strcpy(loc, "112.342473&16.842207");

    I2C1_init();            //��ʼ��I2C
    Get_HDC_ID();
    SPL06_init();
    ls1b_set_gpio_regs();

    gpio_write(40,0);       //GPIO���0��0 ʹLoRa������ģʽ0
    gpio_write(41,0);       

    // ��ʼ������
    UART4_Config_Init();
    UART5_Config_Init();
    //-----------------------
    // ��ȡ����ʱ��
    //-----------------------
    // ��ʼ��RTC������
    ls1x_rtc_init(NULL, NULL);
    // ����RTCʱ��
    ls1x_rtc_set_datetime(&now);

    // ��ʼ�����մ�����
    TSL_init();

    /*
     * �����ѭ��
     */

    for (;;)
    {
        // ��ȡ����ָ��
        UART5_Read();
        // ���ݲɼ����ͼ�ʱ��
        if (cnt >= 10)
        {
            // RTC��ȡʱ��
            ls1x_rtc_get_datetime(&tmp);
            sprintf(t1, "[\"%d\",\"%d\",\"%d\"]", tmp.tm_year + 1900, tmp.tm_mon + 1, tmp.tm_mday);
            sprintf(t2, "[\"%d\",\"%d\",\"%d\"]", tmp.tm_hour, tmp.tm_min, tmp.tm_sec);

            // ��ȡ��ʪ��
            HDC_Get_Temp_Hum(&temp, &hum);
            // ��ȡ��ѹ
            SPL06_Get_Prs(&press, &eleva);

            // ��ȡ����
            lux = TSL2561FN_RD_Data();

            // ��
            fire = MQ_Read_Status();
            if (fire == 0)
            {
                fire = 1;
            }
            else if (fire == 1)
            {
                fire = 0;
            }
            //ƴ�Ӵ���������
            sprintf(t3, "{\"F1\":\"%.2f\",\"F2\":\"%.0f\",\"F3\":\"%d\",\"F4\":\"%.0f\",\"F5\":\"%.0f\"}", temp, hum, fire, press/1000, lux);

            // ƴ�����ݰ�
            link1(t1, t2, t3);
            //����������ݵ�LoRa
            UART4_Test(result);
            // UART5_Test(result);
            printf("tm=%d", tmn);
            i++;
            cnt = 0;
        }
        cnt++;
        // tmn = UART5_Read();
        // tmn = UART4_Read();

        // result�ڴ��ͷţ��������
        memset(result, 0, sizeof(result));
        delay_ms(200);
    }

    /*
     * Never goto here!
     */
    return 0;
}

/*
 * @@ End
 */
