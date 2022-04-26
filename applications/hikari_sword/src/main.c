#include <zephyr.h>
#include <drivers/gpio.h>

#include <adrledrgb.h>

/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS   500

/* The devicetree node identifier for the "led0" alias. */
#define LED0_NODE DT_ALIAS(led0)

#define LEDS_CHAIN_2_NUM  14
#define LEDS_CHAIN_2_PIN  12
#define LEDS_CHAIN_2_PORT  1
RGB_CHAIN_DEF(leds_chain_2, LEDS_CHAIN_2_NUM, LEDS_CHAIN_2_PIN, LEDS_CHAIN_2_PORT, true);
rgb_t* lights_2;
uint32_t num_lights_2 = 0;

#define LEDS_CHAIN_1_NUM   3
#define LEDS_CHAIN_1_PIN  11
#define LEDS_CHAIN_1_PORT  1
RGB_CHAIN_DEF(leds_chain_1, LEDS_CHAIN_1_NUM, LEDS_CHAIN_1_PIN, LEDS_CHAIN_1_PORT, true);
rgb_t* lights_1;
uint32_t num_lights_1 = 0;

/*
 * A build error on this line means your board is unsupported.
 * See the sample documentation for information on how to fix this.
 */
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

void main(void)
{
	int ret;

	printk("Startup\n");

	if (!device_is_ready(led.port)) {
		return;
	}

	ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		return;
	}

	/* ===== */

	ret = adrledrgb_init(&leds_chain_2);
	if (ret < 0) {
		return;
	}

	ret = adrledrgb_init(&leds_chain_1);
	if (ret < 0) {
		return;
	}

	num_lights_1 = leds_chain_1.num_leds;
	lights_1     = leds_chain_1.rgb_values;

	for (uint32_t i = 0; i < num_lights_1; i++) {
    	lights_1[i].red   = 0;
    	lights_1[i].green = 0;
    	lights_1[i].blue  = 0;
  	}

  	num_lights_2 = leds_chain_2.num_leds;
	lights_2     = leds_chain_2.rgb_values;

	for (uint32_t i = 0; i < num_lights_2; i++) {
    	lights_2[i].red   = 0;
    	lights_2[i].green = 0;
    	lights_2[i].blue  = 0;
  	}

  	static uint8_t h = 0;
	/* ===== */

	while (1) {
		ret = gpio_pin_toggle_dt(&led);
		if (ret < 0) {
			return;
		}

		k_msleep(SLEEP_TIME_MS);

		do {
			ret = adrledrgb_update_leds(&leds_chain_2);
			k_msleep(1);
		} while (ret != 0);

		do {
			ret = adrledrgb_update_leds(&leds_chain_1);
			k_msleep(1);
		} while (ret != 0);
		

		if (h == 0) {
			h = 1;
			lights_1[0].red   = 15;
			lights_1[1].green = 15;
			lights_1[2].blue  = 15;

			lights_2[0].green = 1;
			lights_2[1].green = 2;
			lights_2[2].green = 4;
			lights_2[3].green = 8;
			lights_2[4].green = 16;
			lights_2[5].green = 32;
			lights_2[6].blue = 1;
			lights_2[7].blue = 1;
			lights_2[8].blue = 1;
			lights_2[9].blue = 1;
			lights_2[10].blue = 1;
			lights_2[11].blue = 1;
			lights_2[12].blue = 1;
			lights_2[13].blue = 1;
			lights_2[6].green = 1;
			lights_2[7].green = 1;
			lights_2[8].green = 1;
			lights_2[9].green = 1;
			lights_2[10].green = 1;
			lights_2[11].green = 1;
			lights_2[12].green = 1;
			lights_2[13].green = 1;
		} else {
			h = 0;
			lights_1[0].red   = 31;
			lights_1[1].green = 31;
			lights_1[2].blue  = 31;

			lights_2[0].green = 4;
			lights_2[1].green = 8;
			lights_2[2].green = 16;
			lights_2[3].green = 32;
			lights_2[4].green = 64;
			lights_2[5].green = 128;
			lights_2[6].blue = 4;
			lights_2[7].blue = 4;
			lights_2[8].blue = 4;
			lights_2[9].blue = 4;
			lights_2[10].blue = 4;
			lights_2[11].blue = 4;
			lights_2[12].blue = 4;
			lights_2[13].blue = 4;
			lights_2[6].green = 4;
			lights_2[7].green = 4;
			lights_2[8].green = 4;
			lights_2[9].green = 4;
			lights_2[10].green = 4;
			lights_2[11].green = 4;
			lights_2[12].green = 4;
			lights_2[13].green = 4;
		}
	}
}
