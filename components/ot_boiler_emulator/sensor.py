import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import (
    DEVICE_CLASS_TEMPERATURE,
    STATE_CLASS_MEASUREMENT,
    UNIT_CELSIUS,
)

from . import OTBoilerEmulator

CONF_OT_BOILER_EMULATOR_ID = "ot_boiler_emulator_id"
CONF_REQUESTED_BOILER_SETPOINT = "requested_boiler_setpoint"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_OT_BOILER_EMULATOR_ID): cv.use_id(OTBoilerEmulator),
        cv.Optional(CONF_REQUESTED_BOILER_SETPOINT): sensor.sensor_schema(
            unit_of_measurement=UNIT_CELSIUS,
            device_class=DEVICE_CLASS_TEMPERATURE,
            state_class=STATE_CLASS_MEASUREMENT,
            accuracy_decimals=1,
        ),
    }
)


async def to_code(config):
    parent = await cg.get_variable(config[CONF_OT_BOILER_EMULATOR_ID])

    if CONF_REQUESTED_BOILER_SETPOINT in config:
        sens = await sensor.new_sensor(config[CONF_REQUESTED_BOILER_SETPOINT])
        cg.add(parent.set_boiler_setpoint_sensor(sens))
