#include "rx_tx.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "driver/ledc.h"
#include "esp_timer.h"
#include "driver/gptimer.h"
#include "esp_event.h"
#include "esp_log.h"
#include <stdbool.h>


static const char *TAG = "RX_TX";

static bool pulse_high = false;


#define PWM_GPIO 21


void pwm_init()
{
    ledc_timer_config_t timer = {
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .timer_num = LEDC_TIMER_0,
        .duty_resolution = LEDC_TIMER_8_BIT,
        .freq_hz = 38000,  // 38kHz for IR
        .clk_cfg = LEDC_AUTO_CLK
    };
    ledc_timer_config(&timer);

    ledc_channel_config_t channel = {
        .gpio_num = PWM_GPIO,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0, // start OFF
        .hpoint = 0
    };
    ledc_channel_config(&channel);

    ESP_LOGI(TAG, "PWM INIT DONE");
}


void pulse_us(uint32_t duration_us)
{
        
    if(pulse_high){ // if the signal was already high, set it low
        // Turn OFF
        ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, 0);
        ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
    }else{ // otherwise, set it low
        // set 50% duty cycle
        ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, 128);
        ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
    }
    pulse_high = !pulse_high;

    int64_t start = esp_timer_get_time();

    while ((esp_timer_get_time() - start) < duration_us) {
        // loop for (duration_us) us
    }
}


void transmit_message(uint32_t *message, size_t message_len){
    // disable receiver interrupts

    ESP_LOGI(TAG, "TRANSMIT START");

    for(int i = 0; i < message_len; i++){
        pulse_us(message[i]);
    }

    //set signal back to low
    if(pulse_high){
        pulse_us(1);
    }

    // reenable receiver interrupts
}