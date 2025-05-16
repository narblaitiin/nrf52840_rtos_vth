/*
 * Copyright (c) 2025
 * Regis Rousseau
 * Univ Lyon, INSA Lyon, Inria, CITI, EA3720
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef APP_VBAT_H
#define APP_VBAT_H

//  ========== includes ====================================================================
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/adc.h>
#include <math.h>

//  ========== defines =====================================================================
#define ADC_REFERENCE_VOLTAGE       3300    // 3.3V reference voltage of the board
#define ADC_RESOLUTION              4096    // 12-bit resolution
#define BATTERY_MAX_VOLTAGE         2980
#define BATTERY_MIN_VOLTAGE         2270

//  ========== prototypes ==================================================================
int8_t app_nrf52_vbat_init();
int16_t app_nrf52_get_vbat();

#endif /* APP_VBAT_H */