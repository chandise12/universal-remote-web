#include "wifi.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "http.h"
#include "rx_tx.h"
#include "driver/ledc.h"
#include "esp_timer.h"
#include "driver/gptimer.h"
#include "esp_timer.h"

extern TaskHandle_t get_task_handle;

void app_main(){

    ESP_ERROR_CHECK(nvs_flash_init()); 
    wifi_init();
    http_init();
    pwm_init();
    queue_init();

    xTaskCreate(get_task, "GET Task", 4096, NULL, 5, &get_task_handle);
    xTaskCreate(upload_message, "POST message", 4096, NULL, 6, NULL);
    // get_task();

}