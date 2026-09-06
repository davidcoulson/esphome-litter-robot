import esphome.codegen as cg
from esphome.components import sensor
import esphome.config_validation as cv
from esphome.const import CONF_ID
from esphome.core import CORE

CODEOWNERS = []

CONF_CAT_TRACKER_ID = "cat_tracker_id"
CONF_WEIGHT_SENSOR = "weight_sensor"
CONF_TOLERANCE = "tolerance"

cat_tracker_ns = cg.esphome_ns.namespace("cat_tracker")
CatTrackerComponent = cat_tracker_ns.class_("CatTrackerComponent", cg.Component)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(CatTrackerComponent),
        cv.Required(CONF_WEIGHT_SENSOR): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_TOLERANCE, default=0.6): cv.float_range(min=0.0, max=50.0),
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    weight_sensor = await cg.get_variable(config[CONF_WEIGHT_SENSOR])
    cg.add(var.set_weight_sensor(weight_sensor))
    cg.add(var.set_tolerance(config[CONF_TOLERANCE]))
    cat_count = sum(
        1
        for number_conf in CORE.config.get("number", [])
        if number_conf.get("platform") == "cat_tracker"
    )
    cg.add_define("CAT_TRACKER_MAX_CATS", cat_count)
