#include "driver/i2c_slave.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define SLAVE_SDA 27
#define SLAVE_SCL 26
#define SLAVE_ADDR 0x55

static const char *TAG = "SLAVE";

void i2c_slave_task(void *arg)
{
    i2c_slave_config_t slave_config = {
    .i2c_port = I2C_NUM_1,
    .sda_io_num = SLAVE_SDA,
    .scl_io_num = SLAVE_SCL,
    .clk_source = I2C_CLK_SRC_DEFAULT,

    .send_buf_depth = 256,
    .receive_buf_depth = 256,

    .slave_addr = SLAVE_ADDR,
    .addr_bit_len = I2C_ADDR_BIT_LEN_7,

    .intr_priority = 0,

    .flags = {
        .allow_pd = 0,
        .enable_internal_pullup = 1,
    }
};

    i2c_slave_dev_handle_t slave_handle = NULL;

    esp_err_t ret = i2c_new_slave_device(
    &slave_config,
    &slave_handle
);

ESP_LOGI(TAG,
         "Slave Init = %s",
         esp_err_to_name(ret));

if (ret != ESP_OK)
{
    vTaskDelete(NULL);
}

while (1)
{
    vTaskDelay(pdMS_TO_TICKS(1000));
}
}