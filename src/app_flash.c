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

	// configure the storage partition in flash memory
	fs->flash_device = NVS_PARTITION_DEVICE;

	// check if the flash device is ready
	if (!device_is_ready(fs->flash_device)) {
		printk("flash device \"%s\" is not ready\n", fs->flash_device->name);
		return 0;
	} else {
		printk("- found device: \"%s\", getting nvs memory\n", fs->flash_device->name);
	}

	// set the offset of the NVS partition in flash memory
	fs->offset = NVS_PARTITION_OFFSET;

	// retrieve information about the flash memory page at the specified offset
	ret = flash_get_page_info_by_offs(fs->flash_device, fs->offset, &info);
	if (ret) {
		printk("unable to get page info. error: %d\n", ret);
		return ret;
	}
	
	// set the sector size for the NVS partition
	fs->sector_size = info.size;

	// validate the sector size
	if (!fs->sector_size) {
		printk("invalid sector size\n");
		return -1;
	}
	printk("sector size: %d\n", info.size);

	// mount the NVS partition on the flash memory
	fs->sector_count = 4U;		 // number of sectors allocated to the NVS partition
	ret = nvs_mount(fs);
	if (ret) {
		printk("flash to initialize flash memory. error: %d\n", ret);
		return 0;
	}

	// clean up data stored in the NVS partition
	(void)nvs_delete(fs, NVS_SENSOR_ID);

	// calculate and display the free space in the NVS partition
	size =  nvs_calc_free_space(fs);
	printk("flash memory partition size: %d\n", size);

	return 0;
}

//  ========== app_flash_store =============================================================
int8_t app_flash_store(struct nvs_fs *fs, struct vth *data)
{	
	int8_t itr;
	
	// write the data to the first page of the flash memory (8 KB)
	(void)nvs_write(fs, NVS_SENSOR_ID, data, sizeof(*data)*NVS_MAX_RECORDS);

	// log the data being written to flash memory
	for (itr = 0; itr < NVS_MAX_RECORDS; itr++) {
		printk("wrt -> vbat: %d, temp: %d, hum: %d\n", data[itr].vbat, data[itr].temp, data[itr].hum);
	}

	// read the data back from the first page of the flash memory
	(void)nvs_read(fs, NVS_SENSOR_ID, data, sizeof(*data)*NVS_MAX_RECORDS);

	// log the data retrieved from flash memory to verify it matches the original data
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

	// retrieve the sensor device using the device tree API
	dev = DEVICE_DT_GET_ONE(sensirion_sht3xd);

	// collect sensor data until the maximum number of records is reached
	while (itr < NVS_MAX_RECORDS) {
		// measure and store the battery voltage
		data[itr].vbat = app_nrf52_get_vbat();

		// measure and store the temperature using the SHT31 sensor
		data[itr].temp = app_sht31_get_temp(dev);

		// measure and store the humidity using the SHT31 sensor
		data[itr].hum = app_sht31_get_hum(dev);

		// move to the next record
		itr++;
	}

	// save the collected sensor data into the flash memory
	app_flash_store(fs, &data);

	// delete the sensor data from flash memory (likely for testing purposes)
	(void)nvs_delete(fs, NVS_SENSOR_ID);

	return 0;
}

