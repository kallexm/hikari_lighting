#include <zephyr/zephyr.h>
#include <sys/printk.h>
#include <usb/usb_device.h>
#include <drivers/uart.h>

#include "hikari_light_ble.h"
#include "light_resource.h"

#include <drivers/gpio.h>

#define SLEEP_TIME_MS   500

/* Debug led */
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);

/* USB UART device */
static const struct device *usb_uart_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));

static void setup_debug_led(void)
{
	int ret;

	ret = device_is_ready(led.port);
	if (!ret) {
		k_oops();
	}

	ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		k_oops();
	}
}

static void wait_for_uart_attach(void)
{
	uint32_t dtr = 0;
	uint32_t timeout = 20;

	/* Poll if the DTR flag was set.
	 * Blocks in the while loop until terminal is attached.
	 */
	while (!dtr && timeout > 0) {
		uart_line_ctrl_get(usb_uart_dev, UART_LINE_CTRL_DTR, &dtr);
		// Give CPU resources to low priority threads.
		k_msleep(100);
		--timeout;
		gpio_pin_toggle_dt(&led);
	}
}

void main(void)
{
	/* Ready the red debug LED. */
	setup_debug_led();

	/* Ready USB. */
	usb_enable(NULL);

	/* Ready bluetooth. */
	ble_start();

	wait_for_uart_attach();

	printk("Starting Hikari Light Application\n");

	light_resource_init();

	gpio_pin_set_dt(&led, 0);
	while (1) {
		gpio_pin_toggle_dt(&led);
		k_msleep(SLEEP_TIME_MS);
	}
}
