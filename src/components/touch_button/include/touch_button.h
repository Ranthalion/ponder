#pragma once

#include "driver/touch_pad.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "soc/rtc_periph.h"
#include "soc/sens_periph.h"

#define MAX_TOUCH_BUTTONS 14

typedef struct touch_msg {
    touch_pad_intr_mask_t intr_mask;
    uint32_t pad_num;
    uint32_t pad_status;
    uint32_t pad_val;
} touch_event_t;

typedef struct touch_config {
    touch_pad_t buttons[MAX_TOUCH_BUTTONS];
    uint8_t num_buttons;
} touch_config;

void initTouch(touch_config init);
