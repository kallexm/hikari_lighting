#ifndef ADRLEDRGB_H__
#define ADRLEDRGB_H__

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    uint8_t unused;
} rgb_t;

typedef struct {
    uint32_t   data_pin;
    uint32_t   data_port;
    uint32_t   data_pin_reg;

    uint32_t   num_leds;
    rgb_t*     rgb_values;
    uint16_t*  pwm_sequence;
    bool       inverted;
} rgb_chain_t;

#define PREPAUSE_PERIODS (2*24)

#define RGB_CHAIN_DEF(name, numleds, pin, port, pin_inverted) \
    rgb_t (name ## _rgb_values)[numleds] = {0}; \
    uint16_t (name ## _pwm_sequence)[PREPAUSE_PERIODS+(24*numleds)] = {0}; \
    rgb_chain_t name = { \
        .data_pin  = pin, \
        .data_port = port, \
        .data_pin_reg = ( \
              (PWM_PSEL_OUT_CONNECT_Msk & (PWM_PSEL_OUT_CONNECT_Connected << PWM_PSEL_OUT_CONNECT_Pos)) \
            | (PWM_PSEL_OUT_PORT_Msk    & (                          port << PWM_PSEL_OUT_PORT_Pos)) \
            | (PWM_PSEL_OUT_PIN_Msk     & (                           pin << PWM_PSEL_OUT_PIN_Pos)) \
        ), \
        .num_leds = numleds, \
        .rgb_values = (name ## _rgb_values), \
        .pwm_sequence = (name ## _pwm_sequence), \
        .inverted = !!pin_inverted, \
    }

int adrledrgb_init(rgb_chain_t* rgb_chain);

//void adrledrgb_get_leds(rgb_chain_t* rgb_chain, rgb_t* rgb_leds, uint32_t* num_leds);

int adrledrgb_update_leds(rgb_chain_t* rgb_chain);

#endif /* ADRLEDRGB_H__ */