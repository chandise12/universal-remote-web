
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

#define MAX_HTTP_RECV_BUFFER 512
#define MAX_HTTP_OUTPUT_BUFFER 2048

static const char *TAG = "HTTP_CLIENT";

static char local_response_buffer[MAX_HTTP_OUTPUT_BUFFER + 1];
static int output_len = 0;

esp_http_client_handle_t get_client = NULL;
esp_http_client_handle_t post_client = NULL;



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
            ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
            int mbedtls_err = 0;
            esp_err_t err = esp_tls_get_and_clear_last_error((esp_tls_error_handle_t)evt->data, &mbedtls_err, NULL);
            if (err != 0) {
                ESP_LOGI(TAG, "Last esp error code: 0x%x", err);
                ESP_LOGI(TAG, "Last mbedtls failure: 0x%x", mbedtls_err);
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

    return ESP_OK;
}

esp_err_t http_deinit(){
    esp_http_client_cleanup(get_client);
    esp_http_client_cleanup(post_client);

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
    else {
        ESP_LOGE(TAG, "HTTP GET failed: %s", esp_err_to_name(err));
    }

    return err;
}

void get_task(void)
{
    http_get_command();

    // Now parse local_response_buffer here
    cJSON *json = cJSON_Parse(local_response_buffer);
    if (json == NULL) {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            ESP_LOGE(TAG, "Error: %s\n", error_ptr);
        }
        cJSON_Delete(json);
    }

    cJSON *status = cJSON_GetObjectItem(json, "status");
    if (cJSON_IsString(status) && (status->valuestring != NULL)) {
        printf("Name: %s\n", status->valuestring);
    }
}














// static void http_rest_with_url(void)
// {
//     // Declare local_response_buffer with size (MAX_HTTP_OUTPUT_BUFFER + 1) to prevent out of bound access when
//     // it is used by functions like strlen(). The buffer should only be used upto size MAX_HTTP_OUTPUT_BUFFER
//     char local_response_buffer[MAX_HTTP_OUTPUT_BUFFER + 1] = {0};
//     /**
//      * NOTE: All the configuration parameters for http_client must be specified either in URL or as host and path parameters.
//      * If host and path parameters are not set, query parameter will be ignored. In such cases,
//      * query parameter should be specified in URL.
//      *
//      * If URL as well as host and path parameters are specified, values of host and path will be considered.
//      */
//     esp_http_client_config_t config = {
//         .host = CONFIG_EXAMPLE_HTTP_ENDPOINT,
//         .path = "/get",
//         .query = "esp",
//         .event_handler = _http_event_handler,
//         .user_data = local_response_buffer,        // Pass address of local buffer to get response
//         .disable_auto_redirect = true,
//     };
//     ESP_LOGI(TAG, "HTTP request with url =>");
//     esp_http_client_handle_t client = esp_http_client_init(&config);

//     // GET
//     esp_err_t err = esp_http_client_perform(client);
//     if (err == ESP_OK) {
//         ESP_LOGI(TAG, "HTTP GET Status = %d, content_length = %"PRId64,
//                 esp_http_client_get_status_code(client),
//                 esp_http_client_get_content_length(client));
//     } else {
//         ESP_LOGE(TAG, "HTTP GET request failed: %s", esp_err_to_name(err));
//     }
//     ESP_LOG_BUFFER_HEX(TAG, local_response_buffer, strlen(local_response_buffer));

//     // POST
//     const char *post_data = "{\"field1\":\"value1\"}";
//     esp_http_client_set_url(client, "http://"CONFIG_EXAMPLE_HTTP_ENDPOINT"/post");
//     esp_http_client_set_method(client, HTTP_METHOD_POST);
//     esp_http_client_set_header(client, "Content-Type", "application/json");
//     esp_http_client_set_post_field(client, post_data, strlen(post_data));
//     err = esp_http_client_perform(client);
//     if (err == ESP_OK) {
//         ESP_LOGI(TAG, "HTTP POST Status = %d, content_length = %"PRId64,
//                 esp_http_client_get_status_code(client),
//                 esp_http_client_get_content_length(client));
//     } else {
//         ESP_LOGE(TAG, "HTTP POST request failed: %s", esp_err_to_name(err));
//     }

//     //PUT
//     esp_http_client_set_url(client, "http://"CONFIG_EXAMPLE_HTTP_ENDPOINT"/put");
//     esp_http_client_set_method(client, HTTP_METHOD_PUT);
//     err = esp_http_client_perform(client);
//     if (err == ESP_OK) {
//         ESP_LOGI(TAG, "HTTP PUT Status = %d, content_length = %"PRId64,
//                 esp_http_client_get_status_code(client),
//                 esp_http_client_get_content_length(client));
//     } else {
//         ESP_LOGE(TAG, "HTTP PUT request failed: %s", esp_err_to_name(err));
//     }

//     //PATCH
//     esp_http_client_set_url(client, "http://"CONFIG_EXAMPLE_HTTP_ENDPOINT"/patch");
//     esp_http_client_set_method(client, HTTP_METHOD_PATCH);
//     esp_http_client_set_post_field(client, NULL, 0);
//     err = esp_http_client_perform(client);
//     if (err == ESP_OK) {
//         ESP_LOGI(TAG, "HTTP PATCH Status = %d, content_length = %"PRId64,
//                 esp_http_client_get_status_code(client),
//                 esp_http_client_get_content_length(client));
//     } else {
//         ESP_LOGE(TAG, "HTTP PATCH request failed: %s", esp_err_to_name(err));
//     }
//     esp_http_client_cleanup(client);
// }