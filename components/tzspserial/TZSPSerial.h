#pragma once

#include <esphome/core/component.h>
#include <esphome/components/network/ip_address.h>
#include <esphome/components/uart/uart.h>
#include <esphome/components/uart/uart_component_esp_idf.h>
#include <sys/socket.h>

namespace esphome {
namespace tzspserial {

static const auto TAG = "tzsp_serial";

class TZSPSerial : public Component, public uart::UARTDevice {
  public:
    TZSPSerial(uart::IDFUARTComponent *parent) : uart::UARTDevice(parent) {}

    void setup() override;
    void dump_config() override;
    float get_setup_priority() const override { return esphome::setup_priority::AFTER_WIFI; }

    void set_tzsp_ip(network::IPAddress tzsp_ip) { this->tzsp_ip_ = tzsp_ip; }
    void set_tzsp_port(uint16_t tzsp_port) { this->tzsp_port_ = tzsp_port; }
    void set_tzsp_protocol(uint16_t tzsp_protocol) { this->tzsp_protocol_ = tzsp_protocol; }
    void set_frame_size(size_t frame_size) { this->frame_size_ = frame_size; }
    void set_inverted(size_t inverted) { this->inverted_ = inverted; }

  protected:
    network::IPAddress tzsp_ip_;
    uint16_t tzsp_port_;
    uint16_t tzsp_protocol_;
    size_t frame_size_;
    bool inverted_;

  private:
    int socket;
    struct sockaddr_in destination;
    uart::IDFUARTComponent* idf_uart;

    [[noreturn]] void uart_event_task();
};

}
}