#include <zephyr/zephyr.h>
#include <sys/printk.h>
#include <usb/usb_device.h>
#include <drivers/uart.h>
#include <drivers/gpio.h>

#define SLEEP_TIME_MS 500

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);

static const struct device *usb_uart_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));

static void setup_debug_led(void)
{
	(void)device_is_ready(led.port);
	(void)gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
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

#if 0
#include <ztest.h>

static void *cms_suite_setup(void)
{
	setup_debug_led();

	usb_enable(NULL);
	wait_for_uart_attach();

	return NULL;
}

static void cms_suite_teardown(void *fixture)
{
}

ZTEST_SUITE(cms_suite, NULL, cms_suite_setup, NULL, NULL, cms_suite_teardown);

ZTEST(cms_suite, basic)
{
	gpio_pin_set_dt(&led, 0);
	k_msleep(SLEEP_TIME_MS);
	gpio_pin_toggle_dt(&led);
	k_msleep(SLEEP_TIME_MS);
	gpio_pin_toggle_dt(&led);
	k_msleep(SLEEP_TIME_MS);
	gpio_pin_toggle_dt(&led);

	printk("Hello world!\n");
}
#endif

void main(void)
{
	setup_debug_led();

	usb_enable(NULL);

	wait_for_uart_attach();

	printk("Starting cms test\n");

	gpio_pin_set_dt(&led, 0);
	while (1) {
		gpio_pin_toggle_dt(&led);
		k_msleep(SLEEP_TIME_MS);
	}
}
