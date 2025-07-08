/*
 * Copyright (c) 2023
 * Regis Rousseau
 * Univ Lyon, INSA Lyon, Inria, CITI, EA3720
 * SPDX-License-Identifier: Apache-2.0
 */

//  ========== includes ====================================================================
#include "app_vbat.h"
#include "app_sht31.h"
#include "app_flash.h"

//  ========== app_flash_init ==============================================================
int8_t app_flash_init(const struct device *dev)
{
	// check if partition is ready
    if (!device_is_ready(dev)) {
        printk("flash device not ready\n");
        return 0;
    }

	 // initialize head index to 0
    uint32_t head = 0;
    int8_t ret = flash_write(dev, FLASH_HEAD_OFFSET, &head, sizeof(head));
    if (ret != 0) {
        printk("failed to write initial head\n");
        return -1;
    }

    printk("flash initialized. Sector size: %d, count: %d\n", FLASH_SECTOR_SIZE, FLASH_SECTOR_COUNT);
    return 1;
}

//  ========== app_flash_store =============================================================
int8_t app_flash_store(const struct vth *data)
{
    const struct flash_area *fa;
    int ret = flash_area_open(DT_FLASH_AREA_ID(storage), &fa);
    if (ret != 0) {
        printk("Failed to open flash area\n");
        return -1;
    }

    uint32_t head = 0;
    flash_area_read(fa, 0, &head, sizeof(head));  // head at offset 0

    uint32_t index = head % MAX_RECORDS;
    off_t data_offset = sizeof(head) + index * FLASH_SECTOR_SIZE;

    // Erase sector
    ret = flash_area_erase(fa, data_offset, FLASH_SECTOR_SIZE);
    if (ret != 0) {
        printk("Erase failed at offset 0x%x\n", (uint32_t)data_offset);
        return -1;
    }

    // Write record
    ret = flash_area_write(fa, data_offset, data, sizeof(struct vth));
    if (ret != 0) {
        printk("Write failed at offset 0x%x\n", (uint32_t)data_offset);
        return -1;
    }

    // Update head
    head = (head + 1) % MAX_RECORDS;
    flash_area_write(fa, 0, &head, sizeof(head));

    flash_area_close(fa);
    return 1;
}

//  ========== app_flash_handler ===========================================================
int8_t app_flash_handler(const struct device *dev)
{
	struct vth data;

	// retrieve the sensor device using the device tree API
	const struct device *sht31_dev = DEVICE_DT_GET_ONE(sensirion_sht3xd);
	if (!device_is_ready(sht31_dev)) {
        printk("%s: sensor device not ready\n", sht31_dev->name);
        return -1;
    }

	if (!device_is_ready(dev)) {
        printk("%s: device is not ready\n", dev->name);
        return -1;
    }

	// collect sensor data until the maximum number of records is reached
	// measure and store the battery voltage
	data.vbat = app_nrf52_get_vbat();

	// measure and store the temperature using the SHT31 sensor
	data.temp = app_sht31_get_temp(sht31_dev);

	k_msleep(5000);		// small delay  between reading the temperature and humidity values

	// measure and store the humidity using the SHT31 sensor
	data.hum = app_sht31_get_hum(sht31_dev);

	// save the collected sensor data into the flash memory
	return app_flash_store(&data);
}

