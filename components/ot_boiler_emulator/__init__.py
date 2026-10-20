import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import pins
from esphome.const import CONF_ID

CODEOWNERS = ["@dave"]

ot_boiler_emulator_ns = cg.esphome_ns.namespace("ot_boiler_emulator")
OTBoilerEmulator = ot_boiler_emulator_ns.class_("OTBoilerEmulator", cg.Component)

CONF_IN_PIN = "in_pin"
CONF_OUT_PIN = "out_pin"
CONF_COMMS_TIMEOUT = "comms_timeout"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(OTBoilerEmulator),
        cv.Required(CONF_IN_PIN): pins.internal_gpio_input_pin_number,
        cv.Required(CONF_OUT_PIN): pins.internal_gpio_output_pin_number,
        # Seconds without a request from the thermostat before
        # communication_active is published as off.
        cv.Optional(CONF_COMMS_TIMEOUT, default=60): cv.positive_int,
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    cg.add(var.set_pins(config[CONF_IN_PIN], config[CONF_OUT_PIN]))
    cg.add(var.set_comms_timeout(config[CONF_COMMS_TIMEOUT]))
