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

	// configure the storage partition in flash memory
	fs->flash_device = NVS_PARTITION_DEVICE;

	// check if the flash device is ready
	if (!device_is_ready(fs->flash_device)) {
		printk("%s: device is not ready\n", fs->flash_device->name);
		return 0;
	}

	// set the offset of the NVS partition in flash memory
	fs->offset = NVS_PARTITION_OFFSET;

	// retrieve information about the flash memory page at the specified offset
	int8_t ret = flash_get_page_info_by_offs(fs->flash_device, fs->offset, &info);
	if (ret) {
		printk("unable to get page info. error: %d\n", ret);
		return 0;
	}
	
	// set the sector size for the NVS partition
	fs->sector_size = info.size;

	// validate the sector size
	if (!fs->sector_size) {
		printk("invalid sector size\n");
		return 0;
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
//	(void)nvs_delete(fs, NVS_SENSOR_ID);

	// calculate and display the free space in the NVS partition
	ssize_t size =  nvs_calc_free_space(fs);
	printk("flash memory partition size: %d\n", size);

	return 1;
}

//  ========== app_flash_store =============================================================
int8_t app_flash_store(struct nvs_fs *fs, const struct vth *data)
{	
	int8_t ret;
	
	// write the data to the first page of the flash memory (8 KB)
	ret = nvs_write(fs, NVS_SENSOR_ID, data, sizeof(struct vth) * NVS_MAX_RECORDS);
	if (ret < 0) {
        printk("failed to write data to NVS. error: %d\n", ret);
        return 0;
    }

	// log the data being written to flash memory
	for (size_t i = 0; i < NVS_MAX_RECORDS; i++) {
		printk("write -> vbat: %d, temp: %d, hum: %d\n", data[i].vbat, data[i].temp, data[i].hum);
	}

	// read the data back from the first page of the flash memory
	struct vth read_data[NVS_MAX_RECORDS];
	ret = nvs_read(fs, NVS_SENSOR_ID, read_data, sizeof(struct vth) * NVS_MAX_RECORDS);
	if (ret < 0) {
        printk("failed to read data from NVS. Error: %d\n", ret);
        return 0;
    }

	// log the data retrieved from flash memory to verify it matches the original data
	for (size_t i = 0; i < NVS_MAX_RECORDS; i++) {
		printk("read -> vbat: %d, temp: %d, hum: %d", read_data[i].vbat, read_data[i].temp, read_data[i].hum);
	}
	return 1;
}

//  ========== app_flash_handler ===========================================================
int8_t app_flash_handler(struct nvs_fs *fs)
{
	struct vth data[NVS_MAX_RECORDS];

	// retrieve the sensor device using the device tree API
	const struct device *sht31_dev = DEVICE_DT_GET_ONE(sensirion_sht3xd);
	if (!device_is_ready(sht31_dev)) {
        printk("%s: sensor device not ready\n", sht31_dev->name);
        return 0;
    }

	// collect sensor data until the maximum number of records is reached
	for (size_t i = 0; i < NVS_MAX_RECORDS; i++) {
		// measure and store the battery voltage
		data[i].vbat = app_nrf52_get_vbat();

		// measure and store the temperature using the SHT31 sensor
		data[i].temp = app_sht31_get_temp(sht31_dev);

		k_msleep(2000);		// small delay  between reading the temperature and humidity values

		// measure and store the humidity using the SHT31 sensor
		data[i].hum = app_sht31_get_hum(sht31_dev);
	}

	// save the collected sensor data into the flash memory
	int8_t ret = app_flash_store(fs, &data);
	if (ret < 0) {
        printk("failed to store data in flash memory. error: %d\n", ret);
        return 0;
    }

	// delete the sensor data from flash memory (likely for testing purposes)
	ret = nvs_delete(fs, NVS_SENSOR_ID);
	if (ret < 0) {
        printk("failed to delete data from flash memory. error: %d\n", ret);
    }
	return 1;
}

