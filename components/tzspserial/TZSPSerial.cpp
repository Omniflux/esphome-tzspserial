#include "TZSPSerial.h"

namespace esphome {
namespace tzspserial {

void TZSPSerial::setup() {
    this->socket = ::socket(AF_INET, SOCK_DGRAM, 0);
    destination.sin_family = AF_INET;
    destination.sin_port = htons(this->tzsp_port_);
    destination.sin_addr.s_addr = this->tzsp_ip_;

    if (xTaskCreate([](void* o){ static_cast<TZSPSerial*>(o)->uart_event_task(); }, "UART_Event", 4096, this, 12, NULL) != pdPASS) {
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

void TZSPSerial::uart_event_task() {
    uart_event_t event;
    const auto tzsp_header_len = 5;
    uint8_t tzsp_data[tzsp_header_len + this->frame_size_] = { 1, 1, 0, 0, 1 };
    auto buffer = tzsp_data + tzsp_header_len;
    auto protocol = htons(this->tzsp_protocol_);

    memcpy(tzsp_data + 2, &protocol, sizeof(protocol));

    for (;;) {
        if(xQueueReceive(*static_cast<uart::IDFUARTComponent*>(this->parent_)->get_uart_event_queue(), &event, portMAX_DELAY)) {
            switch(event.type) {
                [[likely]] case UART_DATA:
                    size_t bufferLen;

                    uart_get_buffered_data_len(static_cast<uart::IDFUARTComponent*>(this->parent_)->get_hw_serial_number(), &bufferLen);
                    if (auto discard = bufferLen % this->frame_size_) {
                        this->read_array(buffer, discard);
                        ESP_LOGD(TAG, "Discarded %d bytes", discard);
                    }

                    for (auto i = 0; i < event.size / this->frame_size_; i++) {
                        this->read_array(buffer, this->frame_size_);

                        if (this->inverted_) {
                            for (auto j = 0; j < this->frame_size_; j++) {
                                buffer[j] ^= 0xFF;
                            }
                        }

                        ::sendto(this->socket, tzsp_data, sizeof(tzsp_data), 0, reinterpret_cast<sockaddr*>(&this->destination), sizeof(this->destination));
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