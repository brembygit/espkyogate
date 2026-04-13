"""Bentel KYO switch platform — polling control, serial trace, log trace."""

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import switch
from esphome.const import CONF_ID, CONF_TYPE, ENTITY_CATEGORY_CONFIG

from . import bentel_kyo_ns, BentelKyo, CONF_BENTEL_KYO_ID

BentelKyoPollingSwitch = bentel_kyo_ns.class_(
    "BentelKyoPollingSwitch", switch.Switch, cg.Component
)

BentelKyoSerialTraceSwitch = bentel_kyo_ns.class_(
    "BentelKyoSerialTraceSwitch", switch.Switch, cg.Component
)

BentelKyoLogTraceSwitch = bentel_kyo_ns.class_(
    "BentelKyoLogTraceSwitch", switch.Switch, cg.Component
)

SWITCH_TYPES = {
    "polling": (BentelKyoPollingSwitch, "mdi:connection"),
    "serial_trace": (BentelKyoSerialTraceSwitch, "mdi:console"),
    "log_trace": (BentelKyoLogTraceSwitch, "mdi:text-box-search"),
}


def _switch_schema(type_str):
    cls, icon = SWITCH_TYPES[type_str]
    return (
        switch.switch_schema(
            cls,
            icon=icon,
            entity_category=ENTITY_CATEGORY_CONFIG,
        )
        .extend(
            {
                cv.GenerateID(CONF_BENTEL_KYO_ID): cv.use_id(BentelKyo),
            }
        )
        .extend(cv.COMPONENT_SCHEMA)
    )


CONFIG_SCHEMA = cv.typed_schema(
    {t: _switch_schema(t) for t in SWITCH_TYPES},
    key=CONF_TYPE,
)


async def to_code(config):
    var = await switch.new_switch(config)
    await cg.register_component(var, config)
    parent = await cg.get_variable(config[CONF_BENTEL_KYO_ID])
    cg.add(var.set_parent(parent))
