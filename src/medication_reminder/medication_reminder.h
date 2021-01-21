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

#ifndef MEDICATION_REMINDER_H__
#define MEDICATION_REMINDER_H__

#ifdef __cplusplus
extern "C" {
#endif

/* @brief TFM:  Time for medications window end time.
   start: start of 'time for medication' window.
   end:   end of TFM window.
*/ 

//Time For Medication Window.
extern unsigned char tfmwTimeStamp[12];

//Current Time.
extern unsigned char ctTimeStamp[16]; //hrs : mins : secs
//extern unsigned char ctUnix[16];  //convert char to N = atoi(&C);
extern unsigned char ctHrsMins[5]; //string from server representing current minute in the day. (hours*60) + minutes. 

//Medication
extern char medABarcode[69];
extern char medADose[69]; //min to max valuerequired 0-1000
extern char medAFormat[69];
extern char medAName[69];
extern char medAStatus[69];

extern char medBBarcode[69];
extern char medBDose[69]; //min to max valuerequired 0-1000
extern char medBFormat[69];
extern char medBName[69];
extern char medBStatus[69];

extern char medCBarcode[69];
extern char medCDose[69]; //min to max valuerequired 0-1000
extern char medCFormat[69];
extern char medCName[69];
extern char medCStatus[69];

extern char medDBarcode[69];
extern char medDDose[69]; //min to max valuerequired 0-1000
extern char medDFormat[69];
extern char medDName[69];
extern char medDStatus[69];

extern char medEBarcode[69];
extern char medEDose[69]; //min to max valuerequired 0-1000
extern char medEFormat[69];
extern char medEName[69];
extern char medEStatus[69];

extern char testPacket[250];

extern int currentTime;
extern int tfmwEnd;


/* brief:
    function to parse properly formatted string into data required to setup elva medication reminder feature.
    char[10] TFM:  Time for medications window start time. 
    char[10] Unix: Real current time. Used to update current time on elva device.
    TFM >UNIX >medAbarcode >medAdose >medAformat >medAname >medAstatus .
              >medBbarcode >medBdose >medBformat >medBname >medBstatus 
              >medCbarcode >medCdose >medCformat >medCname >medCstatus 
              >medDbarcode >medDdose >medDformat >medDname >medDstatus 
              >medEbarcode >medEdose >medEformat >medAname >medEstatus 
*/
int unpack(char package[1000]);


extern int med_count;

int evaluate_med_list();


#ifdef __cplusplus
}
#endif

#endif /* MEDICATION_REMINDER_H__ */
