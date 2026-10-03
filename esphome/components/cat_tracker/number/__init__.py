import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import ENTITY_CATEGORY_CONFIG, UNIT_POUND
from esphome.types import ConfigType

from esphome.components import number

from .. import CONF_CAT, CatTrackerCat, cat_tracker_ns

DEPENDENCIES = ["cat_tracker"]

CatTrackerCatWeightNumber = cat_tracker_ns.class_(
    "CatTrackerCatWeightNumber", number.Number, cg.Component
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
            cv.Required(CONF_CAT): cv.use_id(CatTrackerCat),
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
)


async def to_code(config: ConfigType) -> None:
    cat = await cg.get_variable(config[CONF_CAT])
    var = await number.new_number(config, cat, min_value=0, max_value=50, step=0.01)
    await cg.register_component(var, config)
