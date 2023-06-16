#include <vector>

#include <esphome/components/network/ip_address.h>

#include "TZSPSerial.h"

namespace esphome {
namespace tzspserial {

void TZSPSerial::setup() {
    if (xTaskCreate([](void* o){ static_cast<TZSPSerial*>(o)->uart_event_task(); }, "UART_Event", 4096, this, 12, NULL) != pdPASS) {
        ESP_LOGE(TAG, "Failed to create event task");
        this->mark_failed();
        return;
    }
}

void TZSPSerial::dump_config() {
    ESP_LOGCONFIG(TAG, "TZSPSerial");
    ESP_LOGCONFIG(TAG, "  Destination: %s:%u", static_cast<network::IPAddress>(this->tzsp_sockaddr_in_.sin_addr.s_addr).str().c_str(), ntohs(this->tzsp_sockaddr_in_.sin_port));
    ESP_LOGCONFIG(TAG, "  Protocol: %u", ntohs(this->tzsp_protocol_));
    ESP_LOGCONFIG(TAG, "  Frame size: %u", this->frame_size_);
    ESP_LOGCONFIG(TAG, "  Inverted: %s", this->inverted_ ? "YES" : "NO");
}

void TZSPSerial::uart_event_task() {
    uart_event_t event;
    std::vector<uint8_t> buffer(this->frame_size_);

    for (;;) {
        if(xQueueReceive(*static_cast<uart::IDFUARTComponent*>(this->parent_)->get_uart_event_queue(), &event, portMAX_DELAY)) {
            switch(event.type) {
                [[likely]] case UART_DATA:
                    size_t bufferLen;

                    uart_get_buffered_data_len(static_cast<uart::IDFUARTComponent*>(this->parent_)->get_hw_serial_number(), &bufferLen);
                    if (auto discard = bufferLen % buffer.size()) {
                        this->read_array(buffer.data(), discard);
                        ESP_LOGD(TAG, "Discarded %d bytes", discard);
                    }

                    for (auto i = 0; i < event.size / buffer.size(); i++) {
                        this->read_array(buffer.data(), buffer.size());

                        if (this->inverted_)
                            for (auto &b : buffer)
                                b ^= 0xFF;

                        this->tzsp_send(buffer);
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