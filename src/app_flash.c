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
int8_t app_flash_init(struct nvs_fs *fs)
{
	struct flash_pages_info info;
	int8_t ret = 0;
	ssize_t size = 0;

	// configuration of storage partition in flash memory
	fs->flash_device = NVS_PARTITION_DEVICE;
	if (!device_is_ready(fs->flash_device)) {
		printk("flash device \"%s\" is not ready\n", fs->flash_device->name);
		return 0;
	} else {
		printk("- found device: \"%s\", getting nvs memory\n", fs->flash_device->name);
	}

	fs->offset = NVS_PARTITION_OFFSET;
	ret = flash_get_page_info_by_offs(fs->flash_device, fs->offset, &info);
	if (ret) {
		printk("unable to get page info. error: %d\n", ret);
		return ret;
	}
	
	fs->sector_size = info.size;
	if (!fs->sector_size) {
		printk("invalid sector size\n");
		return -1;
	}
	printk("sector size: %d\n", info.size);

	// mounting data storage partition in flash memory
	fs->sector_count = 4U;
	ret = nvs_mount(fs);
	if (ret) {
		printk("flash init failed. error: %d\n", ret);
		return 0;
	}

	// cleaning data storage partition
	(void)nvs_delete(fs, NVS_SENSOR_ID);
	size =  nvs_calc_free_space(fs);
	printk("flash memory partition size: %d\n", size);

	return 0;
}

//  ========== app_flash_store =============================================================
int8_t app_flash_store(struct nvs_fs *fs, struct vth *data)
{	
	int8_t itr;
	
	// writing data in the first page of 2kbytes
	(void)nvs_write(fs, NVS_SENSOR_ID, data, sizeof(*data)*NVS_MAX_RECORDS);

	// printing data to be stored in memory
	for (itr = 0; itr < NVS_MAX_RECORDS; itr++) {
		printk("wrt -> vbat: %d, temp: %d, hum: %d\n", data[itr].vbat, data[itr].temp, data[itr].hum);
	}

	// reading the first page
	(void)nvs_read(fs, NVS_SENSOR_ID, data, sizeof(*data)*NVS_MAX_RECORDS);

	// printing data stored in memory
	for (itr = 0; itr < NVS_MAX_RECORDS; itr++) {
		printk("rd -> vbat: %d, temp: %d, hum: %d", data[itr].vbat, data[itr].temp, data[itr].hum);
	}
	return 0;
}

//  ========== app_flash_handler ===========================================================
int8_t app_flash_handler(struct nvs_fs *fs)
{
	int8_t itr = 0;
	int16_t vbat, temp, hum;
	const struct device *dev;
	struct vth data[NVS_MAX_RECORDS];

	// get sensor device
	dev = DEVICE_DT_GET_ONE(sensirion_sht3xd);

	// collect sensor data
	while (itr < NVS_MAX_RECORDS) {
		data[itr].vbat = app_nrf52_get_vbat();
		data[itr].temp = app_sht31_get_temp(dev);
		data[itr].hum = app_sht31_get_hum(dev);
		itr++;
	}
	// store data in flash memory
	app_flash_store(fs, &data);

	// clear the data from flash (for test purposes)
	(void)nvs_delete(fs, NVS_SENSOR_ID);
	return 0;
}

