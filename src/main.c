#include "wifi.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "http.h"

void app_main(){

    ESP_ERROR_CHECK(nvs_flash_init()); 
    wifi_init();
    http_init();

    xTaskCreate(get_task, "GET Task", 4096, NULL, 5, NULL);

    while(1);
}