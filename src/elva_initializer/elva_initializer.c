/*

* Copyright (c) 2012-2014 Wind River Systems, Inc.

*

* SPDX-License-Identifier: Apache-2.0

*/

#include <nrf9160.h>
#include <zephyr.h>
#include <misc/printk.h>
#include <i2c.h>

#include <device.h>
#include <gpio.h>

#include "elva_initializer.h"



void sayHello2(void){
  printk("\nhello from elva initilaizer\n");
  }

void init_elva_gpio(void){ //(struct device* gpio_port){
  dev = device_get_binding("GPIO_0");

  /*--Set LED pin as output--*/
  gpio_pin_configure(dev, 6, GPIO_DIR_IN); //p0.27 ==> BUTTON A
  gpio_pin_configure(dev, 7, GPIO_DIR_IN); //p0.07 ==> BUTTON B

  gpio_pin_configure(dev, DISPLAY, GPIO_DIR_OUT); //p0.27 ==> display on/off

  gpio_pin_configure(dev, SCANNER, GPIO_DIR_OUT); //p0.21 ==> Scanner trigger

  gpio_pin_configure(dev, LED_GRN, GPIO_DIR_OUT); //p0.16 ==> LED1
  gpio_pin_configure(dev, 17, GPIO_DIR_OUT); //p0.17 ==> LED2

  gpio_pin_configure(dev, 19, GPIO_DIR_OUT); //p0.19 ==> Buzzer

  gpio_pin_configure(dev, 12, GPIO_DIR_OUT); //p0.12 ==> vibrator trigger

  gpio_pin_write(dev, DISPLAY, 1); //p0.27 ==> display on/off
  gpio_pin_write(dev, SCANNER, 1);
  gpio_pin_write(dev, 16, 1); //p0.03 == LED2
  gpio_pin_write(dev, 17, 1); //p0.03 == LED2
  gpio_pin_write(dev, 19, 0); //p0.19 == Buzzer
  gpio_pin_write(dev, 12, 0); //p0.12 ==> vibrator trigger
}


void vibrate(bool toggle){
    gpio_pin_write(dev, 12, toggle);
};


void init_elva_i2c(void){
        i2c_dev = device_get_binding(I2C_DEV);
        i2c_configure(i2c_dev, I2C_SPEED_SET(I2C_SPEED_STANDARD));
        //scan for i2c devices.
        k_sleep(500);
	printk("Starting i2c scanner...\n");

	if (!i2c_dev) {
		printk("I2C: Device driver not found.\n");
		return;
	}
	
	uint8_t error = 0u;
	
	for (u8_t i = 4; i <= 0x77; i++) {

		struct i2c_msg msgs[1];
		u8_t dst = 1;
		/* Send the address to read from */
		msgs[0].buf = &dst;
		msgs[0].len = 1U;
		msgs[0].flags = I2C_MSG_WRITE | I2C_MSG_STOP;
		error = i2c_transfer(i2c_dev, &msgs[0], 1, i);
		if (error == 0) {
                  printk("0x%2x FOUND\n", i);
		} else {

		}
	}
}
