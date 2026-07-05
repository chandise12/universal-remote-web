#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "esp_timer.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/ledc.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

static const char *TAG = "IR_SYSTEM";

#define MAX_PULSES 100

// ---------------- DATA STRUCT ----------------

typedef struct {
    int64_t data[MAX_PULSES];
    uint8_t length;
} IRMessage;

// ---------------- GLOBALS ----------------

QueueHandle_t RxQueue;
QueueHandle_t TxQueue;

static esp_timer_handle_t msg_timer;
static TaskHandle_t decodeTaskHandle;

// ---------------- TX STATE FLAG ----------------

static volatile bool tx_active = false;

// ---------------- TIMER CALLBACK ----------------

static void msg_timer_callback(void *arg)
{
    xTaskNotifyGive(decodeTaskHandle);
}

// ---------------- ISR (RX) ----------------

void IRAM_ATTR ir_receiver_isr_handler(void* arg)
{
    // prevent self-trigger during TX
    if (tx_active) {
        return;
    }

    int64_t now = esp_timer_get_time();

    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xQueueSendFromISR(RxQueue, &now, &xHigherPriorityTaskWoken);

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

// ---------------- DECODE TASK ----------------

void decodeTask(void *pvParameters)
{
    int64_t now;
    int64_t last = 0;

    IRMessage msg;
    msg.length = 0;

    while (1)
    {
        // end of message timeout
        if (ulTaskNotifyTake(pdTRUE, 0))
        {
            if (msg.length > 0)
            {
                xQueueSend(TxQueue, &msg, portMAX_DELAY);
            }

            msg.length = 0;
            last = 0;
            continue;
        }

        if (xQueueReceive(RxQueue, &now, portMAX_DELAY))
        {
            if (last == 0)
            {
                last = now;
                continue;
            }

            int64_t delta = now - last;
            last = now;

            if (msg.length < MAX_PULSES)
            {
                msg.data[msg.length++] = delta;
            }

            // restart end-of-message timer
            esp_timer_stop(msg_timer);
            esp_timer_start_once(msg_timer, 50000); // 50ms gap
        }
    }
}

// ---------------- TX TASK ----------------

void txTask(void *pvParameters)
{
    IRMessage msg;

    while (1)
    {
        if (xQueueReceive(TxQueue, &msg, portMAX_DELAY))
        {
            ESP_LOGI(TAG, "TX START");

            // ---------------- DISABLE RX ----------------
            tx_active = true;
            gpio_intr_disable(GPIO_NUM_18);

            vTaskDelay(pdMS_TO_TICKS(1000));

            for (int i = 0; i < msg.length; i++)
            {
                if (i % 2 == 0)
                {
                    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 128);
                }
                else
                {
                    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0);
                }

                ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);

                esp_timer_start_once(msg_timer, msg.data[i]);
                ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
            }

            // ---------------- RE-ENABLE RX ----------------
            gpio_intr_enable(GPIO_NUM_18);
            tx_active = false;

            ESP_LOGI(TAG, "TX END");
        }
    }
}

// ---------------- APP MAIN ----------------

void app_main()
{
    ESP_LOGI(TAG, "Starting IR system");

    RxQueue = xQueueCreate(100, sizeof(int64_t));
    TxQueue = xQueueCreate(5, sizeof(IRMessage));

    if (!RxQueue || !TxQueue)
    {
        ESP_LOGE(TAG, "Queue creation failed");
        return;
    }

    // timer
    esp_timer_create_args_t timer_args = {
        .callback = &msg_timer_callback,
        .arg = NULL,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "ir_timer"
    };

    esp_timer_create(&timer_args, &msg_timer);

    // tasks
    xTaskCreate(decodeTask, "decodeTask", 4096, NULL, 10, &decodeTaskHandle);
    xTaskCreate(txTask, "txTask", 4096, NULL, 10, NULL);

    // LEDC setup
    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_8_BIT,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = 38000,
        .clk_cfg = LEDC_AUTO_CLK
    };

    ledc_channel_config_t ledc_channel = {
        .gpio_num = GPIO_NUM_21,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,
        .hpoint = 0
    };

    ledc_timer_config(&ledc_timer);
    ledc_channel_config(&ledc_channel);

    // GPIO RX setup
    gpio_config_t receiver_config = {
        .pin_bit_mask = 1ULL << GPIO_NUM_18,
        .mode = GPIO_MODE_INPUT,
        .intr_type = GPIO_INTR_ANYEDGE
    };

    gpio_config(&receiver_config);

    gpio_install_isr_service(0);
    gpio_isr_handler_add(GPIO_NUM_18, ir_receiver_isr_handler, NULL);

    ESP_LOGI(TAG, "System ready");
}