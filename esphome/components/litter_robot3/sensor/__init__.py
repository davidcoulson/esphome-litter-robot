import esphome.codegen as cg
from esphome.components import sensor
import esphome.config_validation as cv
from esphome.const import (
    ENTITY_CATEGORY_DIAGNOSTIC,
    STATE_CLASS_MEASUREMENT,
    STATE_CLASS_TOTAL_INCREASING,
)

from .. import (
    CONF_LITTER_ROBOT3_ID,
    LITTER_ROBOT3_DEVICE_SCHEMA,
    LitterRobot3Component,
    litter_robot3_ns,
)

DEPENDENCIES = ["litter_robot3"]

LitterRobot3CleanCycleCountSensor = litter_robot3_ns.class_(
    "LitterRobot3CleanCycleCountSensor",
    sensor.Sensor,
    cg.Component,
    cg.Parented.template(LitterRobot3Component),
)
LitterRobot3CatSensor = litter_robot3_ns.class_(
    "LitterRobot3CatSensor",
    sensor.Sensor,
    cg.Component,
    cg.Parented.template(LitterRobot3Component),
)

CONFIG_SCHEMA = cv.typed_schema(
    {
        "clean_cycle_count": (
            sensor.sensor_schema(
                LitterRobot3CleanCycleCountSensor,
                accuracy_decimals=0,
                state_class=STATE_CLASS_TOTAL_INCREASING,
                icon="mdi:counter",
            )
            .extend(LITTER_ROBOT3_DEVICE_SCHEMA)
            .extend(cv.COMPONENT_SCHEMA)
        ),
        "cat_sensor": (
            sensor.sensor_schema(
                LitterRobot3CatSensor,
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
                icon="mdi:scale",
            )
            .extend(LITTER_ROBOT3_DEVICE_SCHEMA)
            .extend(cv.COMPONENT_SCHEMA)
        ),
    }
)


async def to_code(config):
    parent = await cg.get_variable(config[CONF_LITTER_ROBOT3_ID])
    var = await sensor.new_sensor(config)
    await cg.register_component(var, config)
    await cg.register_parented(var, parent)
