import esphome.codegen as cg
from esphome.components import number
from esphome.components.number import NUMBER_MODES
import esphome.config_validation as cv
from esphome.const import (
    CONF_MODE,
    ENTITY_CATEGORY_CONFIG,
    UNIT_AMPERE,
    UNIT_KILOWATT_HOURS,
    UNIT_MINUTE,
    UNIT_VOLT,
)

from .. import (
    CONF_DXS238XW_ID,
    DXS238XW_COMPONENT_SCHEMA,
    SmIdEntity,
    SmLimitValue,
    dxs238xw_ns,
)

DEPENDENCIES = ["dxs238xw"]

Dxs238xwNumber = dxs238xw_ns.class_("Dxs238xwNumber", number.Number)

MAX_CURRENT_LIMIT = "max_current_limit"
MAX_VOLTAGE_LIMIT = "max_voltage_limit"
MIN_VOLTAGE_LIMIT = "min_voltage_limit"
ENERGY_PURCHASE_VALUE = "energy_purchase_value"
ENERGY_PURCHASE_ALARM = "energy_purchase_alarm"
DELAY_VALUE_SET = "delay_value_set"
STARTING_KWH = "starting_kWh"
PRICE_KWH = "price_kWh"

UNIT_CURRENCY_DOLLAR = "$"

TYPES = {
    MAX_CURRENT_LIMIT: (
        number.number_schema(
            class_=Dxs238xwNumber,
            icon="mdi:current-ac",
            entity_category=ENTITY_CATEGORY_CONFIG,
            unit_of_measurement=UNIT_AMPERE,
        ).extend(
            {
                cv.Optional(CONF_MODE, default="BOX"): cv.enum(
                    NUMBER_MODES, upper=True
                ),
            }
        ),
        SmLimitValue.MIN_CURRENT,
        SmLimitValue.MAX_CURRENT,
        SmLimitValue.STEP_CURRENT,
        SmIdEntity.NUMBER_MAX_CURRENT_LIMIT,
    ),
    MAX_VOLTAGE_LIMIT: (
        number.number_schema(
            class_=Dxs238xwNumber,
            icon="mdi:sine-wave",
            entity_category=ENTITY_CATEGORY_CONFIG,
            unit_of_measurement=UNIT_VOLT,
        ).extend(
            {
                cv.Optional(CONF_MODE, default="BOX"): cv.enum(
                    NUMBER_MODES, upper=True
                ),
            }
        ),
        SmLimitValue.MIN_VOLTAGE,
        SmLimitValue.MAX_VOLTAGE,
        SmLimitValue.STEP_VOLTAGE,
        SmIdEntity.NUMBER_MAX_VOLTAGE_LIMIT,
    ),
    MIN_VOLTAGE_LIMIT: (
        number.number_schema(
            class_=Dxs238xwNumber,
            icon="mdi:sine-wave",
            entity_category=ENTITY_CATEGORY_CONFIG,
            unit_of_measurement=UNIT_VOLT,
        ).extend(
            {
                cv.Optional(CONF_MODE, default="BOX"): cv.enum(
                    NUMBER_MODES, upper=True
                ),
            }
        ),
        SmLimitValue.MIN_VOLTAGE,
        SmLimitValue.MAX_VOLTAGE,
        SmLimitValue.STEP_VOLTAGE,
        SmIdEntity.NUMBER_MIN_VOLTAGE_LIMIT,
    ),
    ENERGY_PURCHASE_VALUE: (
        number.number_schema(
            class_=Dxs238xwNumber,
            icon="mdi:lightning-bolt",
            entity_category=ENTITY_CATEGORY_CONFIG,
            unit_of_measurement=UNIT_KILOWATT_HOURS,
        ).extend(
            {
                cv.Optional(CONF_MODE, default="BOX"): cv.enum(
                    NUMBER_MODES, upper=True
                ),
            }
        ),
        SmLimitValue.MIN_ENERGY_PURCHASE_VALUE,
        SmLimitValue.MAX_ENERGY_PURCHASE_VALUE,
        SmLimitValue.STEP_ENERGY_PURCHASE_VALUE,
        SmIdEntity.NUMBER_ENERGY_PURCHASE_VALUE,
    ),
    ENERGY_PURCHASE_ALARM: (
        number.number_schema(
            class_=Dxs238xwNumber,
            icon="mdi:lightning-bolt",
            entity_category=ENTITY_CATEGORY_CONFIG,
            unit_of_measurement=UNIT_KILOWATT_HOURS,
        ).extend(
            {
                cv.Optional(CONF_MODE, default="BOX"): cv.enum(
                    NUMBER_MODES, upper=True
                ),
            }
        ),
        SmLimitValue.MIN_ENERGY_PURCHASE_ALARM,
        SmLimitValue.MAX_ENERGY_PURCHASE_ALARM,
        SmLimitValue.STEP_ENERGY_PURCHASE_ALARM,
        SmIdEntity.NUMBER_ENERGY_PURCHASE_ALARM,
    ),
    DELAY_VALUE_SET: (
        number.number_schema(
            class_=Dxs238xwNumber,
            icon="mdi:timer-cog-outline",
            entity_category=ENTITY_CATEGORY_CONFIG,
            unit_of_measurement=UNIT_MINUTE,
        ).extend(
            {
                cv.Optional(CONF_MODE, default="BOX"): cv.enum(
                    NUMBER_MODES, upper=True
                ),
            }
        ),
        SmLimitValue.MIN_DELAY_SET,
        SmLimitValue.MAX_DELAY_SET,
        SmLimitValue.STEP_DELAY_SET,
        SmIdEntity.NUMBER_DELAY_VALUE_SET,
    ),
    STARTING_KWH: (
        number.number_schema(
            class_=Dxs238xwNumber,
            icon="mdi:home-lightning-bolt-outline",
            entity_category=ENTITY_CATEGORY_CONFIG,
            unit_of_measurement=UNIT_KILOWATT_HOURS,
        ).extend(
            {
                cv.Optional(CONF_MODE, default="BOX"): cv.enum(
                    NUMBER_MODES, upper=True
                ),
            }
        ),
        SmLimitValue.MIN_STARTING_KWH + 0.0,
        SmLimitValue.MAX_STARTING_KWH + 0.9,
        SmLimitValue.STEP_STARTING_KWH + 0.1,
        SmIdEntity.NUMBER_STARTING_KWH,
    ),
    PRICE_KWH: (
        number.number_schema(
            class_=Dxs238xwNumber,
            icon="mdi:cash",
            entity_category=ENTITY_CATEGORY_CONFIG,
            unit_of_measurement=UNIT_CURRENCY_DOLLAR,
        ).extend(
            {
                cv.Optional(CONF_MODE, default="BOX"): cv.enum(
                    NUMBER_MODES, upper=True
                ),
            }
        ),
        SmLimitValue.MIN_PRICE_KWH + 0.0,
        SmLimitValue.MAX_PRICE_KWH + 0.9,
        SmLimitValue.STEP_PRICE_KWH + 0.1,
        SmIdEntity.NUMBER_PRICE_KWH,
    ),
}

CONFIG_SCHEMA = DXS238XW_COMPONENT_SCHEMA.extend(
    {cv.Optional(type): schema for type, (schema, _, _, _, _) in TYPES.items()}
)


async def to_code(config):
    paren = await cg.get_variable(config[CONF_DXS238XW_ID])

    for type, (_, min, max, step_value, number_id) in TYPES.items():
        if type in config:
            conf = config[type]
            var = await number.new_number(
                conf, min_value=min, max_value=max, step=step_value
            )
            cg.add(var.set_dxs238xw_parent(paren))
            cg.add(var.set_entity_id(number_id))
            cg.add(getattr(paren, f"set_{type}_number")(var))
