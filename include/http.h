#pragma once 
#include "rx_tx.h"

void get_task();
void clear_task();
void upload_message(void *arg);
void post_msg_http(ir_message_t *message);
esp_err_t http_init();
esp_err_t http_deinit();
void queue_init();
