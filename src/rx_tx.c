#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "rx_tx.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "driver/ledc.h"
#include "esp_timer.h"
#include "driver/gptimer.h"
#include "esp_event.h"
#include "esp_log.h"
#include <stdbool.h>


#define IR_TIMEOUT_US 10000 // 10 ms of inactivity indicates end of message

static const char *TAG = "RX_TX";

static bool pulse_high = false;

QueueHandle_t tx_queue;
volatile bool signal_ready = false;
uint32_t ir_buffer[MAX_ARR_SIZE];
volatile uint16_t ir_index = 0;
volatile uint32_t last_edge_time = 0;
volatile bool ir_capturing = false;
esp_timer_handle_t ir_timeout_timer;


void ir_timeout_callback(void *arg)
{
    if(!ir_capturing) return;

    ir_capturing = false;

    ir_message_t msg;

    msg.length = ir_index;

    memcpy(msg.pulses, ir_buffer, ir_index * sizeof(uint32_t));

    ir_index = 0;

    ESP_LOGI(TAG, "END OF RECEIVED MESSAGE");

    xQueueSend(tx_queue, &msg, 0);
}

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

    const esp_timer_create_args_t timer_args = {
        .callback = &ir_timeout_callback,
        .name = "ir_timeout"
    };

    esp_timer_create(&timer_args, &ir_timeout_timer);


    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_ANYEDGE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << RCV_GPIO),
        .pull_up_en = 1,
    };

    gpio_config(&io_conf);

    gpio_install_isr_service(0);
    gpio_isr_handler_add(RCV_GPIO, ir_gpio_isr_handler, NULL);


    ESP_LOGI(TAG, "TIMER INIT DONE");
}

void ir_gpio_isr_handler(void *arg)
{
    if(!ir_capturing) return;

    uint32_t now = (uint32_t)esp_timer_get_time(); // microseconds

    uint32_t duration = now - last_edge_time;
    last_edge_time = now;

    if(ir_index < MAX_ARR_SIZE && ir_index > 0)
    {
        ir_buffer[ir_index-1] = duration;
    }
    ir_index++;

    // restart timeout timer
    esp_timer_stop(ir_timeout_timer);
    esp_timer_start_once(ir_timeout_timer, IR_TIMEOUT_US);
}



void pulse_us(uint32_t duration_us) // set a pin to toggle for a specific number of us, then exit
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

    gpio_intr_disable(RCV_GPIO);

    ESP_LOGI(TAG, "TRANSMIT START");

    for(int i = 0; i < message_len; i++){
        pulse_us(message[i]);
    }

    //set signal back to low
    if(pulse_high){
        pulse_us(1);
    }

    gpio_intr_enable(RCV_GPIO);

    ESP_LOGI(TAG, "TRANSMIT FINISHED");
}



void ir_start_capture()
{
    ir_index = 0;
    ir_capturing = true;

    last_edge_time = esp_timer_get_time();
    
    ESP_LOGI(TAG, "Started IR capture");
}


