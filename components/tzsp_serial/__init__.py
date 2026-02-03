import esphome.codegen as cg
import esphome.config_validation as cv

from esphome import pins
from esphome.components import tzsp, uart
from esphome.const import CONF_ID
from esphome.types import ConfigType

CODEOWNERS = ["@Omniflux"]
AUTO_LOAD = ["tzsp"]
DEPENDENCIES = ["tzsp", "uart"]

CONF_FRAME_SIZE = "frame_size"
CONF_SYMBOL_TIMEOUT = "symbol_timeout"
CONF_INVERTED = "inverted"

capture_ns = cg.esphome_ns.namespace("tzspserial")
TZSPSerial = capture_ns.class_("TZSPSerial", cg.Component, uart.UARTDevice)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(TZSPSerial),
        cv.Required(CONF_FRAME_SIZE): cv.positive_int,
        cv.Optional(CONF_SYMBOL_TIMEOUT, default=10): cv.int_range(0, 126),
        cv.Optional(CONF_INVERTED, default=False): cv.boolean
    }
).extend(cv.COMPONENT_SCHEMA).extend(uart.UART_DEVICE_SCHEMA).extend(tzsp.TZSP_SENDER_SCHEMA)

async def to_code(config: ConfigType) -> None:
    var = cg.new_Pvariable(config[CONF_ID], await cg.get_variable(config[uart.CONF_UART_ID]))
    await cg.register_component(var, config)
    await tzsp.register_tzsp_sender(var, config)
    await uart.register_uart_device(var, config)

    cg.add(var.set_frame_size(config[CONF_FRAME_SIZE]))
    cg.add(var.set_symbol_timeout(config[CONF_SYMBOL_TIMEOUT]))
    cg.add(var.set_inverted(config[CONF_INVERTED]))
