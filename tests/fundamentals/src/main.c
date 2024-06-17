
#include <zephyr/sys/printk.h>
#include <zephyr/kernel.h>

K_EVENT_DEFINE(my_event);

void main(void)
{
	uint32_t ret;

	k_event_post(&my_event, 0x20);

	ret = k_event_wait(&my_event, 0x120, false, K_MSEC(1000));

	printk("Hello World: 0x%08x\n", ret);

	ret = k_event_wait(&my_event, 0x040, false, K_MSEC(1000));

	printk("Hello World: 0x%08x\n", ret);
}
