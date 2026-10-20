import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor

from . import OTBoilerEmulator

CONF_OT_BOILER_EMULATOR_ID = "ot_boiler_emulator_id"
CONF_REQUESTED_CH_ENABLE = "requested_ch_enable"
CONF_REQUESTED_DHW_ENABLE = "requested_dhw_enable"
CONF_COMMUNICATION_ACTIVE = "communication_active"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_OT_BOILER_EMULATOR_ID): cv.use_id(OTBoilerEmulator),
        cv.Optional(CONF_REQUESTED_CH_ENABLE): binary_sensor.binary_sensor_schema(),
        cv.Optional(CONF_REQUESTED_DHW_ENABLE): binary_sensor.binary_sensor_schema(),
        cv.Optional(CONF_COMMUNICATION_ACTIVE): binary_sensor.binary_sensor_schema(),
    }
)


async def to_code(config):
    parent = await cg.get_variable(config[CONF_OT_BOILER_EMULATOR_ID])

    if CONF_REQUESTED_CH_ENABLE in config:
        sens = await binary_sensor.new_binary_sensor(config[CONF_REQUESTED_CH_ENABLE])
        cg.add(parent.set_ch_enable_sensor(sens))

    if CONF_REQUESTED_DHW_ENABLE in config:
        sens = await binary_sensor.new_binary_sensor(config[CONF_REQUESTED_DHW_ENABLE])
        cg.add(parent.set_dhw_enable_sensor(sens))

    if CONF_COMMUNICATION_ACTIVE in config:
        sens = await binary_sensor.new_binary_sensor(config[CONF_COMMUNICATION_ACTIVE])
        cg.add(parent.set_comms_active_sensor(sens))
