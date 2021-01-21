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

#ifndef DISPLAY_LIBRARY_H__
#define DISPLAY_LIBRARY_H__

#ifdef __cplusplus
extern "C" {
#endif

static struct device *spi_dev;
static struct device *dev;

static const char* const spiName = "SPI_3"; //was "SPI_2" in working demo. I2C was "I2C_3".
//struct device *gpio_port;

void sayHello(void);
void spi_init(void); //(struct device* spi_port);
void dispBegin(void);
void clearDisplay(void);
void printDisplay(u16_t line, u8_t text_input[18], u16_t text_size, u8_t* selected_font, u16_t y, u16_t x, u16_t z);//(struct device* spi_port, struct device *gpio_port, u16_t line, u8_t text_input[18], u16_t text_size, u8_t* selected_font, u16_t y, u16_t x, u16_t z);
void check_pin_locations(void); //DELETE ME

void display_draw_image(u8_t line, u8_t (*image_array)[168][18], int image_hight);

/**/
void display_println(u8_t line, u8_t message[36], u8_t (*font)[336][28], int font_hight, int font_width, int char_width);

void small_print(unsigned int column, unsigned int row, unsigned char message[36]);

void medium_print(unsigned int column, unsigned int row, unsigned char message[36]);

void large_print(unsigned int column, unsigned int row, unsigned char message[36]);

void reverseArray(u8_t arr[], int start, int end); 

//u8_t font8bit[8][95];    //OLD FONT NO LONGER USED                    
//u8_t font16bit[24][190]; //OLD FONT NO LONGER USED 
//u8_t font32bit[48][380]; //OLD FONT NO LONGER USED 
//u8_t backGround[168][18];
extern u8_t icon_cross[168][18];
extern u8_t icon_check[168][18];
extern u8_t icon_pill_bottle[168][18];
extern u8_t icon_round_pill[64][18];
//
//u8_t test_array[20][18];
//
extern u8_t Font8Bit[1092][1];
extern u8_t Font16Bit[2912][2];
extern u8_t Font32Bit[1782][4];



#ifdef __cplusplus
}
#endif

#endif /* DISPLAY_LIBRARY_H__ */
