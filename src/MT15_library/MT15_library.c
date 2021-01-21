/*

* Copyright (c) 2012-2014 Wind River Systems, Inc.

*

* SPDX-License-Identifier: Apache-2.0

*/

#include <device.h>
#include <uart.h>
#include <MT15_library.h>

/*
Send Message through uart
*/
void elva_uart_transmit(struct device *uart_port , uint8_t *message, uint32_t size){
  u8_t i;
  for (i=0 ; i < size ; i++){
    uart_poll_out(uart_port, message[i]);
    }
}


void init_MT15(struct device *uart_port){ 
  elva_uart_transmit(uart_port, "{M VERR}", 8); //firmware version
  elva_uart_transmit(uart_port, "{MR001W2}", 9); //Reading Mode: Trigger mode. LED active LOW
  elva_uart_transmit(uart_port, "{MG015W1,#5B}", 13); //set Preamble byte '['.   
  elva_uart_transmit(uart_port, "{MG017W1,#5D}", 13); //set end byte ']'. 
  elva_uart_transmit(uart_port, "{MT007W0,1}", 11); //time to sleep mode (1 secs) . 
}

