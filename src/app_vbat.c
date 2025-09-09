/*
 * Copyright (c) 2025
 * Regis Rousseau
 * Univ Lyon, INSA Lyon, Inria, CITI, EA3720
 * SPDX-License-Identifier: Apache-2.0
 */


//  ========== includes ====================================================================
#include "app_vbat.h"

//  ========== globals =====================================================================
// ADC buffer to store raw ADC readings
int32_t buf;

// ADC channel configuration obtained from the device tree
static const struct adc_dt_spec adc_channel = ADC_DT_SPEC_GET(DT_PATH(zephyr_user));

// ADC sequence configuration to specify the ADC operation
static struct adc_sequence sequence = {
    .channels = 1,
	.buffer = &buf,
	.buffer_size = sizeof(buf),
};

//  ========== app_nrf52_vbat_init =========================================================
int8_t app_nrf52_vbat_init()
{
    int8_t ret;

    // verify if the ADC is ready for operation
    if (!adc_is_ready_dt(&adc_channel)) {
		printk("ADC is not ready. error: %d\n", ret);
		return 0;
	}

    // configure the ADC channel settings
    ret = adc_channel_setup_dt(&adc_channel);
	if (ret < 0) {
		printk("failed to set up ADC channel. error: %d\n", ret);
		return 0;
	}

    // initialize the ADC sequence for continuous or single readings
    ret = adc_sequence_init_dt(&adc_channel, &sequence);
	if (ret < 0) {
		printk("failed to initialize ADC sequence. error: %d\n", ret);
		return 0;
	}
    return 1;
}

//  ======== app_nrf52_get_vbat =============================================
int16_t app_nrf52_get_vbat()
{
    int16_t percent = 0;

    // read sample from the ADC
    int8_t ret = adc_read(adc_channel.dev, &sequence);
    if (ret < 0 ) {        
	    printk("raw adc value is not up to date. error: %d\n", ret);
	    return 0;
    }
    printk("raw adc value: %d\n", buf);

    // convert raw ADC reading to voltage
    int32_t v_adc = (buf * ADC_FULL_SCALE_MV) / ADC_RESOLUTION;
    printk("convert voltage: %d mV\n", v_adc);

    // scale back to actual battery voltage using voltage divider
    int32_t v_bat = (v_adc * DIVIDER_RATIO_NUM) / DIVIDER_RATIO_DEN;

    // clamp voltage within battery range
    if (v_bat > BATTERY_MAX_VOLTAGE) v_bat = BATTERY_MAX_VOLTAGE;
    if (v_bat < BATTERY_MIN_VOLTAGE) v_bat = BATTERY_MIN_VOLTAGE + 1;
    printk("clamped voltage: %d mV\n", v_bat);

    // non-linear scaling
    int32_t range = BATTERY_MAX_VOLTAGE - BATTERY_MIN_VOLTAGE;
    int32_t difference = v_bat - BATTERY_MIN_VOLTAGE;

    if (range > 0 && difference > 0) {
        double normalized = (double)difference / range;  // normalize to range [0, 1]
        double scaled = pow(normalized, 1.2);            // apply non-linear scaling
        percent = (int16_t)(scaled * 100);       // convert to percentage
    } else {
        printk("invalid range or difference\n");
        percent = 0;
    }

    printk("battery level (non-linear, int16): %d%%\n", percent);
    return percent;
}
