/*
 * Copyright (c) 2025
 * Regis Rousseau
 * Univ Lyon, INSA Lyon, Inria, CITI, EA3720
 * SPDX-License-Identifier: Apache-2.0
 */


//  ========== includes ====================================================================
#include "app_vbat.h"

//  ========== globals =====================================================================
int16_t buf;
static const struct adc_dt_spec adc_channel = ADC_DT_SPEC_GET(DT_PATH(zephyr_user));
struct adc_sequence sequence = {
        .channels = 1,
		.buffer = &buf,
		.buffer_size = sizeof(buf),
};

//  ========== app_nrf52_vbat_init =========================================================
int8_t app_nrf52_vbat_init()
{
    int8_t ret = 0;

    if (!adc_is_ready_dt(&adc_channel)) {
		printk("ADC is not ready. error: %d\n", ret);
		return 0;
	} else {
        printk("- found device \"%s\", getting vbat data\n", adc_channel.dev->name);
    }

    // setup ADC channel
    ret = adc_channel_setup_dt(&adc_channel);
	if (ret < 0) {
		printk("could not setup channel. error: %d\n", ret);
		return 0;
	}

    // initializing ADC sequence
    ret = adc_sequence_init_dt(&adc_channel, &sequence);
	if (ret < 0) {
		printk("could not initalize sequnce. error: %d\n", ret);
		return 0;
	}
    return 0;
}

//  ======== app_nrf52_get_vbat =============================================
int16_t app_nrf52_get_vbat()
{
    int16_t percent = 0;
    int8_t ret = 0;
    float max = 4.2;
    float min = 3.0;

    // reading sample from the ADC
    ret = adc_read(adc_channel.dev, &sequence);
    if (ret < 0 ) {        
	    printk("nrf52 vbat sample is not up to date. error: %d\n", ret);
	    return 0;
    }

    // battery level in percent
    percent = (int16_t) ((buf - min) / (max - min)) * 100.0;
    return percent;
}
