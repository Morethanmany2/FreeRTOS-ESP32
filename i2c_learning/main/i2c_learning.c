#include <stdio.h>
#include "driver/i2c_master.h"
#include "driver/i2c_slave.h"
#include "esp_log.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define I2C_SDA_PIN 12
#define I2C_SCL_PIN 14

void i2c_slave_task(void *arg);
static const char *TAG = "I2C";

void app_main(void)
{
    xTaskCreate(
    i2c_slave_task,
    "i2c_slave",
    4096,
    NULL,
    5,
    NULL
);
    i2c_master_bus_config_t bus_config = {
        .i2c_port = I2C_NUM_0,
        .sda_io_num = I2C_SDA_PIN,
        .scl_io_num = I2C_SCL_PIN,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };

    i2c_master_bus_handle_t bus_handle = NULL;
    i2c_master_dev_handle_t dev_handle = NULL;

    esp_err_t ret = i2c_new_master_bus(
        &bus_config,
        &bus_handle
    );

    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "I2C Init Failed: %s",
                 esp_err_to_name(ret));
        return;
    }

    ESP_LOGI(TAG, "I2C Master Initialized");

    ESP_LOGI(TAG, "Starting I2C Scan...");

for (uint8_t addr = 1; addr < 0x80; addr++)
{
    i2c_master_dev_handle_t dev_handle = NULL;

    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = addr,
        .scl_speed_hz = 100000,
    };

    esp_err_t ret = i2c_master_bus_add_device(
        bus_handle,
        &dev_cfg,
        &dev_handle
    );

    if (ret != ESP_OK)
    {
        continue;
    }

    uint8_t dummy = 0x00;

    ret = i2c_master_transmit(
        dev_handle,
        &dummy,
        1,
        50
    );

    if (ret == ESP_OK)
    {
        ESP_LOGI(TAG,
                 "Found device at 0x%02X",
                 addr);
    }

    i2c_master_bus_rm_device(dev_handle);
}
}