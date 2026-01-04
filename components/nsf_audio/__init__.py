import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID
from esphome import pins

CODEOWNERS = ["@esphome"]
DEPENDENCIES = []

nsf_audio_ns = cg.esphome_ns.namespace("nsf_audio")
NSFAudioComponent = nsf_audio_ns.class_("NSFAudioComponent", cg.Component)

CONF_BCLK_PIN = "bclk_pin"
CONF_LRCLK_PIN = "lrclk_pin"
CONF_DOUT_PIN = "dout_pin"

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(NSFAudioComponent),
    cv.Required(CONF_BCLK_PIN): pins.gpio_output_pin_schema,
    cv.Required(CONF_LRCLK_PIN): pins.gpio_output_pin_schema,
    cv.Required(CONF_DOUT_PIN): pins.gpio_output_pin_schema,
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    
    bclk = await cg.gpio_pin_expression(config[CONF_BCLK_PIN])
    cg.add(var.set_bclk_pin(bclk))
    
    lrclk = await cg.gpio_pin_expression(config[CONF_LRCLK_PIN])
    cg.add(var.set_lrclk_pin(lrclk))
    
    dout = await cg.gpio_pin_expression(config[CONF_DOUT_PIN])
    cg.add(var.set_dout_pin(dout))
