#pragma once

#include <string.h>
#include <stdint.h>

#define MAX_ARR_SIZE 100
#define PWM_GPIO 21
#define RCV_GPIO 18

typedef struct {
    uint32_t pulses[MAX_ARR_SIZE];
    uint16_t length;
    int remote_id;
    int button_id;
} ir_message_t;


void pwm_init();
void pulse_us(uint32_t duration_us);
void transmit_message(uint32_t *message, size_t message_len);
void ir_gpio_isr_handler(void *arg);
void ir_start_capture();
