import esphome.codegen as cg
from esphome.components import sensor
import esphome.config_validation as cv
from esphome.const import STATE_CLASS_TOTAL_INCREASING

from .. import CONF_CAT_TRACKER_ID, CatTrackerComponent, cat_tracker_ns
from ..number import CatTrackerCatWeightNumber

DEPENDENCIES = ["cat_tracker"]

CONF_CAT_WEIGHT = "cat_weight"

CatVisitCountSensor = cat_tracker_ns.class_(
    "CatVisitCountSensor",
    sensor.Sensor,
    cg.Component,
    cg.Parented.template(CatTrackerComponent),
)

CONFIG_SCHEMA = (
    sensor.sensor_schema(
        CatVisitCountSensor,
        accuracy_decimals=0,
        state_class=STATE_CLASS_TOTAL_INCREASING,
        icon="mdi:cat",
    )
    .extend(
        {
            cv.GenerateID(CONF_CAT_TRACKER_ID): cv.use_id(CatTrackerComponent),
            cv.GenerateID(CONF_CAT_WEIGHT): cv.use_id(CatTrackerCatWeightNumber),
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
)


async def to_code(config):
    parent = await cg.get_variable(config[CONF_CAT_TRACKER_ID])
    var = await sensor.new_sensor(config)
    await cg.register_component(var, config)
    await cg.register_parented(var, parent)
    cat = await cg.get_variable(config[CONF_CAT_WEIGHT])
    cg.add(var.set_cat_number(cat))
