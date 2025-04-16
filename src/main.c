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
	const struct device *dev;
	int16_t bat, temp, hum;

	printk("sensor handler called\n");
  app_flash_handler(&flash);

	// printk("only the two sensors test: ADC & SHT31\n");

	// bat = app_nrf52_get_vbat();
	// printk("battery level (int16): %d%%\n", bat);

	// temp = app_sht31_get_temp(dev);
	// printk("sht31 Temperature (int16): %d\n", temp);

	// k_msleep(2000);		// small delay  between reading the temperature and humidity values

	// hum = app_sht31_get_hum(dev);
	// printk("sht31 humidity (int16): %d", hum);
}
K_WORK_DEFINE(sens_work, sens_work_handler);

void sens_timer_handler(struct k_timer *rtc_dum)
{
	printk("timer handler triggered\n");
	k_work_submit(&sens_work);
}
K_TIMER_DEFINE(sens_timer, sens_timer_handler, NULL);

//  ========== main ========================================================================
int main(void)
{
	const struct device *dev = NULL;
	struct nvs_fs flash;

	// initialize all devices
	app_sht31_init(dev);
	app_nrf52_vbat_init();
	app_flash_init(&flash);
	
	printk("Sensor SHT31 and Battery Example\nBoard: %s\n", CONFIG_BOARD);

	// start the timer to trigger the interrupt subroutine every 30 seconds
	k_timer_start(&sens_timer, K_SECONDS(30), K_SECONDS(30));		// 30s for test

	return 0;
}