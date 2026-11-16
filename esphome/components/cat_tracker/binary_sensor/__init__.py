import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ICON
from esphome.types import ConfigType

from esphome.components import binary_sensor

from .. import CONF_CAT, CatTrackerCat, cat_tracker_ns

DEPENDENCIES = ["cat_tracker"]

CatTrackerAlert = cat_tracker_ns.class_(
    "CatTrackerAlert", binary_sensor.BinarySensor, cg.Component
)
CatTrackerAlertType = cat_tracker_ns.enum("CatTrackerAlertType", is_class=True)

CONF_TYPE = "type"
CONF_THRESHOLD = "threshold"

# type -> (enum, default threshold): weight and visit alerts are percent change, no_visit is hours.
ALERT_TYPES = {
    "weight_changed": (CatTrackerAlertType.CAT_TRACKER_ALERT_WEIGHT_CHANGED, 5.0),
    "visits_changed": (CatTrackerAlertType.CAT_TRACKER_ALERT_VISITS_CHANGED, 40.0),
    "no_visit": (CatTrackerAlertType.CAT_TRACKER_ALERT_NO_VISIT, 24.0),
}

ICONS = {
    "weight_changed": "mdi:scale-unbalanced",
    "visits_changed": "mdi:chart-line-variant",
    "no_visit": "mdi:clock-alert-outline",
}



def _default_icon(config: ConfigType) -> ConfigType:
    config.setdefault(CONF_ICON, ICONS[config[CONF_TYPE]])
    return config


CONFIG_SCHEMA = cv.All(
    binary_sensor.binary_sensor_schema(CatTrackerAlert)
    .extend(
        {
            cv.Required(CONF_CAT): cv.use_id(CatTrackerCat),
            cv.Required(CONF_TYPE): cv.enum(ALERT_TYPES, lower=True),
            cv.Optional(CONF_THRESHOLD): cv.positive_float,
        }
    )
    .extend(cv.COMPONENT_SCHEMA),
    _default_icon,
)


async def to_code(config: ConfigType) -> None:
    alert_type, default = ALERT_TYPES[config[CONF_TYPE]]
    cat = await cg.get_variable(config[CONF_CAT])
    var = await binary_sensor.new_binary_sensor(config, cat, alert_type)
    await cg.register_component(var, config)
    cg.add(var.set_threshold(config.get(CONF_THRESHOLD, default)))
