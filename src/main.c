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
	// retrieve the sensor device using the device tree API
	const struct device *sht31_dev = DEVICE_DT_GET_ONE(sensirion_sht3xd);
	if (!device_is_ready(sht31_dev)) {
        printk("%s: sensor device not ready\n", sht31_dev->name);
        return -ENODEV;
    }
	
	// printk("two sensors test and W/R flash memory test\n");
 	// int8_t ret = app_flash_handler(sht31_dev);
	// if (ret != 1) {
	// 	printk("failed to call sensor handler");
	// 	return 0;
	// }

	printk("only the two sensors test: ADC & SHT31\n");

	int16_t bat = app_nrf52_get_vbat();
	printk("battery level (int16): %d%%\n", bat);

	int16_t temp = app_sht31_get_temp(sht31_dev);
	printk("SHT31 temperature (int16): %d\n", temp);

	k_msleep(5000);		// small delay  between reading the temperature and humidity values

	int16_t hum = app_sht31_get_hum(sht31_dev);
	printk("SHT31 humidity (int16): %d\n", hum);
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

	// initialize ADC device
	int8_t ret = app_nrf52_vbat_init();
	if (ret != 1) {
		printk("failed to initialize ADC device");
		return 0;
	}
	printk("adc device initialized\n");

	// initialize partition flash memory
	ret = app_flash_init();
	if (ret != 1) {
		printk("failed to initialize internal Flash device\n");
		return 0;
	}

	printk("Sensor SHT31 and Battery Example\n");

	// start the timer to trigger the interrupt subroutine every 30 seconds
	k_timer_start(&sens_timer, K_NO_WAIT, K_SECONDS(30));		// 30s for test
	return 0;
}