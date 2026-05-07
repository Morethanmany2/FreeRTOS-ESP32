#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "driver/gpio.h"

#define LED_GPIO GPIO_NUM_2
#define BUTTON_GPIO GPIO_NUM_0

SemaphoreHandle_t button_semaphore;

// Function prototype
void led_task(void *pvParameters);

// ISR
void IRAM_ATTR button_isr_handler(void *arg)
{
    static uint32_t last_interrupt_time = 0;

    uint32_t current_time = xTaskGetTickCountFromISR();

    if ((current_time - last_interrupt_time) > pdMS_TO_TICKS(200))
    {
        xSemaphoreGiveFromISR(button_semaphore, NULL);

        last_interrupt_time = current_time;
    }
}

// Main application
void app_main(void)
{
    gpio_reset_pin(LED_GPIO);

    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);

    gpio_reset_pin(BUTTON_GPIO);

    gpio_set_direction(BUTTON_GPIO, GPIO_MODE_INPUT);

    gpio_pullup_en(BUTTON_GPIO);

    gpio_set_intr_type(BUTTON_GPIO, GPIO_INTR_NEGEDGE);

    button_semaphore = xSemaphoreCreateBinary();

    xTaskCreate(
        led_task,
        "LED Task",
        2048,
        NULL,
        1,
        NULL
    );

    gpio_install_isr_service(0);

    gpio_isr_handler_add(
        BUTTON_GPIO,
        button_isr_handler,
        NULL
    );

    printf("System Ready!\n");
}

// LED task
void led_task(void *pvParameters)
{
    int led_state = 0;

    while (1)
    {
        if (xSemaphoreTake(button_semaphore, portMAX_DELAY))
        {
            led_state = !led_state;

            gpio_set_level(LED_GPIO, led_state);

            printf("BOOT Button Pressed -> LED %s\n",
                   led_state ? "ON" : "OFF");
        }
    }
}