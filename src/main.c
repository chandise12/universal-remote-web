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

void app_main(){

    ESP_ERROR_CHECK(nvs_flash_init()); 
    wifi_init();
    http_init();
    pwm_init();

    // while(1){
    //     pulse_us(10000);  // ON
    //     pulse_us(10000);  // OFF
    //     pulse_us(10000);  // ON
    //     pulse_us(10000);  // OFF
    // }


    xTaskCreate(get_task, "GET Task", 4096, NULL, 5, NULL);
    // get_task();

}