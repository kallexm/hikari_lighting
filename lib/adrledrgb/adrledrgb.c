#include "../include/adrledrgb.h"

#include <nrfx_pwm.h>
#include <hal/nrf_gpio.h>

#define PULSE_0 ( 6)
#define PULSE_1 (15)


static nrfx_pwm_config_t pwm_config_m   = {
    .output_pins  = { NRF_PWM_PIN_NOT_CONNECTED,
                      NRF_PWM_PIN_NOT_CONNECTED,
                      NRF_PWM_PIN_NOT_CONNECTED,
                      NRF_PWM_PIN_NOT_CONNECTED },
    .irq_priority = 0,
    .base_clock   = NRF_PWM_CLK_16MHz,
    .count_mode   = NRF_PWM_MODE_UP,
    .top_value    = 21,
    .load_mode    = NRF_PWM_LOAD_COMMON,
    .step_mode    = NRF_PWM_STEP_AUTO,
};

static nrfx_pwm_t        pwm_instance_m = NRFX_PWM_INSTANCE(1);

static uint32_t out_pins_m[4] = { NRF_PWM_PIN_NOT_CONNECTED,
                                  NRF_PWM_PIN_NOT_CONNECTED,
                                  NRF_PWM_PIN_NOT_CONNECTED,
                                  NRF_PWM_PIN_NOT_CONNECTED };

static nrf_pwm_sequence_t sequence_m = {
    .values = { .p_common = NULL },
    .length    = 0,
    .repeats   = 0,
    .end_delay = 1000,
};

bool pwm_initialized = false;

int adrledrgb_init(rgb_chain_t* rgb_chain)
{
    if (pwm_initialized == false) {
        nrfx_pwm_init(&pwm_instance_m, &pwm_config_m, NULL, NULL);
        pwm_initialized = true;
    }

    bool inverted = !!rgb_chain->inverted;

    nrf_gpio_pin_write(rgb_chain->data_pin_reg & 0x3F, !inverted);
    nrf_gpio_cfg_output(rgb_chain->data_pin_reg & 0x3F);

    uint16_t* data = rgb_chain->pwm_sequence;
    for (uint32_t i = 0; i < PREPAUSE_PERIODS; i++)
    {
        data[i] = (0x8000 & (inverted << 15)) | (0x7FFF & (0 << 0));
    }

    return 0;
}

/*void adrledrgb_get_leds(rgb_chain_t* rgb_chain, rgb_t* rgb_leds, uint32_t* num_leds)
{
    rgb_leds  = rgb_chain->rgb_values;
    *num_leds = rgb_chain->num_leds;
    return;
}*/

int adrledrgb_update_leds(rgb_chain_t* rgb_chain)
{
    rgb_t val;
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;

    if (!nrfx_pwm_stopped_check(&pwm_instance_m)) {
        
        return -1;
    }

    out_pins_m[0] = rgb_chain->data_pin_reg;
    nrf_pwm_pins_set(pwm_instance_m.p_reg, out_pins_m);

    uint16_t* data     = rgb_chain->pwm_sequence;
    uint32_t  numleds  = rgb_chain->num_leds;
    bool      inverted = !!rgb_chain->inverted;

    uint32_t base = PREPAUSE_PERIODS;
    for (uint32_t i = 0; i < numleds; i++)
    {
        val = rgb_chain->rgb_values[i];
        r = val.red;
        g = val.green;
        b = val.blue;
        for (uint32_t j = 0; j < 8; j++)
        {
            data[base+j+0]  = (0x8000 & (inverted << 15)) | (0x7FFF & ((0x80 & r) ? PULSE_1 : PULSE_0));
            r = r << 1;
            data[base+j+8]  = (0x8000 & (inverted << 15)) | (0x7FFF & ((0x80 & g) ? PULSE_1 : PULSE_0));
            g = g << 1;
            data[base+j+16] = (0x8000 & (inverted << 15)) | (0x7FFF & ((0x80 & b) ? PULSE_1 : PULSE_0));
            b = b << 1;

            /*
             data[base+j+0]  = (0x8000 & (1 << 15)) | (0x7FFF & ((0x80 & r) ? PULSE_1 : PULSE_0));
            r = r << 1;
            data[base+j+8]  = (0x8000 & (1 << 15)) | (0x7FFF & ((0x80 & g) ? PULSE_1 : PULSE_0));
            g = g << 1;
            data[base+j+16] = (0x8000 & (1 << 15)) | (0x7FFF & ((0x80 & b) ? PULSE_1 : PULSE_0));
            b = b << 1;
            */
        }

        base = base + 24;
    }

    sequence_m.values.p_common = data;
    sequence_m.length = (uint16_t)(PREPAUSE_PERIODS+(24*numleds));

    nrfx_pwm_simple_playback(&pwm_instance_m, &sequence_m, 1, NRFX_PWM_FLAG_STOP);

    return 0;
}