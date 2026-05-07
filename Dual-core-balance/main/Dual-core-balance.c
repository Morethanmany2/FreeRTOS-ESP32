#include <stdio.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "esp_log.h"

static const char *TAG = "DUAL_CORE";

QueueHandle_t data_queue;

// ---------------- PRODUCER TASK ----------------

void producer_task(void *pvParameters)
{
    int value = 0;

    while (1)
    {
        value++;

        xQueueSend(data_queue,
                   &value,
                   portMAX_DELAY);

        ESP_LOGI(TAG,
                 "Producer -> Sent: %d | Core: %d",
                 value,
                 xPortGetCoreID());

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

// ---------------- CONSUMER TASK ----------------

void consumer_task(void *pvParameters)
{
    int received;

    while (1)
    {
        if (xQueueReceive(data_queue,
                          &received,
                          portMAX_DELAY))
        {
            ESP_LOGI(TAG,
                     "Consumer -> Received: %d | Core: %d",
                     received,
                     xPortGetCoreID());
        }
    }
}

// ---------------- HEAVY COMPUTE TASK ----------------

void heavy_compute_task(void *pvParameters)
{
    volatile long sum;

    while (1)
    {
        sum = 0;

        for (long i = 0; i < 10000000; i++)
        {
            sum += i;
        }

        ESP_LOGI(TAG,
                 "Heavy Compute Finished | Core: %d",
                 xPortGetCoreID());

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

// ---------------- MONITOR TASK ----------------

void monitor_task(void *pvParameters)
{
    while (1)
    {
        ESP_LOGI(TAG,
                 "Free Heap: %lu bytes | Core: %d",
                 esp_get_free_heap_size(),
                 xPortGetCoreID());

        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}

// ---------------- MAIN ----------------

void app_main(void)
{
    data_queue = xQueueCreate(10, sizeof(int));

    if (data_queue == NULL)
    {
        ESP_LOGE(TAG, "Queue creation failed");
        return;
    }

    // Producer pinned to Core 0
    xTaskCreatePinnedToCore(
        producer_task,
        "Producer Task",
        4096,
        NULL,
        1,
        NULL,
        0);

    // Consumer pinned to Core 1
    xTaskCreatePinnedToCore(
        consumer_task,
        "Consumer Task",
        4096,
        NULL,
        1,
        NULL,
        1);

    // Heavy compute pinned to Core 0
    xTaskCreatePinnedToCore(
        heavy_compute_task,
        "Heavy Compute Task",
        4096,
        NULL,
        2,
        NULL,
        0);

    // Monitor pinned to Core 1
    xTaskCreatePinnedToCore(
        monitor_task,
        "Monitor Task",
        4096,
        NULL,
        1,
        NULL,
        1);

    ESP_LOGI(TAG, "Dual-Core System Started");
}