import esphome.codegen as cg
from esphome.components import number
import esphome.config_validation as cv
from esphome.const import ENTITY_CATEGORY_CONFIG, UNIT_POUND

from .. import CONF_CAT_TRACKER_ID, CatTrackerComponent, cat_tracker_ns

DEPENDENCIES = ["cat_tracker"]

CatTrackerCatWeightNumber = cat_tracker_ns.class_(
    "CatTrackerCatWeightNumber",
    number.Number,
    cg.Component,
    cg.Parented.template(CatTrackerComponent),
)

CONFIG_SCHEMA = (
    number.number_schema(
        CatTrackerCatWeightNumber,
        unit_of_measurement=UNIT_POUND,
        entity_category=ENTITY_CATEGORY_CONFIG,
        icon="mdi:weight",
    )
    .extend(
        {
            cv.GenerateID(CONF_CAT_TRACKER_ID): cv.use_id(CatTrackerComponent),
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
)


async def to_code(config):
    parent = await cg.get_variable(config[CONF_CAT_TRACKER_ID])
    var = await number.new_number(config, min_value=0, max_value=50, step=0.01)
    await cg.register_component(var, config)
    await cg.register_parented(var, parent)
