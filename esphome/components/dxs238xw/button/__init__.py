import esphome.codegen as cg
from esphome.components import button
import esphome.config_validation as cv
from esphome.const import ENTITY_CATEGORY_CONFIG

from .. import CONF_DXS238XW_ID, DXS238XW_COMPONENT_SCHEMA, SmIdEntity, dxs238xw_ns

DEPENDENCIES = ["dxs238xw"]

Dxs238xwButton = dxs238xw_ns.class_("Dxs238xwButton", button.Button)

RESET_DATA = "reset_data"

TYPES = {
    RESET_DATA: (
        button.button_schema(
            class_=Dxs238xwButton,
            icon="mdi:backup-restore",
            entity_category=ENTITY_CATEGORY_CONFIG,
        ),
        SmIdEntity.BUTTON_RESET_DATA,
    ),
}

CONFIG_SCHEMA = DXS238XW_COMPONENT_SCHEMA.extend(
    {cv.Optional(type): schema for type, (schema, _) in TYPES.items()}
)


async def to_code(config):
    paren = await cg.get_variable(config[CONF_DXS238XW_ID])

    for type, (_, number_id) in TYPES.items():
        if type in config:
            conf = config[type]
            var = await button.new_button(conf)
            cg.add(var.set_dxs238xw_parent(paren))
            cg.add(var.set_entity_id(number_id))
            cg.add(getattr(paren, f"set_{type}_button")(var))
