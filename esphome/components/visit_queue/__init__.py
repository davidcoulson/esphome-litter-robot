from esphome import automation
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID

CODEOWNERS = []

CONF_CATS = "cats"
CONF_MAX_PENDING = "max_pending"
CONF_ON_RECORD = "on_record"
MAX_CATS = 6

visit_queue_ns = cg.esphome_ns.namespace("visit_queue")
VisitQueue = visit_queue_ns.class_("VisitQueue", cg.Component)
Outgoing = visit_queue_ns.struct("Outgoing")


def _cats(value):
    """Cat names by slot, as a list or as one comma-separated string (so a substitution can carry it)."""
    if isinstance(value, str):
        value = value.split(",")
    if not isinstance(value, list):
        raise cv.Invalid("cats must be a list or a comma-separated string")
    names = [str(v).strip() for v in value]
    if not 1 <= len(names) <= MAX_CATS or any(not n or "," in n or "=" in n for n in names):
        raise cv.Invalid(f"cats needs 1 to {MAX_CATS} names without ',' or '='")
    return names


CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(VisitQueue),
        cv.Required(CONF_CATS): _cats,
        # Longer than any WasteSense timeout: after this a pending visit is sent without a verdict.
        cv.Optional(CONF_MAX_PENDING, default="45min"): cv.positive_time_period_seconds,
        cv.Optional(CONF_ON_RECORD): automation.validate_automation({}),
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    for name in config[CONF_CATS]:
        cg.add(var.add_cat(name))
    cg.add(var.set_max_pending_s(config[CONF_MAX_PENDING]))
    for conf in config.get(CONF_ON_RECORD, []):
        await automation.build_callback_automation(
            var, "add_on_record_callback", [(Outgoing.operator("const").operator("ref"), "r")], conf
        )
