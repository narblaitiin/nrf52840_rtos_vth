/*
 * Copyright (c) 2025
 * Regis Rousseau
 * Univ Lyon, INSA Lyon, Inria, CITI, EA3720
 * SPDX-License-Identifier: Apache-2.0
 */

//  ========== includes ====================================================================
#include "app_flash.h"
#include "app_sht31.h"
#include "app_vbat.h"

//  ========== interrupt sub-routine =======================================================
void sens_work_handler(struct k_work *work_rtc)
{
	struct nvs_fs flash;

	printk("sensor handler called\n");
	app_flash_handler(&flash);
}
K_WORK_DEFINE(sens_work, sens_work_handler);

void sens_timer_handler(struct k_timer *rtc_dum)
{
	k_work_submit(&sens_work);
}
K_TIMER_DEFINE(sens_timer, sens_timer_handler, NULL);

//  ========== main ========================================================================
int main(void)
{
	const struct device *dev = NULL;
	struct nvs_fs flash;

	// setup all devices
	app_sht31_init(dev);
	app_nrf52_vbat_init();
	app_flash_init(&flash);
	
	printk("Sensor SHT31 and Battery Example\nBoard: %s\n", CONFIG_BOARD);

	// beginning of interrupt subroutine
	k_timer_start(&sens_timer, K_NO_WAIT, K_MSEC(30000));		// 10s for test

	return 0;
}