/*
 * Copyright (c) 2018 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
 */

#include <zephyr.h>
#include <stdio.h>
#include <uart.h>
#include <string.h>
#include "LS013B7DH05_library.h"
#include "elva_initializer.h"
#include "BQ27441_library.h"
#include <medication_reminder.h>
#include <MT15_library.h>
#include <gpio.h>
#include <i2c.h>

#include "ui_windows.h"

/* MQTT includes. */
#include <net/mqtt.h>
#include <net/socket.h>
#include <lte_lc.h>
//#include "certificates.h" ==> NOTE: TLS and some other network features are disabled in prj.conf file.


/* Buffers for MQTT client. */
static u8_t rx_buffer[1024]; //[CONFIG_MQTT_MESSAGE_BUFFER_SIZE];
static u8_t tx_buffer[1024]; //[CONFIG_MQTT_MESSAGE_BUFFER_SIZE];
static u8_t payload_buf[1024]; //[CONFIG_MQTT_PAYLOAD_BUFFER_SIZE];

/* The mqtt client struct */
static struct mqtt_client client;

/* MQTT Broker details. */
static struct sockaddr_storage broker;

/* Connected flag */
static bool connected;

/* File descriptor */
static struct pollfd fds;

/*initial connection after restart.*/
bool restart = true;

/*--Device states used for actions and UI.--*/
enum device_conditions {
    fatal_error, 
    disconnected, 
    standby,
    time_for_med,
    wrong_med,
    med_directions,
    med_taken, 
    score_board
    }dev_condition;

bool update_display = true;
bool publish_message = false;
char message_buff[1024];
bool skip = false;
bool alarm = false;

//battery condition viariables
char batVoltage[10];
int  batLife_i;
float batLife_f;
char batLife_c[10];
char batFcc[10];
char batSocuf[10];
char batSoc[10];
char batOpConfig[10];
char avgmA[10];

char minsCount[18];


/*--Setup UART callback and response to special UART inputs and outputs.--*/
static char uart_buf[256];
static char temp_buff[256];
static char barcode_buffer[256];

char correct_barcode_buffer[256]; //right_barcode_buffer
char alarm_code[256];


static int count = 0;
static char barcode_incoming = 'A';






char medsTable[5][5][32]; //32 chars per item
/* ____________________________________________________________
  |medBbarcode | medBdose | medBformat | medBname | medBstatus |
  |medBbarcode | medBdose | medBformat | medBname | medBstatus |
  |medCbarcode | medCdose | medCformat | medCname | medCstatus |
  |medDbarcode | medDdose | medDformat | medDname | medDstatus |
  |medEbarcode | medEdose | medEformat | medAname | medEstatus |
  |____________|__________|____________|__________|____________|*/

/*--Brief: 
    returns number of medication with matching barcode.
    0 : Scanned barcode does not match anything medicaiton barcode.
    1 : Scanned barcode (barcode_buffer) matcheed medABarcode.
    2 : Scanned barcode (barcode_buffer) matcheed medBBarcode.
    3 : Etc...
--*/



void uart_cb(struct device *x){

    uart_irq_update(x);
    int data_length = 0;

    if(uart_irq_rx_ready(x)){
        data_length = uart_fifo_read(x, uart_buf, sizeof(uart_buf));
        uart_buf[data_length] = 0;
        }

    printk("UART buffer: %s\n", uart_buf);

    if(uart_buf[0]/*last incoming byte*/ == '['){
        barcode_incoming = 'B'; //Start appending incoming barcode message.
        } else if(uart_buf[0]/*last incoming byte*/ == ']'){
            barcode_incoming = 'C'; //End appending incoming barcode message.
            }

    if(barcode_incoming == 'B'){ //Appending incoming message.
        memcpy(&temp_buff[count], uart_buf, 1);
        count++;
        } else if(barcode_incoming == 'C'){
            memset(barcode_buffer, NULL, sizeof(barcode_buffer)); //Clear barcode buffer.
            memcpy(&temp_buff[count], uart_buf, 1);
            memcpy(&barcode_buffer[0], &temp_buff[1], count - 1); //Update barcpde_buffer and exclude first '[' and last ']' characters.
            printk("barcode_buffer: %s\n", barcode_buffer);
            count = 0;
            barcode_incoming = 'A';
            memset(temp_buff, NULL, sizeof(temp_buff));//Clear buffer.
            
            //Scanner LED = off.
            //Update display = true.
            update_display = true;
            }

    };





#if defined(CONFIG_BSD_LIBRARY)

/**@brief Recoverable BSD library error. */
void bsd_recoverable_error_handler(uint32_t err)
{
	printk("bsdlib recoverable error: %u\n", err);
}

/**@brief Irrecoverable BSD library error. */
void bsd_irrecoverable_error_handler(uint32_t err)
{
	printk("bsdlib irrecoverable error: %u\n", err);

	__ASSERT_NO_MSG(false);
}

#endif /* defined(CONFIG_BSD_LIBRARY) */

/**@brief Function to print strings without null-termination
 */
static void data_print(u8_t *prefix, u8_t *data, size_t len)
{
	char buf[len + 1];

	memcpy(buf, data, len);
	buf[len] = 0;
	printk("%s%s\n", prefix, buf);
}

/**@brief Function to publish data on the configured topic
 */
static int data_publish(struct mqtt_client *c, enum mqtt_qos qos,
	u8_t *data, size_t len)
{
	struct mqtt_publish_param param;

	param.message.topic.qos = qos;
	param.message.topic.topic.utf8 = CONFIG_MQTT_PUB_TOPIC;
	param.message.topic.topic.size = strlen(CONFIG_MQTT_PUB_TOPIC);
	param.message.payload.data = data;
	param.message.payload.len = len;
	param.message_id = sys_rand32_get();
	param.dup_flag = 0;
	param.retain_flag = 1;

	data_print("Publishing: ", data, len);
	printk("to topic: %s len: %u\n",
		CONFIG_MQTT_PUB_TOPIC,
		(unsigned int)strlen(CONFIG_MQTT_PUB_TOPIC));

	return mqtt_publish(c, &param);
}

/**@brief Function to subscribe to the configured topic
 */
static int subscribe(void)
{
	struct mqtt_topic subscribe_topic = {
		.topic = {
			.utf8 = CONFIG_MQTT_SUB_TOPIC,
			.size = strlen(CONFIG_MQTT_SUB_TOPIC)
		},
		.qos = MQTT_QOS_0_AT_MOST_ONCE
	};

	const struct mqtt_subscription_list subscription_list = {
		.list = &subscribe_topic,
		.list_count = 1,
		.message_id = 1234
	};

	printk("Subscribing to: %s len %u\n", CONFIG_MQTT_SUB_TOPIC,
		(unsigned int)strlen(CONFIG_MQTT_SUB_TOPIC));

	return mqtt_subscribe(&client, &subscription_list);
}

/**@brief Function to read the published payload.
 */
static int publish_get_payload(struct mqtt_client *c, size_t length)
{
	u8_t *buf = payload_buf;
	u8_t *end = buf + length;
        memset(payload_buf, 0, sizeof(payload_buf));

	if (length > sizeof(payload_buf)) {
		return -EMSGSIZE;
	}

	while (buf < end) {
		int ret = mqtt_read_publish_payload(c, buf, end - buf);

		if (ret < 0) {
			int err;

			if (ret != -EAGAIN) {
				return ret;
			}

			printk("mqtt_read_publish_payload: EAGAIN\n");

			err = poll(&fds, 1, K_SECONDS(CONFIG_MQTT_KEEPALIVE));
			if (err > 0 && (fds.revents & POLLIN) == POLLIN) {
				continue;
			} else {
				return -EIO;
			}
		}

		if (ret == 0) {
			return -EIO;
		}

		buf += ret;
	}

	return 0;
}

/**@brief MQTT client event handler
 */
void mqtt_evt_handler(struct mqtt_client *const c,
		      const struct mqtt_evt *evt)
{
	int err;

	switch (evt->type) {
	case MQTT_EVT_CONNACK:
		if (evt->result != 0) {
			printk("MQTT connect failed %d\n", evt->result);
			break;
		}

		connected = true;
		printk("[%s:%d] MQTT client connected!\n", __func__, __LINE__);
		subscribe();
		break;

	case MQTT_EVT_DISCONNECT:
		printk("[%s:%d] MQTT client disconnected %d\n", __func__,
		       __LINE__, evt->result);

		connected = false;
		break;

	case MQTT_EVT_PUBLISH: {
            const struct mqtt_publish_param *p = &evt->param.publish;

	    printk("[%s:%d] MQTT PUBLISH result=%d len=%d\n", __func__,
	           __LINE__, evt->result, p->message.payload.len);
            
	    err = publish_get_payload(c, p->message.payload.len);
	    if (err >= 0) {
			data_print("Received: ", payload_buf,
				p->message.payload.len);

                        if(skip == false){
                            
                            /*--Unpack package.--*/
                            unpack(payload_buf);
                            
                            /*--Sync elva current time every time new package is unpacked.--*/ 
                            currentTime = atoi(ctHrsMins);
                            //
                            if(tfmwTimeStamp != "OOB"){
                                tfmwEnd = atoi(tfmwTimeStamp);
                                tfmwEnd = tfmwEnd + 120;
                                } else {
                                    tfmwEnd = 0;
                                    };
                            } else {
                                skip = false;
                                };
			/* Echo back received data */  
                        // MQTT Publish: 
//                        data_publish(&client, MQTT_QOS_0_AT_MOST_ONCE,
//				"RICK\n", 5);
//                        data_publish(&client, MQTT_QOS_0_AT_MOST_ONCE,
//				payload_buf, p->message.payload.len);
                          
		} else {
			printk("mqtt_read_publish_payload: Failed! %d\n", err);
			printk("Disconnecting MQTT client...\n");

			err = mqtt_disconnect(c);
			if (err) {
                            printk("Could not disconnect: %d\n", err);
                            }
		}
	} break;

	case MQTT_EVT_PUBACK:
		if (evt->result != 0) {
			printk("MQTT PUBACK error %d\n", evt->result);
			break;
		}

		printk("[%s:%d] PUBACK packet id: %u\n", __func__, __LINE__,
				evt->param.puback.message_id);
		break;

	case MQTT_EVT_SUBACK:
		if (evt->result != 0) {
			printk("MQTT SUBACK error %d\n", evt->result);
			break;
		}

		printk("[%s:%d] SUBACK packet id: %u\n", __func__, __LINE__,
				evt->param.suback.message_id);
		break;

	default:
		printk("[%s:%d] default: %d\n", __func__, __LINE__,
				evt->type);
		break;
	}
}

/**@brief Resolves the configured hostname and
 * initializes the MQTT broker structure
 */
static void broker_init(void)
{
	int err;
	struct addrinfo *result;
	struct addrinfo *addr;
	struct addrinfo hints = {
		.ai_family = AF_INET,
		.ai_socktype = SOCK_STREAM
	};

	err = getaddrinfo(CONFIG_MQTT_BROKER_HOSTNAME, NULL, &hints, &result);
	if (err) {
		printk("ERROR: getaddrinfo failed %d\n", err);

		return;
	}

	addr = result;
	err = -ENOENT;

	/* Look for address of the broker. */
	while (addr != NULL) {
		/* IPv4 Address. */
		if (addr->ai_addrlen == sizeof(struct sockaddr_in)) {
			struct sockaddr_in *broker4 =
				((struct sockaddr_in *)&broker);
			char ipv4_addr[NET_IPV4_ADDR_LEN];

			broker4->sin_addr.s_addr =
				((struct sockaddr_in *)addr->ai_addr)
				->sin_addr.s_addr;
			broker4->sin_family = AF_INET;
			broker4->sin_port = htons(CONFIG_MQTT_BROKER_PORT);

			inet_ntop(AF_INET, &broker4->sin_addr.s_addr,
				  ipv4_addr, sizeof(ipv4_addr));
			printk("IPv4 Address found %s\n", ipv4_addr);

			break;
		} else {
			printk("ai_addrlen = %u should be %u or %u\n",
				(unsigned int)addr->ai_addrlen,
				(unsigned int)sizeof(struct sockaddr_in),
				(unsigned int)sizeof(struct sockaddr_in6));
		}

		addr = addr->ai_next;
		break;
	}

	/* Free the address. */
	freeaddrinfo(result);
}

/**@brief Initialize the MQTT client structure
 */
static void client_init(struct mqtt_client *client)
{
	mqtt_client_init(client);

	broker_init();

	/* MQTT client configuration */
	client->broker = &broker;
	client->evt_cb = mqtt_evt_handler;
	client->client_id.utf8 = (u8_t *)CONFIG_MQTT_CLIENT_ID;
	client->client_id.size = strlen(CONFIG_MQTT_CLIENT_ID);
	client->password = NULL;
	client->user_name = NULL;
	client->protocol_version = MQTT_VERSION_3_1_1;

	/* MQTT buffers configuration */
	client->rx_buf = rx_buffer;
	client->rx_buf_size = sizeof(rx_buffer);
	client->tx_buf = tx_buffer;
	client->tx_buf_size = sizeof(tx_buffer);

	/* MQTT transport configuration */
	client->transport.type = MQTT_TRANSPORT_NON_SECURE;
}

/**@brief Initialize the file descriptor structure used by poll.
 */
static int fds_init(struct mqtt_client *c)
{
	if (c->transport.type == MQTT_TRANSPORT_NON_SECURE) {
		fds.fd = c->transport.tcp.sock;
	} else {
#if defined(CONFIG_MQTT_LIB_TLS)
		fds.fd = c->transport.tls.sock;
#else
		return -ENOTSUP;
#endif
	}

	fds.events = POLLIN;

	return 0;
}

/**@brief Configures modem to provide LTE link. Blocks until link is
 * successfully established.
 */
static void modem_configure(void)
{

#if defined(CONFIG_LTE_LINK_CONTROL)
	if (IS_ENABLED(CONFIG_LTE_AUTO_INIT_AND_CONNECT)) {
		/* Do nothing, modem is already turned on
		 * and connected.
		 */
	} else {
		int err;

		printk("LTE Link Connecting ...\n");
		err = lte_lc_init_and_connect();
		__ASSERT(err == 0, "LTE link could not be established.");
		printk("LTE Link Connected!\n");
	}
#endif
}

bool updateSub = true;

/* Setup Timers. START *////////////////////////////////////////////////////////////////////

#define TIMER_INTERVAL_SUB CONFIG_MQTT_KEEPALIVE //Execute every 10 seconds, primarily used for MQTT SUB.
#define TIMER_INTERVAL_SEC 1 //Execute every 1 seconds.
#define TIMER_INTERVAL_MIN 60 //Execute every 60 seconds.

int err;

struct k_timer time_sec;
struct k_timer time_min;
struct k_timer time_sub;

int tickSecs = 0;
int tickMins = 0;
int tickSubs = 0;
char clock[10];

void expiry_function_sub(struct k_timer *timer_id){

    tickSubs++;

    /*--Toggle every ten seconds to update SUB and ctHrsMins from main loop.--*/
    if(connected == true){
        updateSub = true;
        } else {
            updateSub = false;
            }

    printk("subscribe counter: %d --- CONN: %d --- SUB: %d\n", tickSubs, connected, updateSub); 
    }

int n;
bool vibSwitch = false;
void expiry_function_second(struct k_timer *timer_id){
    tickSecs++;
    
    //Set Vibration to HIGH
    if(alarm == true && tickSecs % 6 == 0){
        vibSwitch = true;
        vibrate(true);
        }
    //Set vibration to LOW
    if(vibSwitch == true && tickSecs % 6 == 1){
        vibrate(false);
        }
     
    }




void expiry_function_minute(struct k_timer *timer_id){
    tickMins++;
    
    /*--Update clock time.--*/
    currentTime++;
    printk("currentTime: %d\n", currentTime);
    printk("minutes counter: %d\n", tickMins); 

    /*--Toggle every minute to update display.--*/
    update_display = true; //to update watch time 00:00
    
//    if(connected == true){
//        updateSub = true;
//        } else {
//            updateSub = false;
//            };

    printk("minutes counter: %d --- CONN: %d --- SUB: %d\n", tickMins, connected, updateSub); 
    }

/* Setup Timers. END *//////////////////////////////////////////////////////////////////////

/*--Report TFM has most recent setting update.--*/
void pub_settings_packet(char reportName[24]){
    //char message_buff[256];
    memset(message_buff, 0, sizeof(message_buff)); /*--clear register.--*/
    memset(batVoltage,  0, sizeof(batVoltage));
    
    //Report Name:
    strcat(message_buff, "{ "); //Begining characters for JSON.
    strcat(message_buff, "!!reportName!!:!!");
    strcat(message_buff, reportName);
    strcat(message_buff, "!!");

    //Device ID:
    strcat(message_buff, ","); //',' between items.
    strcat(message_buff, "!!deviceId!!:!!");
    strcat(message_buff, CONFIG_MQTT_CLIENT_ID);
    strcat(message_buff, "!!");

    //Battery Condition:
    strcat(message_buff, ","); //',' between items.
    strcat(message_buff, "!!battery!!:!!");
    itoa(bat_voltage(), batVoltage, 10);
    strcat(message_buff, batVoltage);
    strcat(message_buff, "!!");

    //Device TimeStamp:
    strcat(message_buff, ","); //',' between items.
    strcat(message_buff, "!!devTimeStamp!!:!!");
    strcat(message_buff, ctHrsMins);
    strcat(message_buff, "!!");

    //MedA:
    strcat(message_buff, ","); //',' between items.
    strcat(message_buff, "!!barcodeA!!:!!");
    strcat(message_buff, medABarcode);
    strcat(message_buff, "!!");
    
    strcat(message_buff, ","); //',' between items.
    strcat(message_buff, "!!doseA!!:!!");
    strcat(message_buff, medADose);
    strcat(message_buff, "!!");

    strcat(message_buff, ","); //',' between items.
    strcat(message_buff, "!!formatA!!:!!");
    strcat(message_buff, medAFormat);
    strcat(message_buff, "!!");

    strcat(message_buff, ","); //',' between items.
    strcat(message_buff, "!!nameA!!:!!");
    strcat(message_buff, medAName);
    strcat(message_buff, "!!");

    strcat(message_buff, ","); //',' between items.
    strcat(message_buff, "!!statusA!!:!!");
    strcat(message_buff, medAStatus);
    strcat(message_buff, "!!");

    //MedB:
    strcat(message_buff, ","); //',' between items.
    strcat(message_buff, "!!barcodeB!!:!!");
    strcat(message_buff, medBBarcode);
    strcat(message_buff, "!!");
    
    strcat(message_buff, ","); //',' between items.
    strcat(message_buff, "!!doseB!!:!!");
    strcat(message_buff, medBDose);
    strcat(message_buff, "!!");

    strcat(message_buff, ","); //',' between items.
    strcat(message_buff, "!!formatB!!:!!");
    strcat(message_buff, medBFormat);
    strcat(message_buff, "!!");

    strcat(message_buff, ","); //',' between items.
    strcat(message_buff, "!!nameB!!:!!");
    strcat(message_buff, medBName);
    strcat(message_buff, "!!");

    strcat(message_buff, ","); //',' between items.
    strcat(message_buff, "!!statusB!!:!!");
    strcat(message_buff, medBStatus);
    strcat(message_buff, "!!");

    //MedC:
    strcat(message_buff, ","); //',' between items.
    strcat(message_buff, "!!barcodeC!!:!!");
    strcat(message_buff, medCBarcode);
    strcat(message_buff, "!!");
    
    strcat(message_buff, ","); //',' between items.
    strcat(message_buff, "!!doseC!!:!!");
    strcat(message_buff, medCDose);
    strcat(message_buff, "!!");

    strcat(message_buff, ","); //',' between items.
    strcat(message_buff, "!!formatC!!:!!");
    strcat(message_buff, medCFormat);
    strcat(message_buff, "!!");

    strcat(message_buff, ","); //',' between items.
    strcat(message_buff, "!!nameC!!:!!");
    strcat(message_buff, medCName);
    strcat(message_buff, "!!");

    strcat(message_buff, ","); //',' between items.
    strcat(message_buff, "!!statusC!!:!!");
    strcat(message_buff, medCStatus);
    strcat(message_buff, "!!");
    
    //MedD:
    strcat(message_buff, ","); //',' between items.
    strcat(message_buff, "!!barcodeD!!:!!");
    strcat(message_buff, medDBarcode);
    strcat(message_buff, "!!");
    
    strcat(message_buff, ","); //',' between items.
    strcat(message_buff, "!!doseD!!:!!");
    strcat(message_buff, medDDose);
    strcat(message_buff, "!!");

    strcat(message_buff, ","); //',' between items.
    strcat(message_buff, "!!formatD!!:!!");
    strcat(message_buff, medDFormat);
    strcat(message_buff, "!!");

    strcat(message_buff, ","); //',' between items.
    strcat(message_buff, "!!nameD!!:!!");
    strcat(message_buff, medDName);
    strcat(message_buff, "!!");

    strcat(message_buff, ","); //',' between items.
    strcat(message_buff, "!!statusD!!:!!");
    strcat(message_buff, medDStatus);
    strcat(message_buff, "!!");   
     
    //MedE:
    strcat(message_buff, ","); //',' between items.
    strcat(message_buff, "!!barcodeE!!:!!");
    strcat(message_buff, medEBarcode);
    strcat(message_buff, "!!");
    
    strcat(message_buff, ","); //',' between items.
    strcat(message_buff, "!!doseE!!:!!");
    strcat(message_buff, medEDose);
    strcat(message_buff, "!!");

    strcat(message_buff, ","); //',' between items.
    strcat(message_buff, "!!formatE!!:!!");
    strcat(message_buff, medEFormat);
    strcat(message_buff, "!!");

    strcat(message_buff, ","); //',' between items.
    strcat(message_buff, "!!nameE!!:!!");
    strcat(message_buff, medEName);
    strcat(message_buff, "!!");

    strcat(message_buff, ","); //',' between items.
    strcat(message_buff, "!!statusE!!:!!");
    strcat(message_buff, medEStatus);
    strcat(message_buff, "!!");
    strcat(message_buff, " }"); //Begining characters for JSON.

    data_publish(&client, MQTT_QOS_0_AT_MOST_ONCE, message_buff, strlen(message_buff));
    };

/*--Reconnect sequence.--*/
void reconnect(){
    /* Reconnect Sequence. */
    if(connected == false){   
        alarm = false;


        clearDisplay();
        small_print(1, 100, "CONNECTING...");

        printk("Disconnecting MQTT client...\n");

        err = mqtt_live(&client);
        if (err != 0) {
        printk("ERROR: mqtt_live %d\n", err);
        }
        printk("LTE reconnecting.\n");

        err = mqtt_disconnect(&client);
        printk("err: %d \n", err);
        if (err) {
            printk("Could not disconnect MQTT client. Error: %d\n", err);
            }

        modem_configure();
        
        client_init(&client);
        
        err = mqtt_connect(&client);
        if (err != 0) {
            printk("ERROR: mqtt_connect %d\n", err);
            return;
            }
       
        err = fds_init(&client);
        if (err != 0) {
            printk("ERROR: fds_init %d\n", err);
            return;
            } 


        err = poll(&fds, 1, K_SECONDS(CONFIG_MQTT_KEEPALIVE)); //CONFIG_MQTT_KEEPALIVE
        if (err < 0) {
            printk("ERROR: poll %d\n", errno);
            //break;
            }
    

        err = mqtt_live(&client);
        if (err != 0) {
            printk("ERROR: mqtt_live %d\n", err);
            //break;
            }

        if ((fds.revents & POLLIN) == POLLIN) {
            err = mqtt_input(&client);
    	if (err != 0) {
                printk("ERROR: mqtt_input %d\n", err);
                //break;
                }
            }


        if ((fds.revents & POLLERR) == POLLERR) {
            printk("POLLERR\n");
            //break;
            }

        /* POLLNVAL - Invalid request: fd not open (output only) */
        if ((fds.revents & POLLNVAL) == POLLNVAL) {
            printk("POLLNVAL\n");
            //break;
            }

        /*--Report that device has reconnected.--*/
        char message_buff[69];
        memset(message_buff, 0, sizeof(message_buff)); /*--clear register.--*/
        memset(batVoltage,  0, sizeof(batVoltage));
        
        strcat(message_buff, "Reconnect");
        strcat(message_buff, ",");

        strcat(message_buff, CONFIG_MQTT_CLIENT_ID);
        strcat(message_buff, ",");

        itoa(bat_voltage(), batVoltage, 10);
        strcat(message_buff, batVoltage);
        
        //CUASES BUGGY BEHAVIOR!!!
        //data_publish(&client, MQTT_QOS_0_AT_MOST_ONCE, message_buff, strlen(message_buff));
        if(restart == true){
            pub_settings_packet("Restart");
            dev_condition = standby;
            restart = false;
            skip = true;
            } else {
                pub_settings_packet("Reconnect");
                skip = true;
                };

        /*--set to starndby to enter standby condition (home screen).--*/
        //dev_condition = standby;
        update_display = true;
        updateSub = true;
        }
    /*--Reconnect Sequence. END--*/
    };



/*--function to keep connection alive and 'MQTT SUB' periodicaly.--*/
void maintain_connection(){

    /*--Enter loop if updateSub == true && connected == true--*/
    if(updateSub == true && connected == true){
        //gpio_pin_read(dev, 6, &button_A_state);
        //gpio_pin_write(dev, LED_RED, 1); //red LED on.

        err = poll(&fds, 1, K_SECONDS(CONFIG_MQTT_KEEPALIVE)); //CONFIG_MQTT_KEEPALIVE
        if (err < 0) {
            printk("ERROR: poll %d\n", errno);
            //break;
            }

        err = mqtt_live(&client);
        if (err != 0) {
            printk("ERROR: mqtt_live %d\n", err);
	   	//break;
            }      

        if ((fds.revents & POLLIN) == POLLIN) {
            err = mqtt_input(&client);
	   	if (err != 0) {
                printk("ERROR: mqtt_input %d\n", err);
                //break;
                }
            }

        if ((fds.revents & POLLERR) == POLLERR) {
            printk("POLLERR\n");
            //break;
            }

        /* POLLNVAL - Invalid request: fd not open (output only) */
        if ((fds.revents & POLLNVAL) == POLLNVAL) {
            printk("POLLNVAL\n");
	        //break;
            }
 
        printk("payload_buf: %s  -- %d\n", payload_buf, strlen(payload_buf));
        updateSub = false;


        } else if(updateSub == false){
            // keep connection alive:
            //gpio_pin_write(dev, LED_RED, 0);

            /*--keep MQTT connection alive.--*/
            err = poll(&fds, 1, K_SECONDS(CONFIG_MQTT_KEEPALIVE)); //CONFIG_MQTT_KEEPALIVE
            if (err < 0) {
                printk("ERROR: poll %d\n", errno);
                //break;
                }
    
            err = mqtt_live(&client);
            if (err != 0) {
                printk("ERROR: mqtt_live %d\n", err);
                //break;
                }
            /* keep MQTT connection alive. END */
            }
    };  



/*--Variables for UI itmes.--*/
//convert to int
float ct_f;
int clockHrs_i;
int clockMin_i;

//convert to string
char clockHrs_c[3];
char clockMin_c[3];

char medCount[3];
/*--Variables for UI itmes. END--*/



/*--Button A, B toggling.--*/

//Button A.
bool btnAtoggle = true;
int btnAstateNew = 0;
int btnAstateOld = 0;

void btn_a_init(){
    btnAtoggle = true;
    btnAstateNew = 0;
    btnAstateOld = 0;
    };

void btn_a_toggle(void){
    //Get current state of button A.
    gpio_pin_read(dev, 6, &btnAstateNew);

    //Has button gone high since last time?
    if(btnAstateNew == false && btnAstateOld == true){
        if(btnAtoggle == false){
            //Toggle on
            btnAtoggle = true;

        } else {
            //Toggle off
            btnAtoggle = false;
            }
        }
        //Save button state so we can tell if it changed text time button is pressed.
        btnAstateOld = btnAstateNew;
    };

//Button B.
bool btnBtoggle = true;
int btnBstateNew = 0;
int btnBstateOld = 0;

void btn_b_init(){
    btnBtoggle = true;
    btnBstateNew = 0;
    btnBstateOld = 0;
    };


void btn_b_toggle(void){
    //Get current state of button A.
    gpio_pin_read(dev, 7, &btnBstateNew);

    //Has button gone high since last time?
    if(btnBstateNew == false && btnBstateOld == true){
        if(btnBtoggle == false){
            //Toggle on
            btnBtoggle = true;

        } else {
            //Toggle off
            btnBtoggle = false;
            }
        }
        //Save button state so we can tell if it changed text time button is pressed.
        btnBstateOld = btnBstateNew;
    };





//STUPIDLY WRITTEN CODE HERE.
void update_battery_life(){
    memset(batLife_c,  0, sizeof(batLife_c));
    //batLife_f = (( bat_voltage() - 3300 ) / 900) * 100;
    batLife_f = bat_voltage() - 3300;
    batLife_f = batLife_f / 900;
    batLife_f = batLife_f * 100;
    batLife_i = batLife_f + 0;
    itoa(batLife_i, batLife_c, 10); 
    strcat(batLife_c, "%");
    };

//Variables for TEMPORARY BAD CODE...
char DOSE[32];
char NAME[32];
char FORM[32];
char STAT[32];

char new_barcode_buffer[256];

void main(void){

    /*--UART device bindings.--*/
    struct device *uart0 = device_get_binding("UART_0");
    struct device *uart1 = device_get_binding("UART_1");
    //Set UART callbacks.
    uart_irq_rx_enable(uart0);
    uart_irq_callback_set(uart1, uart_cb);
    uart_irq_rx_enable(uart1);

    init_MT15(uart1);

    /*--Initialize elva display of boast.--*/
    dispBegin();
    spi_init();
    printk("\nSPI ready!\n");
    clearDisplay();
    /*--Initialize elva display. END--*/ 

    /*--initialize and test leds and buttons.--*/
    printk("initializing GPIOs\n");
    dev = device_get_binding("GPIO_0");
    init_elva_gpio();
    /*--GPIO start and test. END--*/

    /*--Initialize battery monitor and haptic driver (I2C).--*/
    init_elva_i2c();
    i2c_dev = device_get_binding(I2C_DEV);
    /*--Initialize Haptic feedback.--*/
    u8_t vib_init_sequence[3] = {0x01, 0x01, 0x00};
    i2c_write(i2c_dev, &vib_init_sequence[0], 2, 0x5a);
    
    printk("Value of CONFIG_MQTT_ALIVE: %d\n", CONFIG_MQTT_KEEPALIVE);
    vibrate(true);
    k_sleep(1000);
    vibrate(false);

    //Setup battery monitor and haptic driver.
    u8_t write_bytes[1] = {0x30, 0x00, 0x00}; //{0x01, 0x0C, 0x00} 0x1c
    u8_t read_bytes[10] = "HELLO THER";
    bat_setCapacity(800);

    printk("Control    returned : %d\n", bat_control());
    printk("Voltage mV returned : %d\n", bat_voltage());
    printk("FCC    mAh returned : %d\n", bat_FCC());
    //printk("SOCUF test returned : %d\n", bat_SOCUF());
    printk("avgCurrent returned : %d\n", bat_AverageCurrent());
    printk("SOC   test returned : %d\n", bat_SOC());
    printk("OpConfig   returned : %d\n", bat_OpConfig());

    /*--Initialize battery monitor and haptic driver (I2C).--*/
     
    /*--Initialize and test timer.--*/
    k_timer_init(&time_sub, expiry_function_sub, NULL);  //Timer
    k_timer_start(&time_sub, K_SECONDS(TIMER_INTERVAL_SUB), K_SECONDS(TIMER_INTERVAL_SUB)); //execute every minute

    k_timer_init(&time_sec, expiry_function_second, NULL);  //Timer
    k_timer_start(&time_sec, K_SECONDS(TIMER_INTERVAL_SEC), K_SECONDS(TIMER_INTERVAL_SEC)); //execute every second

    k_timer_init(&time_min, expiry_function_minute, NULL);  //Timer
    k_timer_start(&time_min, K_SECONDS(TIMER_INTERVAL_MIN), K_SECONDS(TIMER_INTERVAL_MIN)); //execute every minute
    /*--Timer begine and test. END--*/
    
    ///////////////////////////////////////////////
    
    /*-- --*/
    static u32_t button_A_state;   
    int n = 0;

    printk("elva starting\n");

    gpio_pin_write(dev, LED_RED, 0);
    gpio_pin_write(dev, LED_GRN, 0);
    gpio_pin_write(dev, SCANNER, 1);
    gpio_pin_write(dev, DISPLAY, 1);


    while(true){
        
//        /*--Recharge Device.--*/
//        bool drawRecharge = false;
//        while(batVoltage > 3500){
//          if(drawRecharge == false){
//              drawRecharge = true;
//              clearDisplay();
//              medium_print(1, 100, "Low");
//              medium_print(1, 80, "Battery");
//              };
//              k_cpu_idle();
//          };

        /*--LTE functions--*/
        reconnect();
        maintain_connection();

        /*--Enter this function when connection is lost.--*/
        switch(dev_condition){
            case disconnected:
                //disconnected
                if(update_display == true){
                    update_display = false;
                    clearDisplay();
                    small_print(1, 100, "CONNECTING...");
                    small_print(1, 80 , "0");
                    reconnect();
                    };

                break;
            
            case standby:
                //standby (home screen)
                //alarm = true;

                /*--Show the correct stuff on the display.--*/
                if(update_display == true){
                    update_display = false;

                    btn_a_init();
                    btn_b_init();

                    gpio_pin_write(dev, DISPLAY, 1);
                    gpio_pin_write(dev, SCANNER, 1);

                    evaluate_med_list();

                    clearDisplay();

                    /*--Generate clock string from int currentTime.--*/
                    ct_f = currentTime / 60; 
                    clockHrs_i = ct_f; //hrs
                    clockMin_i = currentTime % 60;
                    
                    itoa(clockHrs_i % 12, clockHrs_c, 10);
                    itoa(clockMin_i, clockMin_c, 10);
                     
                    //Clear 'clock' register.
                    memset(clock,  0, sizeof(clock));
                    //Appent Hour
                    strcat(clock, clockHrs_c);
                    strcat(clock, ":");
                    strcat(clock, clockMin_c);
                    
                    //itoa(currentTime, clock, 10);
//                    medium_print(1, 112, clock);
                    large_print(1, 145, clockHrs_c);
                    large_print(1,  90, clockMin_c);

                    //Generate Battery info:
                    //1) clear registers
                    memset(batVoltage,  0, sizeof(batVoltage));
                    memset(batSocuf,  0, sizeof(batSocuf));

                    //2) Convert intergers to characters.
                    itoa(bat_voltage(), batVoltage, 10);
                    itoa(bat_SOCUF(), batSocuf, 10);

                    //3) Concatinate string for better presentation.
                    strcat(batVoltage, " mV ");
                    strcat(batVoltage, batSocuf);
                    strcat(batVoltage, "%");

                    //4) Update battery text on display.
                    update_battery_life();
                    small_print(17 - strlen(batLife_c), 160, batLife_c);
                    //small_print(1, 80, avgmA);
                    
                    /*--Temporary items for development testing.--*/
                    itoa(tickMins, minsCount, 10);
                    strcat(minsCount, "mins ");
                    strcat(minsCount, CONFIG_MQTT_CLIENT_ID);

//                    small_print(1, 50, minsCount);
//                    small_print(1, 35, batVoltage);
//                    small_print(1, 20, ctHrsMins);
                    small_print(1, 20, CONFIG_MQTT_CLIENT_ID);
                    
                    /*-- Test UI schemes here: --*/
//                    medium_print(1, 140, "TAKE:");
//                    large_print(1, 110, "2");
//                    medium_print(1,  60, "PILLS");
//                    medium_print(1,  30, "ADVIL");
                    //pub_settings_packet("Still_connected");
                    //data_publish(&client, MQTT_QOS_0_AT_MOST_ONCE, "G", 1);
                    printk("Device Condition: standby (home screen).\n");
                    }; 
                
                /*--Scan button toggling functions and execute.--*/
                //btn_a_toggle();
                //btn_b_toggle();
                //gpio_pin_write(dev, SCANNER, btnBtoggle);

                if(med_count > 0 && tfmwTimeStamp != "OOB"){
                    memset(barcode_buffer, 0, sizeof(barcode_buffer));
                    update_display = true;
                    publish_message = true;
                    dev_condition  = time_for_med;
                    };

                k_cpu_idle();

                break;

            //time_for_med.
            case time_for_med:
                
                /*--Publish TFM begin report.--*/
                if(publish_message == true){
                    publish_message = false;
                    //pub_settings_packet("TFM_settings");
                    
                    btn_a_init();
                    btn_b_init();
                    
                    alarm = true;
                    };
                
                /*--Scan button toggling functions and execute.--*/
                btn_a_toggle();
                btn_b_toggle();

                gpio_pin_write(dev, DISPLAY, btnAtoggle);
                if(alarm == false){
                    gpio_pin_write(dev, SCANNER, btnBtoggle);
                    };

                /*--Turn Off alarm.--*/
                if(btnBtoggle == false && alarm == true){
                    alarm = false;
                    btn_a_init();
                    btn_b_init();
                    };
                
                /*--Show the correct stuff on the display.--*/
                if(update_display == true){
                    update_display = false;
                    evaluate_med_list(); 
                    //PUBLISH 'TFM REMINDER STARTED' REPORT TO BACKEND HERE
                    clearDisplay();
                                        //Generate Battery info:
                    //1) clear registers
                    memset(batVoltage,  0, sizeof(batVoltage));
                    //2) Convert intergers to characters.
                    itoa(bat_voltage(), batVoltage, 10);
                    //3) Concatinate string for better presentation.
                    strcat(batVoltage, " mV");
                    //4) Update battery text on display.
                    update_battery_life();
                    small_print(17 - strlen(batLife_c), 160, batLife_c);

                    medium_print(0, 140, "SCAN:");

                    if(strcmp("pending", medAStatus) == 0 && strcmp("empty", medAName) != 0){
                        medium_print(0, 110, medAName);
                        };
                    if(strcmp("pending", medBStatus) == 0 && strcmp("empty", medBName) != 0){
                        medium_print(0, 80,  medBName);
                        };
                    if(strcmp("pending", medCStatus) == 0 && strcmp("empty", medCName) != 0){
                        medium_print(0, 50,  medCName);
                        };
                    if(strcmp("pending", medDStatus) == 0 && strcmp("empty", medDName) != 0){
                        medium_print(0, 20,  medDName);
                        };
                    if(strcmp("pending", medEStatus) == 0 && strcmp("empty", medEName) != 0){
                        small_print(0, 10 , medEName);
                        };
                        
                    itoa(med_count, medCount, 10);
                    small_print(1, 15 , medCount);

                    printk("Med count: %d\n", med_count);
                    printk("Device Condition: time_for_med.\n");
                    };
                
                /*--Check setting and react propertly.--*/
                evaluate_med_list(); 
                if(med_count == 0 || currentTime >= tfmwEnd){
                    update_display = true;
                    publish_message = true;
                    dev_condition  = score_board;
                    } else if(tfmwTimeStamp == "OOB"){ //Error has occured and packet will be empty. If Data is produced do not trust it.
                        //pub_settings_packet("Invalid_data");
                        } else {
                            //Do Nothing.
                            };
                                
                /*--Go to 'Directions' View after scanned the correct barcode.--*/
                //TEMPORARY BAD CODE...
                

                //1)Barcode A:
                if(strcmp(barcode_buffer, medABarcode) == 0){
                    //Clear registers.
                    memset(DOSE, 0, sizeof(DOSE));
                    memset(FORM, 0, sizeof(FORM));
                    memset(NAME, 0, sizeof(NAME));
                    memset(barcode_buffer, 0, sizeof(barcode_buffer));
                    
                    //Insert proper data.
                    strcat(DOSE, medADose);
                    strcat(FORM, medAFormat);
                    strcat(NAME, medAName);

                    //Update medAStatus
                    memset(medAStatus, 0, sizeof(medAStatus));
                    strcat(medAStatus, "scanned");
                    memset(STAT, 0, sizeof(STAT));
                    strcat(STAT, medAStatus);

                    //Go to medA directions.
                    update_display = true;
                    publish_message = true;
                    evaluate_med_list();
                    //pub_settings_packet("med_scanned");
                    dev_condition  = med_directions;
                    } else {
                        //Do Nothing.
                        };

                //2)Barcode B:
                if(strcmp(barcode_buffer, medBBarcode) == 0){
                    //Clear registers.
                    memset(DOSE, 0, sizeof(DOSE));
                    memset(FORM, 0, sizeof(FORM));
                    memset(NAME, 0, sizeof(NAME));
                    memset(barcode_buffer, 0, sizeof(barcode_buffer));
                    
                    //Insert proper data.
                    strcat(DOSE, medBDose);
                    strcat(FORM, medBFormat);
                    strcat(NAME, medBName);

                    //Update medBStatus
                    memset(medBStatus, 0, sizeof(medBStatus));
                    strcat(medBStatus, "scanned");
                    memset(STAT, 0, sizeof(STAT));
                    strcat(STAT, medBStatus);

                    //Go to medB directions.
                    update_display = true;
                    publish_message = true;
                    evaluate_med_list();
                    //pub_settings_packet("med_scanned");
                    dev_condition  = med_directions;
                    } else {
                        //Do Nothing.
                        };
                                        
                //3)Barcode C:
                if(strcmp(barcode_buffer, medCBarcode) == 0){
                    //Clear registers.
                    memset(DOSE, 0, sizeof(DOSE));
                    memset(FORM, 0, sizeof(FORM));
                    memset(NAME, 0, sizeof(NAME));
                    memset(barcode_buffer, 0, sizeof(barcode_buffer));
                    
                    //Insert proper data.
                    strcat(DOSE, medCDose);
                    strcat(FORM, medCFormat);
                    strcat(NAME, medCName);

                    //Update medCStatus
                    memset(medCStatus, 0, sizeof(medCStatus));
                    strcat(medCStatus, "scanned");
                    memset(STAT, 0, sizeof(STAT));
                    strcat(STAT, medCStatus);

                    //Go to medC directions.
                    update_display = true;
                    publish_message = true;
                    evaluate_med_list();
                    //pub_settings_packet("med_scanned");
                    dev_condition  = med_directions;
                    } else {
                        //Do Nothing.
                        };

                //4)Barcode D:
                if(strcmp(barcode_buffer, medDBarcode) == 0){
                    //Clear registers.
                    memset(DOSE, 0, sizeof(DOSE));
                    memset(FORM, 0, sizeof(FORM));
                    memset(NAME, 0, sizeof(NAME));
                    memset(barcode_buffer, 0, sizeof(barcode_buffer));
                    
                    //Insert proper data.
                    strcat(DOSE, medDDose);
                    strcat(FORM, medDFormat);
                    strcat(NAME, medDName);

                    //Update medDStatus
                    memset(medDStatus, 0, sizeof(medDStatus));
                    strcat(medDStatus, "scanned");
                    memset(STAT, 0, sizeof(STAT));
                    strcat(STAT, medDStatus);
                    
                    //Go to medD directions.
                    update_display = true;
                    publish_message = true;
                    evaluate_med_list();
                    //pub_settings_packet("med_scanned");
                    dev_condition  = med_directions;
                    } else {
                        //Do Nothing.
                        };

                //5)Barcode E:
                if(strcmp(barcode_buffer, medEBarcode) == 0){
                    //Clear registers.
                    memset(DOSE, 0, sizeof(DOSE));
                    memset(FORM, 0, sizeof(FORM));
                    memset(NAME, 0, sizeof(NAME));
                    memset(barcode_buffer, 0, sizeof(barcode_buffer));
                    
                    //Insert proper data.
                    strcat(DOSE, medEDose);
                    strcat(FORM, medEFormat);
                    strcat(NAME, medEName);

                    //Update medEStatus
                    memset(medEStatus, 0, sizeof(medEStatus));
                    strcat(medEStatus, "scanned");
                    memset(STAT, 0, sizeof(STAT));
                    strcat(STAT, medEStatus);

                    //Go to medE directions.
                    update_display = true;
                    publish_message = true;
                    evaluate_med_list();
                    //pub_settings_packet("med_scanned");
                    dev_condition  = med_directions;
                    } else {
                        //Do Nothing.
                        };

                break;

            case wrong_med:
                //wrong_med
                break;
            
            case med_directions:
                /*--Publish correct med scanned report.--*/
                if(publish_message == true){
                    publish_message = false;
                    pub_settings_packet("med_scanned");
                    btn_a_init();
                    btn_b_init();
                    };

                /*--Show the correct stuff on the display.--*/
                if(update_display == true){
                    update_display = false;

                    //PUBLISH 'TFM REMINDER STARTED' REPORT TO BACKEND HERE
                    clearDisplay();
                    medium_print(1, 140, "TAKE:");
                    large_print(1, 110, DOSE);
                    medium_print(1,  60, FORM);
                    medium_print(1,  30, NAME);

                    printk("Device Condition: med_directions.\n");
                    };

                /*--Scan button toggling functions and execute.--*/
                btn_a_toggle();
                btn_b_toggle();
                
                /*--Go to med_taken view.--*/
                if(btnBtoggle == false){
                    update_display = true;
                    publish_message = true;
                    dev_condition = med_taken;
                    }; //else if btnAtoggle == false go back to TFM view.

                break;
            
            case med_taken:
                /*--Publish med_taken report.--*/
                if(publish_message == true){ 
                    publish_message = false;
                    
                    btn_a_init();
                    btn_b_init();
                    };

                /*--Show the correct stuff on the display.--*/
                if(update_display == true){
                    update_display = false;
                    clearDisplay();
                    medium_print(1, 112, "GREAT");
                    medium_print(2,  82, "JOB");
                    printk("Device Condition: med_taken.\n");
                    };

                /*--Scan button toggling functions and execute.--*/
                btn_a_toggle();
                btn_b_toggle();

                if(btnBtoggle == false){
                    update_display = true;
                    publish_message = true;
                    dev_condition = time_for_med;
                    };

                break;


            case score_board:

                if(update_display == true){
                    update_display = false;
                    clearDisplay();
                    alarm = false;
                    medium_print(2, 112, "ALL");
                    medium_print(2,  82, "DONE");
                    printk("Device Condition: SCORE BOARD.\n");

                    btn_a_init();
                    btn_b_init();
                    };
                
                                /*--Publish med_taken report.--*/
                if(publish_message == true){ 
                    publish_message = false;
                    pub_settings_packet("tfm_score");
                    };

                /*--Scan button toggling functions and execute.--*/
                btn_a_toggle();
                btn_b_toggle();

                if(btnBtoggle == false){
                    update_display = true;
                    dev_condition = standby;
                    };

                break;

            default:
                //Code here.
                break;      
        };

        }
   
    } /*--main() END--*/
