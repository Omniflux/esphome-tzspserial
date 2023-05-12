#include "TZSPSerial.h"

namespace esphome {
namespace tzspserial {

void TZSPSerial::setup() {
    this->socket = ::socket(AF_INET, SOCK_DGRAM, 0);
    destination.sin_family = AF_INET;
    destination.sin_port = htons(this->tzsp_port_);
    destination.sin_addr.s_addr = this->tzsp_ip_;

    if (xTaskCreate(uart_event_task, "UART_Event", 4096, this, 12, NULL) != pdPASS) {
        ESP_LOGE(TAG, "Failed to create event task");
        this->mark_failed();
        return;
    }
}

void TZSPSerial::dump_config() {
    ESP_LOGCONFIG(TAG, "TZSPSerial");
    ESP_LOGCONFIG(TAG, "  Destination: %s:%u", this->tzsp_ip_.str().c_str(), this->tzsp_port_);
    ESP_LOGCONFIG(TAG, "  Protocol: %u", this->tzsp_protocol_);
    ESP_LOGCONFIG(TAG, "  Frame size: %u", this->frame_size_);
    ESP_LOGCONFIG(TAG, "  Inverted: %s", this->inverted_ ? "YES" : "NO");
}

void uart_event_task(void* pvParameters) {
    auto tz = static_cast<TZSPSerial*>(pvParameters);

    uart_event_t event;
    const auto tzsp_header_len = 5;
    uint8_t tzsp_data[tzsp_header_len + tz->frame_size_] = { 1, 1, 0, 0, 1 };
    auto buffer = tzsp_data + tzsp_header_len;
    auto protocol = htons(tz->tzsp_protocol_);

    memcpy(tzsp_data + 2, &protocol, sizeof(protocol));

    for (;;) {
        if(xQueueReceive*(static_cast<uart::IDFUARTComponent*>(tz->parent_)->get_uart_event_queue(), &event, portMAX_DELAY)) {
            switch(event.type) {
                [[likely]] case UART_DATA:
                    size_t bufferLen;

                    uart_get_buffered_data_len(static_cast<uart::IDFUARTComponent*>(tz->parent_)->get_hw_serial_number(), &bufferLen);
                    if (auto discard = bufferLen % tz->frame_size_) {
                        tz->read_array(buffer, discard);
                        ESP_LOGD(TAG, "Discarded %d bytes", discard);
                    }

                    for (auto i = 0; i < event.size / tz->frame_size_; i++) {
                        tz->read_array(buffer, tz->frame_size_);

                        if (tz->inverted_) {
                            for (auto j = 0; j < tz->frame_size_; j++) {
                                buffer[j] ^= 0xFF;
                            }
                        }

                        ::sendto(tz->socket, tzsp_data, sizeof(tzsp_data), 0, reinterpret_cast<sockaddr*>(&tz->destination), sizeof(tz->destination));
                    }

                    break;
                default:
                    ESP_LOGI(TAG, "Unhandled UART event type: %d", event.type);
                    break;
            }
        }
    }
}

}
}