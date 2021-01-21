/*
 * Copyright (c) 2017 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
 */
/**@file
 *
 * @brief   Orientation detector module.
 *
 * Module that uses accelerometer data to detect its orientation.
 */

#ifndef ELVA_INITIALIZER_H__
#define ELVA_INITIALIZER_H__

#define LED_GRN   16
#define LED_RED   17
#define DISPLAY   27
#define SCANNER   21

#ifdef __cplusplus
extern "C" {
#endif

static struct device *dev;
static struct device *i2c_dev;
static const char* const I2C_DEV = "I2C_2";
//#ifdef CONFIG_SOC_NRF9160
//#define I2C_DEV "I2C_2" //"I2C_3"
//#endif

void sayHello2(void);

void init_elva_gpio(void); //(struct device* gpio_port);

void vibrate(bool toggle);

void init_elva_i2c(void);

#ifdef __cplusplus
}
#endif

#endif /* ELVA_INITIALIZER_H__ */
