#include <zephyr/zephyr.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/conn.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>

#include "hikari_light.h"

#define RGB_SERVICE_VAL \
	BT_UUID_128_ENCODE(0xcbb84a52, 0x063d, 0x463f, 0x9a71, 0x9aac578001d8)

#define RGB_MODE_CHRC_VAL \
	BT_UUID_128_ENCODE(0xf1873813, 0xe162, 0x4cc7, 0x8f87, 0x33442611ae6e)

#define RGB_CONF_CHRC_VAL \
	BT_UUID_128_ENCODE(0xdcbed995, 0x388c, 0x413b, 0x8f40, 0x7d1ed59a75a7)

#define DETECTOR_STATE_CHRC_VAL \
	BT_UUID_128_ENCODE(0x4794fb9a, 0xe4fc, 0x497f, 0x9e51, 0x85b92b57e4e3)

static struct bt_uuid_128 rgb_srv_uuid = BT_UUID_INIT_128(RGB_SERVICE_VAL);
static struct bt_uuid_128 rgb_mode_chrc_uuid = BT_UUID_INIT_128(RGB_MODE_CHRC_VAL);
static struct bt_uuid_128 rgb_conf_chrc_uuid = BT_UUID_INIT_128(RGB_CONF_CHRC_VAL);
static struct bt_uuid_128 detector_state_chrc_uuid = BT_UUID_INIT_128(DETECTOR_STATE_CHRC_VAL);

static ssize_t read_mode(struct bt_conn *conn, const struct bt_gatt_attr *attr,
						 void *buf, uint16_t len, uint16_t offset)
{
	//enum hikari_light_mode mode = hikari_light_mode_get();
	enum hikari_light_mode mode = 23;

	return bt_gatt_attr_read(conn, attr, buf, len, offset, (uint8_t *)&mode, sizeof(uint8_t));
}

static ssize_t write_mode(struct bt_conn *conn, const struct bt_gatt_attr *attr,
						  const void *buf, uint16_t len, uint16_t offset, uint8_t flags)
{
	if (offset + len > sizeof(uint8_t)) {
		return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
	}

	enum hikari_light_mode mode = ((uint8_t *)buf)[0];
	//hikari_light_mode_set(mode);

	return len;
}

static ssize_t read_conf(struct bt_conn *conn, const struct bt_gatt_attr *attr,
						 void *buf, uint16_t len, uint16_t offset)
{
	static bool report_bigger = true;

	const uint8_t *conf = (uint8_t *)attr->user_data;

	uint8_t conf_to_read;
	if (report_bigger) {
		conf_to_read = *conf + 1;
		report_bigger = false;
	} else {
		conf_to_read = *conf;
		report_bigger = true;
	}

	return bt_gatt_attr_read(conn, attr, buf, len, offset, (void *)(&conf_to_read), sizeof(uint8_t));
}

static ssize_t write_conf(struct bt_conn *conn, const struct bt_gatt_attr *attr,
						  const void *buf, uint16_t len, uint16_t offset, uint8_t flags)
{
	uint8_t *conf_ptr = (uint8_t *)attr->user_data;

	if (offset + len > sizeof(uint8_t)) {
		return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
	}

	memcpy(conf_ptr + offset, buf, len);

	return len;
}

static ssize_t read_detector_state(struct bt_conn *conn, const struct bt_gatt_attr *attr,
								   void *buf, uint16_t len, uint16_t offset)
{
	static bool detecting = true;

	if (offset + len > sizeof(uint8_t)) {
		return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
	}

	detecting = !detecting;

	return bt_gatt_attr_read(conn, attr, buf, len, offset, (void *)(uint8_t *)detecting, sizeof(uint8_t));
}

static bool notify_detector_state = false;

static void detector_state_ccc_cfg_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
	notify_detector_state = (value == BT_GATT_CCC_NOTIFY) ? 1 : 0;
}

static uint8_t conf_value = 21;
static uint8_t detector_state = 3;

BT_GATT_SERVICE_DEFINE(rgb_svc,
	BT_GATT_PRIMARY_SERVICE(&rgb_srv_uuid),
	BT_GATT_CHARACTERISTIC((const struct bt_uuid *)&rgb_mode_chrc_uuid,
						   BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE,
						   BT_GATT_PERM_READ | BT_GATT_PERM_WRITE,
						   read_mode, write_mode, NULL),
	BT_GATT_CHARACTERISTIC((const struct bt_uuid *)&rgb_conf_chrc_uuid,
						   BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE,
						   BT_GATT_PERM_READ | BT_GATT_PERM_WRITE,
						   read_conf, write_conf, &conf_value),
	BT_GATT_CHARACTERISTIC((const struct bt_uuid *)&detector_state_chrc_uuid,
						   BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
						   BT_GATT_PERM_READ,
						   read_detector_state, NULL, &detector_state),
	BT_GATT_CCC(detector_state_ccc_cfg_changed,
				BT_GATT_PERM_READ | BT_GATT_PERM_WRITE)
);

static void connected(struct bt_conn *conn, uint8_t err)
{
	if (err) {
		printk("Connection failed (err 0x%02x)", err);
	} else {
		printk("Connected\n");
	}
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
	printk("Disconnected (reason 0x%02x)\n", reason);
}

BT_CONN_CB_DEFINE(conn_callbacks) = {
	.connected = connected,
	.disconnected = disconnected,
};

static const struct bt_data adv[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA_BYTES(BT_DATA_UUID128_ALL, RGB_SERVICE_VAL),
};

void mtu_updated(struct bt_conn *conn, uint16_t tx, uint16_t rx)
{
	printk("Updated MTU: TX: %d RX: %d bytes\n", tx, rx);
}

static struct bt_gatt_cb gatt_callbacks = {
	.att_mtu_updated = mtu_updated,
};

void ble_start(void)
{
	int err;

	err = bt_enable(NULL);
	if (err) {
		printk("Bluetooth init failed, err %d\n", err);
		return;
	}
	printk("Bluetooth successfully initialized\n");

	err = bt_le_adv_start(BT_LE_ADV_CONN_NAME, adv, ARRAY_SIZE(adv), NULL, 0);
	if (err) {
		printk("Advertising failed to start, err %d\n", err);
		return;
	}
	printk("Advertising successfully started\n");

	bt_gatt_cb_register(&gatt_callbacks);
}