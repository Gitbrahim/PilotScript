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
#include <BQ27441_library.h>
#include "elva_initializer.h"

/*Code starts here. */

/*definitions. */
#define I2C_TIMEOUT 2000
#define ADDRESS 0x55  //BQ27441 address.
bool SEAL_FLAG = false;
bool USER_CONFIG_CONTROL = false;


/*General Constants. */
#define UNSEAL_KEY	0x8000 // Secret code to unseal the BQ27441-G1A
#define DEVICE_ID	0x0421 // Default device ID

/*Standard commands: */
char Control                        = 0x00; //Read Write.
char Temperature                    = 0x02; //Read Write.
char Voltage                        = 0x04; //Read. 
char Flags                          = 0x06; //Read.
char NominalAvailableCapacity       = 0x08; //Read.
char FullAvailableCapacity          = 0x0A; //Read.
char RemainingCapacity              = 0x0C; //Read.
char FullChargeCapacity             = 0x0E; //Read.
char AverageCurrent                 = 0x10; //Read.
char StandbyCurrent                 = 0x12; //Read.
char MaxLoadCurrent                 = 0x14; //Read.
char AveragePower                   = 0x18; //Read.
char StateOfCharge                  = 0x1C; //Read.
char InternalTemperature            = 0x1E; //Read.
char StateOfHealth                  = 0x20; //Read.
char RemainingCapacityUnfiltered    = 0x28; //Read.
char RemainingCapacityFiltered      = 0x2A; //Read.
char FullChargeCapacityUnfiltered   = 0x2C; //Read.
char FullChargeCapacityFiltered     = 0x2E; //Read.
char StateOfChargeUnfiltered        = 0x30; //Read.      
char TrueRemainingCapacity          = 0x6A; //Read.

/*Extended commands: */
char OpConfig           = 0x3A;
char DesignCapacity     = 0x3C;
char DataClass          = 0x3E;
char DataBlock          = 0x3F;
char BlockData          = 0x40;
char BlockDataCheckSum  = 0x60;
char BlockDataControl   = 0x61;
char Reserved           = 0x62;


/* Control Sub-commands. */
// Issuing a Control() command requires a subsequent 2-byte subcommand. These
// additional bytes specify the particular control function desired. The 
// Control() command allows the system to control specific features of the fuel
// gauge during normal operation and additional features when the device is in 
// different access modes.
#define CONTROL_STATUS		0x00
#define CONTROL_DEVICE_TYPE	0x01
#define CONTROL_FW_VERSION	0x02
#define CONTROL_DM_CODE		0x04
#define CONTROL_PREV_MACWRITE	0x07
#define CONTROL_CHEM_ID		0x08
#define CONTROL_BAT_INSERT	0x0C
#define CONTROL_BAT_REMOVE	0x0D
#define CONTROL_SET_HIBERNATE	0x11
#define CONTROL_CLEAR_HIBERNATE	0x12
#define CONTROL_SET_CFGUPDATE	0x13
#define CONTROL_SHUTDOWN_ENABLE	0x1B
#define CONTROL_SHUTDOWN	0x1C
#define CONTROL_SEALED		0x20
#define CONTROL_PULSE_SOC_INT	0x23
#define CONTROL_RESET		0x41
#define CONTROL_SOFT_RESET	0x42
#define CONTROL_EXIT_CFGUPDATE	0x43
#define CONTROL_EXIT_RESIM	0x44


/* Control Status Word - Bit Definitions. */
// Bit positions for the 16-bit data of CONTROL_STATUS.
// CONTROL_STATUS instructs the fuel gauge to return status information to 
// Control() addresses 0x00 and 0x01. The read-only status word contains status
// bits that are set or cleared either automatically as conditions warrant or
// through using specified subcommands.
#define STATUS_SHUTDOWNEN	(1<<15)
#define STATUS_WDRESET		(1<<14)
#define STATUS_SS		(1<<13)
#define STATUS_CALMODE		(1<<12)
#define STATUS_CCA		(1<<11)
#define STATUS_BCA		(1<<10)
#define STATUS_QMAX_UP		(1<<9)
#define STATUS_RES_UP		(1<<8)
#define STATUS_INITCOMP		(1<<7)
#define STATUS_HIBERNATE	(1<<6)
#define STATUS_SLEEP		(1<<4)
#define STATUS_LDMD		(1<<3)
#define STATUS_RUP_DIS		(1<<2)
#define STATUS_VOK		(1<<1)


/* Flag Command - Bit Definitions. */
// Bit positions for the 16-bit data of Flags()
// This read-word function returns the contents of the fuel gauging status
// register, depicting the current operating status.
#define FLAG_OT		(1<<15)
#define FLAG_UT		(1<<14)
#define FLAG_FC		(1<<9)
#define FLAG_CHG	(1<<8)
#define FLAG_OCVTAKEN	(1<<7)
#define FLAG_ITPOR	(1<<5)
#define FLAG_CFGUPMODE	(1<<4)
#define FLAG_BAT_DET	(1<<3)
#define FLAG_SOC1	(1<<2)
#define FLAG_SOCF	(1<<1)
#define FLAG_DSG	(1<<0)

////////////////////////////////////////
// Configuration Class, Subclass ID's //
////////////////////////////////////////
// To access a subclass of the extended data, set the DataClass() function
// with one of these values.
// Configuration Classes
#define ID_SAFETY		2   // Safety
#define ID_CHG_TERMINATION	36  // Charge Termination
#define ID_CONFIG_DATA		48  // Data
#define ID_DISCHARGE		49  // Discharge
#define ID_REGISTERS		64  // Registers
#define ID_POWER		68  // Power
// Gas Gauging Classes
#define ID_IT_CFG		80  // IT Cfg
#define ID_CURRENT_THRESH	81  // Current Thresholds
#define ID_STATE		82  // State
// Ra Tables Classes
#define ID_R_A_RAM		89  // R_a RAM
// Calibration Classes
#define ID_CALIB_DATA		104 // Data
#define ID_CC_CAL		105 // CC Cal
#define ID_CURRENT		107 // Current
// Security Classes
#define ID_CODES		112 // Codes

/*global variables. */
char write_byte = 9;
char read_byte  = 9; 
uint8_t error = 0u;

int write_command(char *send_bytes[3], int num_bytes){ 
    i2c_dev = device_get_binding(I2C_DEV);
    i2c_write(i2c_dev, &send_bytes[0], num_bytes, ADDRESS);
    if(error == 0){
        return error; //i2c write success.
        } else {
              return 1; //i2c write fail.
              }
    }


int read_command(char *command_bytes[2], int num_command_bytes, int num_read_bytes){
    char incoming_bytes[2] = {NULL, NULL};
    int read_data = NULL;
    //send command byte(s) to battery BQ27441.
    error = write_command(&command_bytes[0], num_command_bytes);
    if(error == 0){
        //read incoming byte(s) from BQ27441.
        error = i2c_read(i2c_dev, &incoming_bytes[0], num_read_bytes, ADDRESS); //read bytes
        if(error == 0){
            //Return read data as interger (16bit) variable.
            read_data = (incoming_bytes[1] << 8);
            read_data = (incoming_bytes[0] | read_data);
            return read_data;
            //return read_byte; //success.

            //Read failure report.
            } else {
                  return 0; //i2c read failed.
                  }
        //write failure report
        } else {
              return 1; //i2c write failed.
              }
    }


///////////////////////////
/* INITIALIZER FUNCTIONS */
///////////////////////////

bool bat_setCapacity(int capacity){
    // Write to STATE subclass (82) of BQ27441 extended memory.
    // Offset 0x0A (10)
    // Design capacity is a 2-byte piece of data - MSB first
    unsigned char capMSB = capacity >> 8; //capture left byte from capacity.
    unsigned char capLSB = capacity & 0x00FF; //capture right side of capacity.
    unsigned char capacityData[2] = {capMSB, capLSB};
    return writeExtendedData(ID_STATE, 10, capacityData, 2);
    }
  


///////////////////////
/* STANDARD COMMANDS */
///////////////////////

int bat_control(void){
    char byte[2] = {0x00, 0x00};   
    return read_command(&byte, 1, 1);
    }


int bat_voltage(void){
    return read_command(&Voltage, 1, 2);
    }

int bat_FCC(void){
    return read_command(&FullChargeCapacity, 1, 2);
    }

int bat_SOC(void){
    return read_command(&StateOfCharge, 1, 1);
    }


int bat_SOCUF(void){
    return read_command(&StateOfChargeUnfiltered, 1, 1);
    }
        
int bat_AverageCurrent(void){    
    return read_command(&AverageCurrent, 1, 1);
    }
       

int bat_OpConfig(void){
    return read_command(&OpConfig, 1, 1);
    }

// Issue a soft-reset to the BQ27441-G1A
bool softReset(void){
    return executeControlWord(CONTROL_SOFT_RESET);
    }

////////////////////////////
/* EXTENDED DATA COMMANDS */
////////////////////////////

/* Issue a BlockDataControl() command to enable BlockData access. */
bool blockDataControl(void){
    //char sub_address = BlockDataControl; 
    char enable_byte = 0x00;         
    char bytes[2] = {BlockDataControl, enable_byte};
    return write_command(&bytes[0], 2);
    }

/* Issue a DataClass() command to set the data class to be accessed. */
bool blockDataClass(char id){
    char bytes[2] = {DataBlock, id};
    return write_command(&bytes[0], 2);
    }

/* Issue a DataBlock() command to set the data block to be accessed. */
bool blockDataOffset(char offset){
    char bytes[2] = {DataBlock, offset};
    return write_command(&bytes[0], 2);
    }

/* Read the current checksum using BlockDataCheckSum(). */
char blockDataChecksum(void){
    char csum;
    csum = read_command(&BlockDataCheckSum, 1, 1);
    return csum;
    }

/* Use BlockData() to read a byte from the loaded extended data. */
char readBlockData(char offset){
    char ret;
    char address = offset + BlockData;
    ret = read_command(&address, 1, 1);
    }

/* Use BlockData() to write a byte to an offset of the loaded data. */
bool writeBlockData(char offset, char data){
    char address = offset + BlockData;
    char bytes[2] = {address, data};
    return write_command(&bytes, 2);
    }

/* Read all 32 bytes of the loaded extended data and compute a
   checksum based on the values. */
char computeBlockCheckSum(void){
    char data[32];
    //char bytes[2] = {BlockData, data}; 
    //I2C read wrtire.
    error = write_command(&BlockData, 1);
    if(error == 0){
        //read incoming byte(s) from BQ27441.
        i2c_read(i2c_dev, &data[0], 32, ADDRESS); //read bytes
        }

    char csum = 0;
    for (int i=0; i<32; i++){
        csum += data[i];
        }
    csum = 255 - csum;
    return csum;
    }

/* Use the BlockDataCheckSum() command to write a checksum value. */
bool writeBlockChecksum(char csum){
    char bytes[2] = {BlockDataCheckSum, csum};
    return write_command(&bytes[0], 2); 
    }

// Write a specified number of bytes to extended data specifying a 
// class ID, position offset.
bool writeExtendedData(char classID, char offset, char *data, char len){
    if(len > 32) return false;
    if(!USER_CONFIG_CONTROL) enterConfig(false);
    if(!blockDataControl()) // enable block data memory control
        return false; // Return false if enable fails
    if(!blockDataClass(classID)) // Write class ID using DataBlockClass()
        return false;

    blockDataOffset(offset / 32); // Write 32-bit block offset (usually 0)
    computeBlockCheckSum(); // Compute checksum going in
    uint8_t oldCsum = blockDataChecksum();

    // Write data bytes:
    for (int i = 0; i < len; i++){
    	// Write to offset, mod 32 if offset is greater than 32
    	// The blockDataOffset above sets the 32-bit block
    	writeBlockData((offset % 32) + i, data[i]);
        }

    // Write new checksum using BlockDataChecksum (0x60)
    uint8_t newCsum = computeBlockCheckSum(); // Compute the new checksum
    writeBlockChecksum(newCsum);
    if (!USER_CONFIG_CONTROL) exitConfig(true);
    return true;
    }

 
//////////////////////////////
//// CONTROL SUBCOMMANDS /////
//////////////////////////////

// Enter configuration mode - set userControl if calling from an Arduino sketch
// and you want control over when to exitConfig
bool enterConfig(bool userControl){
    if(userControl) USER_CONFIG_CONTROL = true;
    if (sealed()){
        SEAL_FLAG = true;
        unseal(); //must be unsealed beforemaking changes.
        }
    if(executeControlWord(CONTROL_SET_CFGUPDATE)){
        int timeout = I2C_TIMEOUT;
	while ((timeout--) && (!(status() & FLAG_CFGUPMODE)))
            k_sleep(1);
	if (timeout > 0)
            return true;
        }
    return false;
    }
 
// Exit configuration mode with the option to perform a resimulation
bool exitConfig(bool resim){
    // There are two methods for exiting config mode:
    //    1. Execute the EXIT_CFGUPDATE command
    //    2. Execute the SOFT_RESET command
    // EXIT_CFGUPDATE exits config mode _without_ an OCV (open-circuit voltage)
    // measurement, and without resimulating to update unfiltered-SoC and SoC.
    // If a new OCV measurement or resimulation is desired, SOFT_RESET or
    // EXIT_RESIM should be used to exit config mode.
    if (resim){
        if (softReset()){
            int timeout = I2C_TIMEOUT;
            while ((timeout--) && ((flags() & FLAG_CFGUPMODE))) k_sleep(1);
                if (timeout > 0){
                    if (SEAL_FLAG) seal(); // Seal back up if we IC was sealed coming in
                    return true;
                    }
            }
	return false;
	} else {
            return executeControlWord(CONTROL_EXIT_CFGUPDATE);
            }	
    }

// Read the flags() command
int flags(void){
    return read_command(&Flags, 1, 2);
}

/* Read the control status (char Control) subcommand of control(). */
int status(void){
    return readControlWord(&Control);
    }

int readControlWord(int function){
    char subCommandMSB = (function >> 8);
    char subCommandLSB = (function & 0x00FF);
    char command[3] = {0, subCommandLSB, subCommandMSB};
    char data[3] = {0, 0, 0};
    int ret = 0;
    int error = 0;
    char msb;
    int lsb = 0;
    write_command(&command[0], 3);
    ret = read_command(0x00, 1, 2);
    if(ret){
        msb = ret;
        lsb = (ret << 8);
        ret = lsb | msb;
        return ret;
        }
    return 0;
    } 

// Execute a subcommand() from the BQ27441-G1A's control()
bool executeControlWord(int function){
    char subCommandMSB = (function >> 8);
    char subCommandLSB = (function & 0x00FF);
    char command[3] = {0x00, subCommandLSB, subCommandMSB};
    char data[2] = {0, 0};
	
    if(write_command(&command[0], 3)){
        return true;
    	} else {
            return false;
            }
    }

///////////////////////////
//// PRIVATE FUNCTIONS ////
///////////////////////////

bool sealed(void){
    int stat = status();
    return stat & STATUS_SS;
    }
 
/* Seal Battery monitor. */
bool seal(void){
    return readControlWord(CONTROL_SEALED);
    }

/* Unseal battery monitor. */
bool unseal(void){
    // To unseal the BQ27441, write the key to the control
    // command. Then immediately write the same key to control again.
    if (readControlWord(UNSEAL_KEY)){
        return readControlWord(UNSEAL_KEY);
        }
    return false;
    }

/*  _deviceAddress(BQ72441_I2C_ADDRESS), _sealFlag(false), _userConfigControl(false)
// Write a specified number of bytes to extended data specifying a 
// class ID, position offset.
bool BQ27441::writeExtendedData(uint8_t classID, uint8_t offset, uint8_t * data, uint8_t len)
{
	if (len > 32)
		return false;
	
	if (!_userConfigControl) enterConfig(false);
	
	if (!blockDataControl()) // // enable block data memory control
		return false; // Return false if enable fails
	if (!blockDataClass(classID)) // Write class ID using DataBlockClass()
		return false;
	
	blockDataOffset(offset / 32); // Write 32-bit block offset (usually 0)
	computeBlockChecksum(); // Compute checksum going in
	uint8_t oldCsum = blockDataChecksum();

	// Write data bytes:
	for (int i = 0; i < len; i++)
	{
		// Write to offset, mod 32 if offset is greater than 32
		// The blockDataOffset above sets the 32-bit block
		writeBlockData((offset % 32) + i, data[i]);
	}
	
	// Write new checksum using BlockDataChecksum (0x60)
	uint8_t newCsum = computeBlockChecksum(); // Compute the new checksum
	writeBlockChecksum(newCsum);

	if (!_userConfigControl) exitConfig();
	
	return true;
} */












