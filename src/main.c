#include "wifi.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "http.h"

void app_main(){

    ESP_ERROR_CHECK(nvs_flash_init()); 
    wifi_init();
    http_init();

    while(1){
        get_task();
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}