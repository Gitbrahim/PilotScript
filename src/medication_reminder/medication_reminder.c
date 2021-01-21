/*

* Copyright (c) Practech, Inc.


*/
#include <nrf9160.h>
#include <zephyr.h>
#include <misc/printk.h>
#include <medication_reminder.h>




//Time For Medication Window.
unsigned char tfmwTimeStamp[12];

//Current Time.
unsigned char ctTimeStamp[16]; //hrs : mins : secs
//unsigned char ctUnix[16];  //convert char to N = atoi(&C);
unsigned char ctHrsMins[5];

//Medication
char medABarcode[69];
char medADose[69]; //min to max valuerequired 0-1000
char medAFormat[69];
char medAName[69];
char medAStatus[69];

char medBBarcode[69];
char medBDose[69]; //min to max valuerequired 0-1000
char medBFormat[69];
char medBName[69];
char medBStatus[69];

char medCBarcode[69];
char medCDose[69]; //min to max valuerequired 0-1000
char medCFormat[69];
char medCName[69];
char medCStatus[69];

char medDBarcode[69];
char medDDose[69]; //min to max valuerequired 0-1000
char medDFormat[69];
char medDName[69];
char medDStatus[69];

char medEBarcode[69];
char medEDose[69]; //min to max valuerequired 0-1000
char medEFormat[69];
char medEName[69];
char medEStatus[69];

extern int currentTime = 0;
extern int tfmwEnd = 0; //convert char to int HOW TO: N = atoi(&C);

extern char testPacket[250] = ">1234567890>0987654321>087946132531>50>Pill>ADVIL>Pending>087946132531>50>Pill>ADVIL>Pending>087946132531>50>Pill>ADVIL>Pending>087946132531>50>Pill>ADVIL>Pending>087946132531>50>Pill>ADVIL>Pending>";
/* brief:
    tfmwTimeStamp:  Time for medications window start (time of day in mins ==> hrs*60 + mins). 
    char[10] Unix: Real current time. Used to update current time on elva device.
    tfmwTimeStamp >ctHrsMins >medAbarcode >medAdose >medAformat >medAname >medAstatus .
                          >medBbarcode >medBdose >medBformat >medBname >medBstatus 
                          >medCbarcode >medCdose >medCformat >medCname >medCstatus 
                          >medDbarcode >medDdose >medDformat >medDname >medDstatus 
                          >medEbarcode >medEdose >medEformat >medAname >medEstatus 
*/

int unpack(char package[512]){
    int i = 0;
    int itemIndex = 0;
    char itemBuffer[128]; 
    int n;
    
    /*--clear all registers before unpacking.--*/
    memset(tfmwTimeStamp, 0, sizeof(tfmwTimeStamp));
    memset(ctHrsMins,     0, sizeof(ctHrsMins));

    memset(medABarcode, 0, sizeof(medABarcode));
    memset(medADose,    0, sizeof(medADose));
    memset(medAFormat,  0, sizeof(medAFormat));
    memset(medAName,    0, sizeof(medAName));
    memset(medAStatus,  0, sizeof(medAStatus));

    memset(medBBarcode, 0, sizeof(medBBarcode));
    memset(medBDose,    0, sizeof(medBDose));
    memset(medBFormat,  0, sizeof(medBFormat));
    memset(medBName,    0, sizeof(medBName));
    memset(medBStatus,  0, sizeof(medBStatus));

    memset(medCBarcode, 0, sizeof(medCBarcode));
    memset(medCDose,    0, sizeof(medCDose));
    memset(medCFormat,  0, sizeof(medCFormat));
    memset(medCName,    0, sizeof(medCName));
    memset(medCStatus,  0, sizeof(medCStatus));

    memset(medDBarcode, 0, sizeof(medDBarcode));
    memset(medDDose,    0, sizeof(medDDose));
    memset(medDFormat,  0, sizeof(medDFormat));
    memset(medDName,    0, sizeof(medDName));
    memset(medDStatus,  0, sizeof(medDStatus));

    memset(medEBarcode, 0, sizeof(medEBarcode));
    memset(medEDose,    0, sizeof(medEDose));
    memset(medEFormat,  0, sizeof(medEFormat));
    memset(medEName,    0, sizeof(medEName));
    memset(medEStatus,  0, sizeof(medEStatus));

    for(i ; i <= strlen(package) - 1 ; i++){
        if(package[i] == '>'){ 
            itemIndex++;  //Start depositting into next item.
            n = 0;

            } else {
                switch(itemIndex){ //select location to append string.
                    case 1: //parse TFM.
                        tfmwTimeStamp[n] = package[i];
                        //tfmWindow.timeStamp[n] = package[i];    
                        n++;
                        break;
                    case 2: //parse UNIX.
                        ctHrsMins[n] = package[i];
                        //currentTime.timeStamp[n] = package[i];
                        n++;
                        break;

                    //MEDICATION A:
                    case 3: //parse A barcode
                        medABarcode[n] = package[i];
                        //medA.barcode[n] = package[i];
                        n++;
                        break;
                    case 4: //parse A dose
                        medADose[n] = package[i];
                        //medA.dose[n] = package[i];
                        n++;
                        break;
                    case 5: //parse A format
                        medAFormat[n] = package[i];
                        //medA.format[n] = package[i];
                        n++;
                        break;
                    case 6: //parse A name 
                        medAName[n] = package[i];
                        //medA.name[n] = package[i];
                        n++;
                        break;                   
                    case 7: //parse A status 
                        medAStatus[n] = package[i];
                        //medA.status[n] = package[i];
                        n++;
                        break;

                    //MEDICATION B:
                    case 8: //parse B barcode
                        medBBarcode[n] = package[i];
                        //medB.barcode[n] = package[i];
                        n++;
                        break;
                    case 9: //parse B dose
                        medBDose[n] = package[i];
                        //medB.dose[n] = package[i];
                        n++;
                        break;
                    case 10: //parse B format
                        medBFormat[n] = package[i];
                        //medB.format[n] = package[i];
                        n++;
                        break;
                    case 11: //parse B name 
                        medBName[n] = package[i];
                        //medB.name[n] = package[i];
                        n++;
                        break;                   
                    case 12: //parse B status 
                        medBStatus[n] = package[i];
                        //medB.status[n] = package[i];
                        n++;
                        break;

                    //MEDICATION C:
                    case 13: //parse C barcode
                        medCBarcode[n] = package[i];
                        //medC.barcode[n] = package[i];
                        n++;
                        break;
                    case 14: //parse C dose
                        medCDose[n] = package[i];
                        //medC.dose[n] = package[i];
                        n++;
                        break;
                    case 15: //parse C format
                        medCFormat[n] = package[i];
                        //medC.format[n] = package[i];
                        n++;
                        break;
                    case 16: //parse C name 
                        medCName[n] = package[i];
                        //medC.name[n] = package[i];
                        n++;
                        break;                   
                    case 17: //parse C status 
                        medCStatus[n] = package[i];
                        //medC.status[n] = package[i];
                        n++;
                        break;

                    //MEDICATION D:
                    case 18: //parse D barcode
                        medDBarcode[n] = package[i];
                        //medD.barcode[n] = package[i];
                        n++;
                        break;
                    case 19: //parse D dose
                        medDDose[n] = package[i];
                        //medD.dose[n] = package[i];
                        n++;
                        break;
                    case 20: //parse D format
                        medDFormat[n] = package[i];
                        //medD.format[n] = package[i];
                        n++;
                        break;
                    case 21: //parse D name 
                        medDName[n] = package[i];
                        //medD.name[n] = package[i];
                        n++;
                        break;                   
                    case 22: //parse D status 
                        medDStatus[n] = package[i];
                        //medD.status[n] = package[i];
                        n++;
                        break;

                    //MEDICATION E:
                    case 23: //parse E barcode
                        medEBarcode[n] = package[i];
                        //medE.barcode[n] = package[i];
                        n++;
                        break;
                    case 24: //parse E dose
                        medEDose[n] = package[i];
                        //medE.dose[n] = package[i];
                        n++;
                        break;
                    case 25: //parse E format
                        medEFormat[n] = package[i];
                        //medE.format[n] = package[i];
                        n++;
                        break;
                    case 26: //parse E name 
                        medEName[n] = package[i];
                        //medE.name[n] = package[i];
                        n++;
                        break;                   
                    case 27: //parse E status 
                        medEStatus[n] = package[i];
                        //medE.status[n] = package[i];
                        n++;
                        break;

                    case 28: //Parsing done
                        printk("Done parsing med reminder package.\n");
                        break;

                    default:
                        //BAIL ON FUNCTION GIVE ERROR REPORT
                        printk("ERROR: Data not usable. loop end.\n");
                        return 1;
                        //itemBuffer[i] = package[i];
                        break;
                    }
                    //itemBuffer[i] = package[i];
                    //tfmWindow.timeStamp[i] = package[i];
                    }
        }//loop end
////        printk("Barcode  A : %s\n", medABarcode);
////        printk("Dose     A : %s\n", medADose);
////        printk("Format   A : %s\n", medAFormat);
////        printk("Name     A : %s\n", medAName);
////        printk("Status   A : %s\n", medAStatus);
////
////        printk("Barcode  B : %s\n", medBBarcode);
////        printk("Dose     B : %s\n", medBDose);
////        printk("Format   B : %s\n", medBFormat);
////        printk("Name     B : %s\n", medBName);
////        printk("Status   B : %s\n", medBStatus);
////
////        printk("Barcode  C : %s\n", medCBarcode);
////        printk("Dose     C : %s\n", medCDose);
////        printk("Format   C : %s\n", medCFormat);
////        printk("Name     C : %s\n", medCName);
////        printk("Status   C : %s\n", medCStatus);
////
////        printk("Barcode  D : %s\n", medDBarcode);
////        printk("Dose     D : %s\n", medDDose);
////        printk("Format   D : %s\n", medDFormat);
////        printk("Name     D : %s\n", medDName);
////        printk("Status   D : %s\n", medDStatus);
////
////        printk("Barcode  E : %s\n", medEBarcode);
////        printk("Dose     E : %s\n", medEDose);
////        printk("Format   E : %s\n", medEFormat);
////        printk("Name     E : %s\n", medEName);
////        printk("Status   E : %s\n", medEStatus);

        return 0;
    }

int med_count;

int evaluate_med_list(){
    
    med_count = 0;
    //Create Med A:
    if(strcmp(medABarcode , "empty")   != 0 &&
       strcmp(medADose    , "empty")   != 0 &&
       strcmp(medAFormat  , "empty")   != 0 &&
       strcmp(medAName    , "empty")   != 0 &&
       strcmp(medAStatus  , "pending") == 0 ){ 
        med_count++;
        } else {
            //Do Nothing
            };
    
    //Create Med B
    if(strcmp(medBBarcode , "empty")   != 0 &&
       strcmp(medBDose    , "empty")   != 0 &&
       strcmp(medBFormat  , "empty")   != 0 &&
       strcmp(medBName    , "empty")   != 0 &&
       strcmp(medBStatus  , "pending") == 0 ){ 
        med_count++;
        } else {
            //Do Nothing
            };

    //Create Med C
    if(strcmp(medCBarcode , "empty")   != 0 &&
       strcmp(medCDose    , "empty")   != 0 &&
       strcmp(medCFormat  , "empty")   != 0 &&
       strcmp(medCName    , "empty")   != 0 &&
       strcmp(medCStatus  , "pending") == 0 ){ 
        med_count++;
        } else {
            //Do Nothing
            };

    //Create Med D
    if(strcmp(medDBarcode , "empty")   != 0 &&
       strcmp(medDDose    , "empty")   != 0 &&
       strcmp(medDFormat  , "empty")   != 0 &&
       strcmp(medDName    , "empty")   != 0 &&
       strcmp(medDStatus  , "pending") == 0 ){ 
        med_count++;
        } else {
            //Do Nothing
            };

    //Create Med E
    if(strcmp(medEBarcode , "empty")   != 0 &&
       strcmp(medEDose    , "empty")   != 0 &&
       strcmp(medEFormat  , "empty")   != 0 &&
       strcmp(medEName    , "empty")   != 0 &&
       strcmp(medEStatus  , "pending") == 0 ){ 
        med_count++;
        } else {
            //Do Nothing
            };

    return 0;
    };

/* brief:
    Format response message to inform server about device results and status.
    char[10] TFM:  Time for medications window end time. 
    char[10] Unix: Real current time. Used to update current time on elva device.
    TFM >UNIX >medAbarcode >medAdose >medAformat >medAname >medAstatus .
              >medBbarcode >medBdose >medBformat >medBname >medBstatus 
              >medCbarcode >medCdose >medCformat >medCname >medCstatus 
              >medDbarcode >medDdose >medDformat >medDname >medDstatus 
              >medEbarcode >medEdose >medEformat >medAname >medEstatus 
*/
//int pack(package[1000]){
//    strncat(&C, &XX, strlen(XX));
//    };


