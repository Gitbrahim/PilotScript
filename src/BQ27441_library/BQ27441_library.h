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

#ifndef BQ27441_LIBRARY_H__
#define BQ27441_LIBRARY_H__

#ifdef __cplusplus
extern "C" {
#endif


/*Code starts here. */


//static const char* const I2C_DEV = "I2C_2";
//static struct device *i2c_dev;


int write_command(char *write_bytes[2], int num_bytes);
int read_command(char *command_bytes[2], int num_command_bytes, int num_read_bytes);

///////////////////////////
/* INITIALIZER FUNCTIONS */
///////////////////////////

bool bat_setCapacity(int capcity);

///////////////////////
/* STANDARD COMMANDS */
///////////////////////

int bat_control(void);

int bat_voltage(void);

int bat_FCC(void);

int bat_AverageCurrent(void);

int bat_SOC(void);

int bat_SOCUF(void);

int bat_OpConfig(void);

bool softReset(void);

////////////////////////////
/* EXTENDED DATA COMMANDS */
////////////////////////////

/* Issue a BlockDataControl() command to enable BlockData access. */
bool blockDataControl(void);

/* Issue a DataClass() command to set the data class to be accessed. */
bool blockDataClass(char id);

/* Issue a DataBlock() command to set the data block to be accessed. */
bool blockDataOffset(char offset);

/* Read the current checksum using BlockDataCheckSum(). */
char blockDataChecksum(void);

/* Use BlockData() to read a byte from the loaded extended data. */
char readBlockData(char offset);

/* Use BlockData() to write a byte to an offset of the loaded data. */
bool writeBlockData(char offset, char data);

/* Read all 32 bytes of the loaded extended data and compute a
   checksum based on the values. */
char computeBlockCheckSum(void);

/* Use the BlockDataCheckSum() command to write a checksum value. */
bool writeBlockChecksum(char csum);

// Write a specified number of bytes to extended data specifying a 
// class ID, position offset.
bool writeExtendedData(char classID, char offset, char *data, char len);

//////////////////////////////
//// CONTROL SUBCOMMANDS /////
//////////////////////////////

// Enter configuration mode - set userControl if calling from an Arduino sketch
// and you want control over when to exitConfig
bool enterConfig(bool userControl);

bool exitConfig(bool resim);

int flags(void);

/* Read the control status (char Control) subcommand of control(). */
int status(void);

/**
    Read a 16-bit subcommand() from the BQ27441-G1A's control()
	
	@param function is the subcommand of control() to be read
	@return 16-bit value of the subcommand's contents
*/	
int readControlWord(int function);

/**
    Execute a subcommand() from the BQ27441-G1A's control()
	
	@param function is the subcommand of control() to be executed
	@return true on success
*/
bool executeControlWord(int function);

///////////////////////////
//// PRIVATE FUNCTIONS ////
///////////////////////////

bool sealed(void);
 
/* Seal Battery monitor. */
bool seal(void);

/* Unseal battery monitor. */
bool unseal(void);

/*Code ends here. */

#ifdef __cplusplus
}
#endif

#endif /* BQ27441_LIBRARY_H__ */
