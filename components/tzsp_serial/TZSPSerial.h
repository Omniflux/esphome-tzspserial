#pragma once

#include <esphome/core/component.h>
#include <esphome/components/uart/uart.h>
#include <esphome/components/uart/uart_component_esp_idf.h>

#include <esphome/components/tzsp/tzsp.h>

namespace esphome::tzspserial {

class TZSPSerial : public Component, public uart::UARTDevice, public tzsp::TZSPSender {
  public:
    TZSPSerial(uart::IDFUARTComponent *parent) : uart::UARTDevice(parent) {}

    void setup() override;
    void dump_config() override;

    void set_frame_size(size_t frame_size) { this->frame_size_ = frame_size; }
    void set_symbol_timeout(uint8_t symbol_timeout) { this->symbol_timeout_ = symbol_timeout; }
    void set_inverted(bool inverted) { this->inverted_ = inverted; }
    void set_tzsp_discard_protocol(uint16_t tzsp_discard_protocol) { this->tzsp_discard_protocol_ = htons(tzsp_discard_protocol); }

  protected:
    size_t frame_size_{};
    uint8_t symbol_timeout_{};
    bool inverted_{};
    uint16_t tzsp_discard_protocol_{};

  private:
    [[noreturn]] void uart_event_task();
    void load_buffer(std::vector<uint8_t>& buffer);
};

}
