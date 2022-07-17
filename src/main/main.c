#include "touch_button.h"
#include "esp_log.h"
#include "esp_sleep.h"

#define TOUCH_BUTTON_NUM 14

static const char *TAG = "Ponder";

void app_main(void)
{
    ESP_LOGI(TAG, "Start app_main");
    touch_config touch_init = {
        .buttons = {
            TOUCH_PAD_NUM1,
            TOUCH_PAD_NUM2,
            TOUCH_PAD_NUM3,
            TOUCH_PAD_NUM4,
            TOUCH_PAD_NUM5,
            TOUCH_PAD_NUM6,
            TOUCH_PAD_NUM7,
            TOUCH_PAD_NUM8,
            TOUCH_PAD_NUM9,
            TOUCH_PAD_NUM10,
            TOUCH_PAD_NUM11,
            TOUCH_PAD_NUM12,
            TOUCH_PAD_NUM13,
            TOUCH_PAD_NUM14,
        },
        .num_buttons = TOUCH_BUTTON_NUM,
    };

    initTouch(touch_init);
    esp_sleep_enable_touchpad_wakeup();
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);

    ESP_LOGI(TAG, "End app_main");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    esp_light_sleep_start();
}
