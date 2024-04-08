#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "driver/rmt_tx.h"
#include "robitronic_ir_encoder.h"

// References / Docs / Source /etc:
// 5.2.1 RMT example code on which this is based - https://github.com/espressif/esp-idf/tree/v5.2.1/examples/peripherals/rmt/led_strip
// 5.2.1 api docs (with encoder API) - https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/rmt.html#rmt-kconfig-options
// 5.2 release driver source -  https://github.com/espressif/esp-idf/blob/v5.2.1/components/driver/rmt/rmt_tx.c
// latest RMT driver source - https://github.com/espressif/esp-idf/blob/master/components/esp_driver_rmt/src/rmt_tx.c
// old RMT driver source - https://github.com/espressif/esp-idf/blob/release/v4.4/components/driver/rmt.c
//
// old 4.4.7 api docs (no encoder API) - https://docs.espressif.com/projects/esp-idf/en/v4.4.7/esp32/api-reference/peripherals/rmt.html
static const char *TAG = "IR";

#define RMT_IR_RESOLUTION_HZ 10000000 // 10MHz resolution, 1 tick = 0.1us
#define RMT_IR_GPIO_NUM      IR_GPIO_PIN

#define EXAMPLE_REPEAT_MS           10
#define EXAMPLE_CODE_DIGITS         2
static uint8_t ir_code[EXAMPLE_CODE_DIGITS] = {0x33,0xCC};

void app_main() {
    ESP_LOGI(TAG, "Create RMT TX channel");
    rmt_channel_handle_t led_chan = NULL;
    rmt_tx_channel_config_t tx_chan_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT, // select source clock
        .gpio_num = RMT_IR_GPIO_NUM,
        .mem_block_symbols = 64, // increase the block size can make the LED less flickering
        .resolution_hz = RMT_IR_RESOLUTION_HZ,
        .trans_queue_depth = 4, // set the number of transactions that can be pending in the background
    };
    ESP_ERROR_CHECK(rmt_new_tx_channel(&tx_chan_config, &led_chan));


    ESP_LOGI(TAG, "Install led strip encoder");
    rmt_encoder_handle_t led_encoder = NULL;
    led_strip_encoder_config_t encoder_config = {
        .resolution = RMT_IR_RESOLUTION_HZ,
    };
    ESP_ERROR_CHECK(rmt_new_led_strip_encoder(&encoder_config, &led_encoder));


    ESP_LOGI(TAG, "Enable RMT TX channel");
    ESP_ERROR_CHECK(rmt_enable(led_chan));


    ESP_LOGI(TAG, "Start IR transmit");
    rmt_transmit_config_t tx_config = {
        .loop_count = 0, // no transfer loop
    };

    while (1) {
        ESP_ERROR_CHECK(rmt_transmit(led_chan, led_encoder, ir_code, sizeof(ir_code), &tx_config));
        ESP_ERROR_CHECK(rmt_tx_wait_all_done(led_chan, portMAX_DELAY));
        vTaskDelay(pdMS_TO_TICKS(EXAMPLE_REPEAT_MS));
    }
}
