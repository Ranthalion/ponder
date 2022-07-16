#include "touch_button.h"

static const char *TOUCH_TAG = "Touch pad";
static QueueHandle_t que_touch = NULL;

/*
  Handle an interrupt triggered when a pad is touched.
  Recognize what pad has been touched and save it in a table.
*/
static void touchsensor_interrupt_cb(void *arg)
{
    int task_awoken = pdFALSE;
    touch_event_t evt;

    evt.intr_mask = touch_pad_read_intr_status_mask();
    evt.pad_status = touch_pad_get_status();
    evt.pad_num = touch_pad_get_current_meas_channel();

    xQueueSendFromISR(que_touch, &evt, &task_awoken);
    if (task_awoken == pdTRUE) {
        portYIELD_FROM_ISR();
    }
}

static void tp_example_set_thresholds(touch_config init)
{
    uint32_t touch_value;
    for (int i = 0; i < init.num_buttons; i++) {
        //read benchmark value
        touch_pad_read_benchmark(init.buttons[i], &touch_value);
        //set interrupt threshold.
        touch_pad_set_thresh(init.buttons[i], touch_value * 0.50);//button_threshold[i]);
        ESP_LOGI(TOUCH_TAG, "touch pad [%d] base %d, thresh %d", \
                 init.buttons[i], touch_value, (uint32_t)(touch_value * 0.50));//button_threshold[i]));
    }
}

static void touchsensor_filter_set(touch_filter_mode_t mode)
{
    /* Filter function */
    touch_filter_config_t filter_info = {
        .mode = mode,           // Test jitter and filter 1/4.
        .debounce_cnt = 1,      // 1 time count.
        .noise_thr = 0,         // 50%
        .jitter_step = 4,       // use for jitter mode.
        .smh_lvl = TOUCH_PAD_SMOOTH_IIR_2,
    };
    touch_pad_filter_set_config(&filter_info);
    touch_pad_filter_enable();
    ESP_LOGI(TOUCH_TAG, "touch pad filter init");
}

static void tp_example_read_task(void *pvParameter)
{
    touch_event_t evt = {0};
    /* Wait touch sensor init done */
    vTaskDelay(50 / portTICK_RATE_MS);

    while (1) {
        int ret = xQueueReceive(que_touch, &evt, (portTickType)portMAX_DELAY);
        if (ret != pdTRUE) {
            continue;
        }

        if (evt.intr_mask & TOUCH_PAD_INTR_MASK_ACTIVE) {
          ESP_LOGI(TOUCH_TAG, "TouchSensor [%d] be activated, status mask 0x%x", evt.pad_num, evt.pad_status);
        }

        if (evt.intr_mask & TOUCH_PAD_INTR_MASK_INACTIVE) {
          ESP_LOGI(TOUCH_TAG, "TouchSensor [%d] be inactivated, status mask 0x%x", evt.pad_num, evt.pad_status);
        }

        if (evt.intr_mask & TOUCH_PAD_INTR_MASK_SCAN_DONE) {
            ESP_LOGI(TOUCH_TAG, "The touch sensor group measurement is done [%d].", evt.pad_num);
        }

        if (evt.intr_mask & TOUCH_PAD_INTR_MASK_TIMEOUT) {
            /* Add your exception handling in here. */
            ESP_LOGI(TOUCH_TAG, "Touch sensor channel %d measure timeout. Skip this exception channel!!", evt.pad_num);
            touch_pad_timeout_resume(); // Point on the next channel to measure.
        }
    }
}


void initTouch(touch_config init)
{
  // Initialize touch pad peripheral, it will start a timer to run a filter
  ESP_LOGI(TOUCH_TAG, "Initializing touch pad");
  /* Initialize touch pad peripheral. */
  touch_pad_init();

  for (int i = 0; i < init.num_buttons; i++) {
      touch_pad_config(init.buttons[i]);
  }

  if (que_touch == NULL) {
      que_touch = xQueueCreate(init.num_buttons, sizeof(touch_event_t));
  }

  /* Filter setting */
  touchsensor_filter_set(TOUCH_PAD_FILTER_IIR_16);
  touch_pad_timeout_set(true, SOC_TOUCH_PAD_THRESHOLD_MAX);
  /* Register touch interrupt ISR, enable intr type. */
  touch_pad_isr_register(touchsensor_interrupt_cb, NULL, TOUCH_PAD_INTR_MASK_ALL);
  /* If you have other touch algorithm, you can get the measured value after the `TOUCH_PAD_INTR_MASK_SCAN_DONE` interrupt is generated. */
  touch_pad_intr_enable(TOUCH_PAD_INTR_MASK_ACTIVE | TOUCH_PAD_INTR_MASK_INACTIVE | TOUCH_PAD_INTR_MASK_TIMEOUT);

  /* Enable touch sensor clock. Work mode is "timer trigger". */
  touch_pad_set_fsm_mode(TOUCH_FSM_MODE_TIMER);
  touch_pad_fsm_start();

  // Start a task to show what pads have been touched
  xTaskCreate(&tp_example_read_task, "touch_pad_read_task", 2048, NULL, 5, NULL);


  vTaskDelay(50 / portTICK_RATE_MS);
  tp_example_set_thresholds(init);

  /* Denoise setting at TouchSensor 0. */
  touch_pad_denoise_t denoise = {
    /* The bits to be cancelled are determined according to the noise level. */
    .grade = TOUCH_PAD_DENOISE_BIT4,
    /* By adjusting the parameters, the reading of T0 should be approximated to the reading of the measured channel. */
    .cap_level = TOUCH_PAD_DENOISE_CAP_L4,
  };
  touch_pad_denoise_set_config(&denoise);
  touch_pad_denoise_enable();
  ESP_LOGI(TOUCH_TAG, "Touch pad initialized");
}
