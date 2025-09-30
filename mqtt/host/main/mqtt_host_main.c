#include <stdio.h>
#include <string.h>
#include "esp_event.h"
#include "esp_log.h"
#include "mqtt_client.h"
#include "esp_transport.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

esp_transport_handle_t esp_transport_tcp_init2(void);

static const char *TAG = "mqtt_host";

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;
    switch (event->event_id) {
    case MQTT_EVENT_BEFORE_CONNECT:
        ESP_LOGI(TAG, "MQTT_EVENT_BEFORE_CONNECT");
        break;
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED (session_present=%d)", event->session_present);
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        break;
    default:
        ESP_LOGI(TAG, "MQTT event id=%d", (int)event->event_id);
        break;
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "Starting MQTT host with UDP transport mock");
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Inject the custom UDP-backed transport
    esp_transport_handle_t udp_transport = esp_transport_tcp_init2();

    esp_mqtt_client_config_t cfg = {
        .broker.address.uri = "mqtt://udp.local",
        .network.timeout_ms = 2000,
        .network.transport = udp_transport,
    };

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&cfg);
    if (!client) {
        ESP_LOGE(TAG, "Failed to init mqtt client");
        return;
    }
    ESP_ERROR_CHECK(esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL));
    ESP_ERROR_CHECK(esp_mqtt_client_start(client));

    // Keep running; IDF linux target main thread stays alive
    while (1) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

int main(void)
{
    app_main();
    return 0;
}