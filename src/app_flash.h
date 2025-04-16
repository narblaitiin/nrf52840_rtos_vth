/*
 * Copyright (c) 2023
 * Regis Rousseau
 * Univ Lyon, INSA Lyon, Inria, CITI, EA3720
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef APP_FLASH_H
#define APP_FLASH_H

//  ========== includes ====================================================================
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/flash.h>
#include <zephyr/storage/flash_map.h>
#include <zephyr/fs/nvs.h>

//  ========== globals =====================================================================
struct vth {
	int16_t vbat;
	int16_t temp;
	int16_t hum;
};

//  ========== defines =====================================================================
#define NVS_PARTITION			storage_partition
#define NVS_PARTITION_DEVICE	FIXED_PARTITION_DEVICE(NVS_PARTITION)
#define NVS_PARTITION_OFFSET	FIXED_PARTITION_OFFSET(NVS_PARTITION)
#define NVS_MAX_RECORDS         5	// 1 structure: 3 samples (vbat, temp, hum) = 6 bytes
#define NVS_SENSOR_ID           1                                       

//  ========== prototypes ==================================================================
int8_t app_flash_init(struct nvs_fs *fs);
int8_t app_flash_store(struct nvs_fs *fs, const struct vth *data);
int8_t app_flash_handler(struct nvs_fs *fs);

#endif /* APP_FLASH_H */