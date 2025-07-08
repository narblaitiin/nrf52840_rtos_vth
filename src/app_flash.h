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

//  ========== globals =====================================================================
struct vth {
	int16_t vbat;
	int16_t temp;
	int16_t hum;
};

//  ========== defines =====================================================================
#define FLASH_PARTITION_OFFSET  		0x000FC000
#define FLASH_SECTOR_SIZE       		2048
#define FLASH_SECTOR_COUNT      		8
#define FLASH_TOTAL_SIZE				(FLASH_SECTOR_SIZE * FLASH_SECTOR_COUNT)

#define MAX_RECORDS 					10
#define RECORD_SIZE sizeof(struct vth)
#define FLASH_HEAD_OFFSET FLASH_PARTITION_OFFSET           // stores current head index (4 bytes)
#define FLASH_DATA_OFFSET (FLASH_HEAD_OFFSET + sizeof(uint32_t))  // start of actual data                               

//  ========== prototypes ==================================================================
int8_t app_flash_init(const struct device *dev);
int8_t app_flash_store(const struct device *dev, const struct vth *data);
int8_t app_flash_handler(const struct device *dev);

#endif /* APP_FLASH_H */