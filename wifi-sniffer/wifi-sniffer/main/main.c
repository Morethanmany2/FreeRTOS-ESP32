#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "esp_wifi.h"
#include "esp_wifi_types.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"

static const char *TAG = "SNIFFER";

typedef struct
{
    int rssi;
    uint8_t mac[6];
    wifi_promiscuous_pkt_type_t type;

} packet_info_t;

QueueHandle_t packet_queue;

void wifi_sniffer_packet_handler(void *buff,
                                 wifi_promiscuous_pkt_type_t type)
{
    wifi_promiscuous_pkt_t *pkt =
        (wifi_promiscuous_pkt_t *)buff;

    packet_info_t packet;

    packet.rssi = pkt->rx_ctrl.rssi;
    packet.type = type;

    memcpy(packet.mac,
           pkt->payload + 10,
           6);

    xQueueSend(packet_queue,
               &packet,
               portMAX_DELAY);
}

void packet_task(void *pvParameters)
{
    packet_info_t packet;

    while (1)
    {
        if (xQueueReceive(packet_queue,
                          &packet,
                          portMAX_DELAY))
        {
            ESP_LOGI(TAG,
                     "RSSI: %d | MAC: %02X:%02X:%02X:%02X:%02X:%02X | TYPE: %d",
                     packet.rssi,
                     packet.mac[0],
                     packet.mac[1],
                     packet.mac[2],
                     packet.mac[3],
                     packet.mac[4],
                     packet.mac[5],
                     packet.type);
        }
    }
}

void wifi_sniffer_init()
{
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(
        esp_wifi_set_storage(WIFI_STORAGE_RAM));

    ESP_ERROR_CHECK(
        esp_wifi_set_mode(WIFI_MODE_NULL));

    ESP_ERROR_CHECK(
        esp_wifi_start());

    ESP_ERROR_CHECK(
        esp_wifi_set_promiscuous(true));

    ESP_ERROR_CHECK(
        esp_wifi_set_promiscuous_rx_cb(
            wifi_sniffer_packet_handler));

    ESP_LOGI(TAG, "Promiscuous mode enabled");
}

void app_main(void)
{
    ESP_LOGI("TEST", "NEW CODE RUNNING");

    esp_err_t ret = nvs_flash_init();

    if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
        ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }

    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(
        esp_event_loop_create_default());

    packet_queue = xQueueCreate(
        100,
        sizeof(packet_info_t));

    if (packet_queue == NULL)
    {
        ESP_LOGE(TAG, "Queue creation failed");
        return;
    }

    xTaskCreate(
        packet_task,
        "packet_task",
        4096,
        NULL,
        5,
        NULL);

    wifi_sniffer_init();

    ESP_LOGI(TAG, "WiFi Sniffer Started");

    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}