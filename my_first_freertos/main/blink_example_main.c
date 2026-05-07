#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"

#define LED1 GPIO_NUM_2
#define LED2 GPIO_NUM_4

void fast_blink_task(void *pvParameters)
{
    while (1)
    {
        gpio_set_level(LED1, 1);

        printf("FAST LED ON\n");

        vTaskDelay(pdMS_TO_TICKS(200));

        gpio_set_level(LED1, 0);

        printf("FAST LED OFF\n");

        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

void slow_blink_task(void *pvParameters)
{
    while (1)
    {
        gpio_set_level(LED2, 1);

        printf("SLOW LED ON\n");

        vTaskDelay(pdMS_TO_TICKS(1000));

        gpio_set_level(LED2, 0);

        printf("SLOW LED OFF\n");

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void app_main(void)
{
    gpio_reset_pin(LED1);
    gpio_set_direction(LED1, GPIO_MODE_OUTPUT);

    gpio_reset_pin(LED2);
    gpio_set_direction(LED2, GPIO_MODE_OUTPUT);

    xTaskCreate(
        fast_blink_task,
        "Fast Blink Task",
        2048,
        NULL,
        1,
        NULL
    );

    xTaskCreate(
        slow_blink_task,
        "Slow Blink Task",
        2048,
        NULL,
        1,
        NULL
    );
}