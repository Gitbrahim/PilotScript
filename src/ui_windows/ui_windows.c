/*

* Copyright (c) Practech, Inc. 2020


*/
#include <nrf9160.h>
#include <zephyr.h>
#include <misc/printk.h>
#include <ui_windows.h>
#include <LS013B7DH05_library.h>
#include "elva_initializer.h"
#include <gpio.h>;

void WINDOWS_HELLO(){
    printk("UI_WINDOWS READY\n");
    };



int home(){
    //Maintain connection.
    //Manage 'Display ON/OFF' button.
    clearDisplay();
    small_print(3, 100, "HOME");
    return 0;
    };


      
