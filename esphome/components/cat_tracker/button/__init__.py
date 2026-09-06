import esphome.codegen as cg
from esphome.components import button
import esphome.config_validation as cv
from esphome.const import ENTITY_CATEGORY_CONFIG

from .. import CONF_CAT_TRACKER_ID, CatTrackerComponent, cat_tracker_ns

DEPENDENCIES = ["cat_tracker"]

CatTrackerResetButton = cat_tracker_ns.class_(
    "CatTrackerResetButton",
    button.Button,
    cg.Component,
    cg.Parented.template(CatTrackerComponent),
)

CONFIG_SCHEMA = (
    button.button_schema(
        CatTrackerResetButton,
        icon="mdi:eye-refresh",
        entity_category=ENTITY_CATEGORY_CONFIG,
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
    var = await button.new_button(config)
    await cg.register_component(var, config)
    await cg.register_parented(var, parent)
