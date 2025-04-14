/*
 * Copyright (c) 2025
 * Regis Rousseau
 * Univ Lyon, INSA Lyon, Inria, CITI, EA3720
 * SPDX-License-Identifier: Apache-2.0
 */


//  ========== includes ====================================================================
#include "app_vbat.h"

//  ========== globals =====================================================================
static int16_t buf;
static const struct adc_dt_spec adc_channel = ADC_DT_SPEC_GET(DT_PATH(zephyr_user));
static struct adc_sequence sequence = {
        .channels = 1, // BIT(adc_channel.channel_id);
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
    int32_t voltage = 0;        // variable to store converted ADC value

    // read sample from the ADC
    ret = adc_read(adc_channel.dev, &sequence);
    if (ret < 0 ) {        
	    printk("raw adc valueis not up to date. error: %d\n", ret);
	    return 0;
    }
    printk("raw adc value: %d\n", buf);

    // convert ADC reading to voltage
    voltage = (buf * ADC_REFERENCE_VOLTAGE) / ADC_RESOLUTION;
    printk("convert voltage: %d mV\n", voltage);

    // ensure voltage is within range
    if (voltage > BATTERY_MAX_VOLTAGE) voltage = BATTERY_MAX_VOLTAGE;
    if (voltage < BATTERY_MIN_VOLTAGE) voltage = BATTERY_MIN_VOLTAGE + 1;
    printk("clamped voltage: %d mV\n", voltage);

    // non-linear scaling
    int32_t range = BATTERY_MAX_VOLTAGE - BATTERY_MIN_VOLTAGE;
    int32_t difference = voltage - BATTERY_MIN_VOLTAGE;

    if (range > 0 && difference > 0) {
        // use power scaling: percentage = ((difference / range) ^ 1.5) * 100
        double normalized = (double)difference / range;  // normalize to range [0, 1]
        double scaled = pow(normalized, 1.5);            // apply non-linear scaling
        percent = (int16_t)(scaled * 100);               // convert to percentage
    } else {
        printk("error: Invalid range or difference.\n");
        percent = 0;
    }

    printk("battery level (non-linear, int16): %d%%\n", percent);
    return percent;
}
