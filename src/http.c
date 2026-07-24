
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>
#include <string.h>
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_http_client.h"
#include "esp_tls.h"
#include "cJSON.h"
#include "LOGIN.h"
#include "rx_tx.h"

#define GET_TASK_PERIOD 1000 // 1000ms = 1 s
#define MAX_HTTP_RECV_BUFFER 512
#define MAX_HTTP_OUTPUT_BUFFER 2048

static const char *TAG = "HTTP_CLIENT";

static char local_response_buffer[MAX_HTTP_OUTPUT_BUFFER + 1];
static int output_len = 0;

esp_http_client_handle_t get_client = NULL;
esp_http_client_handle_t post_client_clear = NULL;
esp_http_client_handle_t post_client_upload = NULL;

int curr_remote_id = -1;
int curr_button_id = -1; 

TaskHandle_t get_task_handle;
extern QueueHandle_t tx_queue;
extern volatile bool ir_capturing;



esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    static int _output_len;       // Stores number of bytes read
    
    switch(evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGD(TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
            break;
        case HTTP_EVENT_ON_DATA:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);

            // Clean the buffer in case of a new request
             if (evt->user_data) {

                // Copy received chunk into the user buffer
                int copy_len = 0;

                if(evt->data_len <= (MAX_HTTP_OUTPUT_BUFFER - _output_len - 1)){
                    copy_len = evt->data_len;
                }else{
                    copy_len = (MAX_HTTP_OUTPUT_BUFFER - _output_len - 1);
                }

                if (copy_len > 0) {
                    memcpy(
                        (char *)evt->user_data + _output_len,
                        evt->data,
                        copy_len
                    );

                    _output_len += copy_len;

                    // Keep it a valid C string for JSON parsing
                    ((char *)evt->user_data)[_output_len] = '\0';
                }
            }
            break;

        case HTTP_EVENT_ON_FINISH:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
            _output_len = 0;
            break;

        case HTTP_EVENT_DISCONNECTED:
            // ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
            int mbedtls_err = 0;
            esp_err_t err = esp_tls_get_and_clear_last_error((esp_tls_error_handle_t)evt->data, &mbedtls_err, NULL);
            if (err != 0) {
                // ESP_LOGI(TAG, "Last esp error code: 0x%x", err);
                // ESP_LOGI(TAG, "Last mbedtls failure: 0x%x", mbedtls_err);
            }
            _output_len = 0;
            break;

        case HTTP_EVENT_REDIRECT:
            ESP_LOGD(TAG, "HTTP_EVENT_REDIRECT");
            break;

        default:
            break;
    }
    return ESP_OK;
}

esp_err_t http_init(){

    // INITIALIZE "GET" STATE
    esp_http_client_config_t config = {
        .url = COMMAND_URL,
        .event_handler = _http_event_handler,
        .user_data = local_response_buffer,
    };

    get_client = esp_http_client_init(&config);

    if (get_client == NULL) {
        ESP_LOGE(TAG, "Failed to initialize HTTP GET client");
        return ESP_FAIL;
    }

    esp_http_client_set_method(get_client, HTTP_METHOD_GET);


    // INITIALIZE "POST" CLEAR TASK
    esp_http_client_config_t post_config_clr = {
        .url = CLEAR_URL,
        .event_handler = _http_event_handler,
        .user_data = NULL,
    };

    post_client_clear = esp_http_client_init(&post_config_clr);

    if (post_client_clear == NULL) {
        ESP_LOGE(TAG, "Failed to initialize HTTP POST clear client");
        return ESP_FAIL;
    }

    esp_http_client_set_method(post_client_clear, HTTP_METHOD_POST);


    // INITIALIZE "POST" UPLOAD TASK
    esp_http_client_config_t post_config_upld = {
        .url = UPLOAD_URL,
        .event_handler = _http_event_handler
    };

    post_client_upload = esp_http_client_init(&post_config_upld);

    if (post_client_upload == NULL) {
        ESP_LOGE(TAG, "Failed to initialize HTTP POST upload client");
        return ESP_FAIL;
    }

    esp_http_client_set_method(post_client_upload, HTTP_METHOD_POST);


    return ESP_OK;
}

void queue_init()
{
    tx_queue = xQueueCreate(5, sizeof(ir_message_t));

    if(tx_queue == NULL){
        ESP_LOGE(TAG, "Failed to create IR queue");
    }
}

esp_err_t http_deinit(){
    esp_http_client_cleanup(get_client);
    esp_http_client_cleanup(post_client_clear);

    return ESP_OK;
}

esp_err_t http_get_command(void)
{
    memset(local_response_buffer, 0, sizeof(local_response_buffer));
    output_len = 0;

    esp_err_t err = esp_http_client_perform(get_client);

    if (err == ESP_OK) {

        int status = esp_http_client_get_status_code(get_client);

        ESP_LOGI(TAG,
                 "GET Status = %d, content length = %" PRId64,
                 status,
                 esp_http_client_get_content_length(get_client));


        if (status == 200) {
            ESP_LOGI(TAG, "Response: %s", local_response_buffer);
        }
        else {
            ESP_LOGW(TAG,  "Server returned status %d", status);
        }
    }

    return err;
}

uint32_t *json_to_int_arr(cJSON *json, size_t *out_len)
{
    if (json == NULL || out_len == NULL) {
        return NULL;
    }

    cJSON *message = cJSON_GetObjectItem(json, "message");
    if (!cJSON_IsArray(message)) {
        printf("is not an array\n");
        return NULL;
    }

    size_t len = cJSON_GetArraySize(message);
    printf("length of message: %d\n", len);

    if (len == 0) {
        *out_len = 0;
        return NULL;
    }

    uint32_t *arr = malloc(len * sizeof(uint32_t));
    if (arr == NULL) {
        return NULL;
    }

    for (size_t i = 0; i < len; i++) {
        cJSON *item = cJSON_GetArrayItem(message, i);

        if (cJSON_IsNumber(item)) {
            arr[i] = (uint32_t)item->valuedouble;
        } else {
            // Handle unexpected type
            arr[i] = 0;
        }
    }

    *out_len = len;
    return arr;
}


void clear_task(){
    esp_err_t err = esp_http_client_perform(post_client_clear);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "HTTP POST Status = %d, content_length = %"PRId64,
            esp_http_client_get_status_code(post_client_clear));
    } else {
        // ESP_LOGE(TAG, "HTTP POST request failed: %s", esp_err_to_name(err));
    }
}

void post_msg_http(ir_message_t *message){

    cJSON *json = cJSON_CreateObject();
    cJSON_AddNumberToObject(json, "length", message->length);
    cJSON_AddNumberToObject(json, "remote_id", curr_remote_id);
    cJSON_AddNumberToObject(json, "button_id", curr_button_id);

    curr_remote_id = -1;
    curr_button_id = -1; 

    cJSON *array = cJSON_AddArrayToObject(json, "message");

    for(int i = 0; i < message->length; i++)
    {
        cJSON_AddItemToArray(array, cJSON_CreateNumber(message->pulses[i]));
    }

    char *json_string = cJSON_PrintUnformatted(json);
    ESP_LOGI(TAG, "Sending: %s", json_string);

    esp_http_client_set_header(post_client_upload, "Content-Type", "application/json");
    esp_http_client_set_post_field(post_client_upload, json_string, strlen(json_string));

    esp_err_t err = esp_http_client_perform(post_client_upload);
        if (err == ESP_OK) {
            ESP_LOGI(TAG, "HTTP POST Status = %d"PRId64,
                    esp_http_client_get_status_code(post_client_upload));
        } else {
            ESP_LOGE(TAG, "HTTP POST request failed: %s", esp_err_to_name(err));
        }

    cJSON_Delete(json);
    free(json_string);

    // wake get_task
    vTaskNotifyGiveFromISR(get_task_handle, NULL);
}

void upload_message(void *arg){

    ir_message_t message;

    while(1){
        if(xQueueReceive(tx_queue, &message, portMAX_DELAY))
        {
            ESP_LOGI(TAG, "Sending IR signal");

            post_msg_http(&message);
        }
    }
}

void get_task(void){
    while(1){

        http_get_command();

        // Now parse local_response_buffer here
        cJSON *json = cJSON_Parse(local_response_buffer);
        if (json == NULL) {
            const char *error_ptr = cJSON_GetErrorPtr();
            if (error_ptr != NULL) {
                ESP_LOGE(TAG, "Error: %s\n", error_ptr);
            }
            cJSON_Delete(json);

            continue;
        }
        ESP_LOGI(TAG, "RAW JSON: %s", local_response_buffer);

        cJSON *status = cJSON_GetObjectItem(json, "status");
        if (!cJSON_IsString(status) || (status->valuestring == NULL)) {
            continue;
        }

        char *transmit_str = "TRANSMIT";
        char *listen_str = "LISTEN";

        if( strcmp(status->valuestring, transmit_str) == 0 ){ // check if command is TRANSMIT
           
            ESP_LOGE(TAG, "COMMAND IS TRANSIT\n");

            size_t msg_len = 0;
            uint32_t *message = json_to_int_arr(json, &msg_len);
            
            if(message == NULL || msg_len == 0){
                continue;
            }

            //transmit message
            transmit_message(message, msg_len);

            free(message);

        }else if( strcmp(status->valuestring, listen_str) == 0 ){
            ESP_LOGE(TAG, "COMMAND IS LISTEN\n");
            cJSON *remote_id = cJSON_GetObjectItem(json, "remote_id");
            cJSON *button_id = cJSON_GetObjectItem(json, "button_id");

            curr_remote_id = remote_id->valueint;
            curr_button_id = button_id->valueint;

            printf("remote: %d, button: %d\n", curr_remote_id, curr_button_id);

            ir_start_capture(remote_id, button_id);

            ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        }else{
            ESP_LOGE(TAG, "COMMAND IS %s\n", status->valuestring);
        }

        cJSON_Delete(json);
        
        // clear task on database end
        clear_task();

        vTaskDelay(pdMS_TO_TICKS(GET_TASK_PERIOD));
    }
}