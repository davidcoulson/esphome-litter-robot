import re

from esphome import automation
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID, CONF_WEIGHT
from esphome.core import ID
from esphome.cpp_generator import MockObj, TemplateArgsType
from esphome.types import ConfigType

CODEOWNERS = []

CONF_CAT_TRACKER_ID = "cat_tracker_id"
CONF_WEIGHT_TOLERANCE = "weight_tolerance"
CONF_ON_VISIT = "on_visit"
CONF_CAT = "cat"
CONF_CATS = "cats"
CONF_WASTE_TYPE = "waste_type"
CONF_WASTE_WEIGHT = "waste_weight"
CONF_LEGACY_OBJECT_ID = "legacy_object_id"
CONF_MIN_RSSI = "min_rssi"
CONF_GAP = "gap"
CONF_RSSI = "rssi"

cat_tracker_ns = cg.esphome_ns.namespace("cat_tracker")
CatTrackerComponent = cat_tracker_ns.class_("CatTrackerComponent", cg.Component)
CatTrackerCat = cat_tracker_ns.class_("CatTrackerCat", cg.Component)
CatTrackerRecordVisitAction = cat_tracker_ns.class_(
    "CatTrackerRecordVisitAction", automation.Action
)

CatTrackerTagSeenAction = cat_tracker_ns.class_(
    "CatTrackerTagSeenAction", automation.Action
)
CatTrackerVisitEndedAction = cat_tracker_ns.class_(
    "CatTrackerVisitEndedAction", automation.Action
)

CatTrackerCountType = cat_tracker_ns.enum("CatTrackerCountType", is_class=True)
WASTE_TYPES = {
    "URINE": CatTrackerCountType.CAT_TRACKER_COUNT_TYPE_URINE,
    "FECES": CatTrackerCountType.CAT_TRACKER_COUNT_TYPE_FECES,
}

CAT_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(CatTrackerCat),
        cv.Optional(CONF_LEGACY_OBJECT_ID): cv.string,
        # BLE tag sightings weaker than this are ignored; a visit ends after this long without one.
        cv.Optional(CONF_MIN_RSSI, default=-85): cv.int_range(min=-127, max=0),
        cv.Optional(CONF_GAP, default="60s"): cv.positive_time_period_milliseconds,
    }
)


def _validate_cat(value: object) -> ConfigType:
    config = CAT_SCHEMA({CONF_ID: value} if isinstance(value, str) else value)
    if CONF_LEGACY_OBJECT_ID not in config:
        # Default to the 2026.9.2 "Cat N Weight" entity name when the id matches it.
        if match := re.fullmatch(r"cat(\d+)", str(config[CONF_ID])):
            config[CONF_LEGACY_OBJECT_ID] = f"Cat {match.group(1)} Weight"
    return config


CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(CatTrackerComponent),
        cv.Optional(CONF_WEIGHT_TOLERANCE, default=0.6): cv.float_range(
            min=0.0, max=10.0
        ),
        cv.Optional(CONF_CATS, default=[]): cv.ensure_list(_validate_cat),
        cv.Optional(CONF_ON_VISIT): automation.validate_automation({}),
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config: ConfigType) -> None:
    var = cg.new_Pvariable(config[CONF_ID], config[CONF_WEIGHT_TOLERANCE])
    await cg.register_component(var, config)
    for cat_conf in config[CONF_CATS]:
        cat = cg.new_Pvariable(cat_conf[CONF_ID], str(cat_conf[CONF_ID]))
        await cg.register_component(cat, cat_conf)
        if (legacy_object_id := cat_conf.get(CONF_LEGACY_OBJECT_ID)) is not None:
            cg.add(cat.set_legacy_object_id(legacy_object_id))
        cg.add(cat.set_min_rssi(cat_conf[CONF_MIN_RSSI]))
        cg.add(cat.set_gap_ms(cat_conf[CONF_GAP]))
        cg.add(var.register_cat(cat))
    for conf in config.get(CONF_ON_VISIT, []):
        await automation.build_callback_automation(
            var, "add_on_visit_callback", [], conf
        )
    cg.add_define("CAT_TRACKER_MAX_CATS", len(config[CONF_CATS]))


def _validate_waste_weight(config: ConfigType) -> ConfigType:
    if CONF_WASTE_WEIGHT in config and CONF_WASTE_TYPE not in config:
        raise cv.Invalid(f"'{CONF_WASTE_WEIGHT}' requires '{CONF_WASTE_TYPE}'")
    return config


@automation.register_action(
    "cat_tracker.record_visit",
    CatTrackerRecordVisitAction,
    cv.All(
        cv.Schema(
            {
                cv.GenerateID(CONF_CAT_TRACKER_ID): cv.use_id(CatTrackerComponent),
                cv.Optional(CONF_CAT): cv.use_id(CatTrackerCat),
                cv.Optional(CONF_WEIGHT): cv.templatable(cv.float_),
                cv.Optional(CONF_WASTE_TYPE): cv.enum(WASTE_TYPES, upper=True),
                cv.Optional(CONF_WASTE_WEIGHT): cv.templatable(cv.float_),
            }
        ),
        cv.has_exactly_one_key(CONF_CAT, CONF_WEIGHT),
        _validate_waste_weight,
    ),
    synchronous=True,
)
async def cat_tracker_record_visit_to_code(
    config: ConfigType,
    action_id: ID,
    template_arg: cg.TemplateArguments,
    args: TemplateArgsType,
) -> MockObj:
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_CAT_TRACKER_ID])
    if (cat_id := config.get(CONF_CAT)) is not None:
        cat = await cg.get_variable(cat_id)
        cg.add(var.set_cat(cat))
    if (weight := config.get(CONF_WEIGHT)) is not None:
        weight_value = await cg.templatable(weight, args, cg.float_)
        cg.add(var.set_weight(weight_value))
    if (waste_type := config.get(CONF_WASTE_TYPE)) is not None:
        cg.add(var.set_waste_type(waste_type))
    if (waste_weight := config.get(CONF_WASTE_WEIGHT)) is not None:
        waste_weight_value = await cg.templatable(waste_weight, args, cg.float_)
        cg.add(var.set_waste_weight(waste_weight_value))
    return var


@automation.register_action(
    "cat_tracker.tag_seen",
    CatTrackerTagSeenAction,
    cv.Schema(
        {
            cv.Required(CONF_CAT): cv.use_id(CatTrackerCat),
            cv.Required(CONF_RSSI): cv.templatable(cv.int_),
        }
    ),
    synchronous=True,
)
async def cat_tracker_tag_seen_to_code(
    config: ConfigType,
    action_id: ID,
    template_arg: cg.TemplateArguments,
    args: TemplateArgsType,
) -> MockObj:
    var = cg.new_Pvariable(action_id, template_arg)
    cg.add(var.set_cat(await cg.get_variable(config[CONF_CAT])))
    cg.add(var.set_rssi(await cg.templatable(config[CONF_RSSI], args, cg.int_)))
    return var


@automation.register_action(
    "cat_tracker.visit_ended",
    CatTrackerVisitEndedAction,
    cv.Schema(
        {
            cv.GenerateID(CONF_CAT_TRACKER_ID): cv.use_id(CatTrackerComponent),
            cv.Optional(CONF_CAT): cv.use_id(CatTrackerCat),
        }
    ),
    synchronous=True,
)
async def cat_tracker_visit_ended_to_code(
    config: ConfigType,
    action_id: ID,
    template_arg: cg.TemplateArguments,
    args: TemplateArgsType,
) -> MockObj:
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_CAT_TRACKER_ID])
    if (cat_id := config.get(CONF_CAT)) is not None:
        cg.add(var.set_cat(await cg.get_variable(cat_id)))
    return var
