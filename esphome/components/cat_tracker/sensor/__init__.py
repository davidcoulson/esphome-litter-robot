import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import (
    DEVICE_CLASS_DURATION,
    DEVICE_CLASS_WEIGHT,
    STATE_CLASS_MEASUREMENT,
    STATE_CLASS_TOTAL_INCREASING,
    UNIT_POUND,
    UNIT_SECOND,
)
from esphome.types import ConfigType

from esphome.components import sensor

from .. import CONF_CAT, CatTrackerCat, cat_tracker_ns

DEPENDENCIES = ["cat_tracker"]

UNIT_OUNCE = "oz"

CatTrackerCountSensor = cat_tracker_ns.class_(
    "CatTrackerCountSensor", sensor.Sensor, cg.Component
)

CatTrackerSensorType = cat_tracker_ns.enum("CatTrackerSensorType", is_class=True)


def _sub_sensor(**traits) -> cv.Schema:
    return sensor.sensor_schema(CatTrackerCountSensor, **traits)


# Sub-key -> (schema, sensor type). The sub-key IS the type.
SUB_SENSORS = {
    "weight": (
        _sub_sensor(
            unit_of_measurement=UNIT_POUND,
            accuracy_decimals=2,
            device_class=DEVICE_CLASS_WEIGHT,
            state_class=STATE_CLASS_MEASUREMENT,
            icon="mdi:scale-bathroom",
        ),
        CatTrackerSensorType.CAT_TRACKER_SENSOR_TYPE_WEIGHT,
    ),
    "visits_count": (
        _sub_sensor(
            accuracy_decimals=0,
            state_class=STATE_CLASS_TOTAL_INCREASING,
            icon="mdi:cat",
        ),
        CatTrackerSensorType.CAT_TRACKER_SENSOR_TYPE_VISITS_COUNT,
    ),
    "feces_count": (
        _sub_sensor(
            accuracy_decimals=0,
            state_class=STATE_CLASS_TOTAL_INCREASING,
            icon="mdi:emoticon-poop",
        ),
        CatTrackerSensorType.CAT_TRACKER_SENSOR_TYPE_FECES_COUNT,
    ),
    "urine_count": (
        _sub_sensor(
            accuracy_decimals=0,
            state_class=STATE_CLASS_TOTAL_INCREASING,
            icon="mdi:water",
        ),
        CatTrackerSensorType.CAT_TRACKER_SENSOR_TYPE_URINE_COUNT,
    ),
    "feces_weight": (
        _sub_sensor(
            unit_of_measurement=UNIT_OUNCE,
            accuracy_decimals=1,
            state_class=STATE_CLASS_MEASUREMENT,
            icon="mdi:scale",
        ),
        CatTrackerSensorType.CAT_TRACKER_SENSOR_TYPE_FECES_WEIGHT,
    ),
    "urine_weight": (
        _sub_sensor(
            unit_of_measurement=UNIT_OUNCE,
            accuracy_decimals=1,
            state_class=STATE_CLASS_MEASUREMENT,
            icon="mdi:scale",
        ),
        CatTrackerSensorType.CAT_TRACKER_SENSOR_TYPE_URINE_WEIGHT,
    ),
    "visits_per_day_7d": (
        _sub_sensor(
            accuracy_decimals=1,
            state_class=STATE_CLASS_MEASUREMENT,
            icon="mdi:chart-line",
        ),
        CatTrackerSensorType.CAT_TRACKER_SENSOR_TYPE_VISITS_PER_DAY_7D,
    ),
    "avg_weight_7d": (
        _sub_sensor(
            unit_of_measurement=UNIT_POUND,
            accuracy_decimals=2,
            device_class=DEVICE_CLASS_WEIGHT,
            state_class=STATE_CLASS_MEASUREMENT,
            icon="mdi:scale-bathroom",
        ),
        CatTrackerSensorType.CAT_TRACKER_SENSOR_TYPE_AVG_WEIGHT_7D,
    ),
    "last_visit_duration": (
        _sub_sensor(
            unit_of_measurement=UNIT_SECOND,
            accuracy_decimals=0,
            device_class=DEVICE_CLASS_DURATION,
            state_class=STATE_CLASS_MEASUREMENT,
            icon="mdi:timer-outline",
        ),
        CatTrackerSensorType.CAT_TRACKER_SENSOR_TYPE_LAST_VISIT_DURATION,
    ),
    "avg_visit_duration_7d": (
        _sub_sensor(
            unit_of_measurement=UNIT_SECOND,
            accuracy_decimals=0,
            device_class=DEVICE_CLASS_DURATION,
            state_class=STATE_CLASS_MEASUREMENT,
            icon="mdi:timer-sand-complete",
        ),
        CatTrackerSensorType.CAT_TRACKER_SENSOR_TYPE_AVG_VISIT_DURATION_7D,
    ),
    "hours_since_visit": (
        _sub_sensor(
            unit_of_measurement="h",
            accuracy_decimals=1,
            state_class=STATE_CLASS_MEASUREMENT,
            icon="mdi:timer-sand",
        ),
        CatTrackerSensorType.CAT_TRACKER_SENSOR_TYPE_HOURS_SINCE_VISIT,
    ),
}

CONFIG_SCHEMA = cv.Schema(
    {
        cv.Required(CONF_CAT): cv.use_id(CatTrackerCat),
        **{cv.Optional(key): schema for key, (schema, _) in SUB_SENSORS.items()},
    }
)


async def to_code(config: ConfigType) -> None:
    cat = await cg.get_variable(config[CONF_CAT])
    for key, (_, sensor_type) in SUB_SENSORS.items():
        if (conf := config.get(key)) is not None:
            var = await sensor.new_sensor(conf, cat, sensor_type)
            await cg.register_component(var, conf)
