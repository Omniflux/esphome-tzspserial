import esphome.codegen as cg
import esphome.config_validation as cv

from esphome import pins
from esphome.components import uart
from esphome.const import CONF_ID

DEPENDENCIES = ['uart']

CONF_TZSP = "tzsp"
CONF_TZSP_IP = "ip"
CONF_TZSP_PORT = "port"
CONF_TZSP_PROTOCOL = "protocol"
CONF_FRAME_SIZE = "frame_size"
CONF_INVERTED = "inverted"

capture_ns = cg.esphome_ns.namespace("tzspserial")
TZSPSerial = capture_ns.class_("TZSPSerial", cg.Component, uart.UARTDevice)

TZSP_SCHEMA = cv.Schema(
    {
        cv.Required(CONF_TZSP_IP): cv.ipv4,
        cv.Optional(CONF_TZSP_PORT, default=0x9090): cv.port,
        cv.Optional(CONF_TZSP_PROTOCOL, default=0): cv.int_range(0, 65535),
    }
)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(TZSPSerial),
        cv.Required(CONF_FRAME_SIZE): cv.positive_int,
        cv.Optional(CONF_INVERTED, default=False): cv.boolean,
        cv.Required(CONF_TZSP): TZSP_SCHEMA
    }
).extend(cv.COMPONENT_SCHEMA).extend(uart.UART_DEVICE_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID], await cg.get_variable(config[uart.CONF_UART_ID]))
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)

    cg.add(var.set_frame_size(config[CONF_FRAME_SIZE]))
    cg.add(var.set_tzsp_ip(config[CONF_TZSP][CONF_TZSP_IP].args))
    cg.add(var.set_inverted(config[CONF_INVERTED]))
    cg.add(var.set_tzsp_port(config[CONF_TZSP][CONF_TZSP_PORT]))
    cg.add(var.set_tzsp_protocol(config[CONF_TZSP][CONF_TZSP_PROTOCOL]))