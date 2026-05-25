#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

typedef struct
{
    int x;
    int y;
} motion_cmd_t;

// Queue Handle
QueueHandle_t motionQueue;

// --------------------------------------------------
// CORE 0 TASK
// Simulates Jetson sending coordinates
// --------------------------------------------------

void communication_task(void *pvParameters)
{
    motion_cmd_t cmd;

    int coordinate = 100;

    while (1)
    {
        // Create dummy coordinates
        cmd.x = coordinate;
        cmd.y = coordinate + 50;

        printf("\n[CORE0] Received Coordinate\n");
        printf("[CORE0] X=%d Y=%d\n", cmd.x, cmd.y);

        // Send to motion queue
        xQueueSend(motionQueue, &cmd, portMAX_DELAY);

        printf("[CORE0] Sent to Motion Queue\n");

        // Wait before sending next coordinate
        // Simulates waiting for next Jetson command
        vTaskDelay(pdMS_TO_TICKS(5000));

        coordinate += 100;
    }
}

// --------------------------------------------------
// CORE 1 TASK
// Simulates motor movement
// --------------------------------------------------

void motion_task(void *pvParameters)
{
    motion_cmd_t received_cmd;

    while (1)
    {
        // Wait indefinitely for coordinate
        if (xQueueReceive(motionQueue, &received_cmd, portMAX_DELAY))
        {
            printf("\n========== CORE1 ==========\n");

            printf("[CORE1] Coordinate Received\n");

            printf("[CORE1] Moving to:\n");
            printf("[CORE1] X=%d Y=%d\n",
                   received_cmd.x,
                   received_cmd.y);

            // Simulate motor movement
            for (int i = 1; i <= 5; i++)
            {
                printf("[CORE1] Moving... %d\n", i);

                vTaskDelay(pdMS_TO_TICKS(1000));
            }

            printf("[CORE1] Reached Destination\n");

            printf("===========================\n");
        }
    }
}

// --------------------------------------------------
// MAIN
// --------------------------------------------------

void app_main(void)
{
    // Create Queue
    motionQueue = xQueueCreate(5, sizeof(motion_cmd_t));

    // Core 0 Task
    xTaskCreatePinnedToCore(
        communication_task,
        "communication_task",
        4096,
        NULL,
        2,
        NULL,
        0);

    // Core 1 Task
    xTaskCreatePinnedToCore(
        motion_task,
        "motion_task",
        4096,
        NULL,
        2,
        NULL,
        1);
}