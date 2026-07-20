#pragma once

#include <string.h>
#include <stdint.h>


void pwm_init();
void pulse_us(uint32_t duration_us);
void transmit_message(uint32_t *message, size_t message_len);
