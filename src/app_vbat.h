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

//  ========== prototypes ==================================================================
int8_t app_nrf52_vbat_init();
int16_t app_nrf52_get_vbat();

#endif /* APP_VBAT_H */