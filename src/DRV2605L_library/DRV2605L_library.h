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

#ifndef DRV2605L_LIBRARY_H__
#define DRV2605L_LIBRARY_H__

#ifdef __cplusplus
extern "C" {
#endif

/*
Send Message through uart
uart_port: ponter to uart instance to use.
message: pointer data to be sent through TX.
size: size of data to be sent through TX 
*/
void elva_uart_transmit(struct device *uart_port , uint8_t *message, uint32_t size);

void init_MT15(struct device *uart_port);

#ifdef __cplusplus
}
#endif

#endif /* DRV2605L_LIBRARY_H__ */
