/*******************************************************************************
  System Configuration Header

  File Name:
    configuration.h

  Summary:
    Build-time configuration header for the system defined by this project.

  Description:
    An MPLAB Project may have multiple configurations.  This file defines the
    build-time options for a single configuration.

  Remarks:
    This configuration header must not define any prototypes or data
    definitions (or include any files that do).  It only provides macro
    definitions for build-time configuration options

*******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
* Copyright (C) 2018 Microchip Technology Inc. and its subsidiaries.
*
* Subject to your compliance with these terms, you may use Microchip software
* and any derivatives exclusively with Microchip products. It is your
* responsibility to comply with third party license terms applicable to your
* use of third party software (including open source software) that may
* accompany Microchip software.
*
* THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
* EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
* WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
* PARTICULAR PURPOSE.
*
* IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
* INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
* WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
* BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
* FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
* ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
* THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
*******************************************************************************/
// DOM-IGNORE-END

#ifndef CONFIGURATION_H
#define CONFIGURATION_H

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************
/*  This section Includes other configuration headers necessary to completely
    define this configuration.
*/

#include "user.h"
#include "toolchain_specifics.h"

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

extern "C" {

#endif
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: System Configuration
// *****************************************************************************
// *****************************************************************************



// *****************************************************************************
// *****************************************************************************
// Section: System Service Configuration
// *****************************************************************************
// *****************************************************************************
/* TIME System Service Configuration Options */
#define SYS_TIME_INDEX_0                     0
#define SYS_TIME_MAX_TIMERS                  5
#define SYS_TIME_HW_COUNTER_WIDTH            32
#define SYS_TIME_HW_COUNTER_PERIOD           4294967295U
#define SYS_TIME_HW_COUNTER_HALF_PERIOD	     (SYS_TIME_HW_COUNTER_PERIOD>>1)
#define SYS_TIME_CPU_CLOCK_FREQUENCY         200000000
#define SYS_TIME_COMPARE_UPDATE_EXECUTION_CYCLES      (620)

#define SYS_CONSOLE_INDEX_0                       0





/* File System Service Configuration */

#define SYS_FS_MEDIA_NUMBER               1
#define SYS_FS_VOLUME_NUMBER              1

#define SYS_FS_AUTOMOUNT_ENABLE           true
#define SYS_FS_CLIENT_NUMBER              1
#define SYS_FS_MAX_FILES                  1
#define SYS_FS_MAX_FILE_SYSTEM_TYPE       1
#define SYS_FS_MEDIA_MAX_BLOCK_SIZE       512
#define SYS_FS_MEDIA_MANAGER_BUFFER_SIZE  2048
#define SYS_FS_USE_LFN                    1
#define SYS_FS_FILE_NAME_LEN              255
#define SYS_FS_CWD_STRING_LEN             1024

/* File System RTOS Configurations*/
#define SYS_FS_STACK_SIZE                 1024
#define SYS_FS_PRIORITY                   1

#define SYS_FS_FAT_VERSION                "v0.14"
#define SYS_FS_FAT_READONLY               false
#define SYS_FS_FAT_CODE_PAGE              437
#define SYS_FS_FAT_MAX_SS                 SYS_FS_MEDIA_MAX_BLOCK_SIZE



#define SYS_FS_MEDIA_TYPE_IDX0 				SYS_FS_MEDIA_TYPE_NVM
#define SYS_FS_TYPE_IDX0 					FAT
					
#define SYS_FS_MEDIA_IDX0_MOUNT_NAME_VOLUME_IDX0 			"/mnt/myDrive1"
#define SYS_FS_MEDIA_IDX0_DEVICE_NAME_VOLUME_IDX0			"/dev/nvma1"
								


#define SYS_NET_INDEX0_MODE       				SYS_NET_MODE_CLIENT
#define SYS_NET_INDEX0_PORT        				1
#define SYS_NET_INDEX0_RECONNECT       			1
#define SYS_NET_INDEX0_ENABLE_TLS        		1
#define SYS_NET_INDEX0_IPPROT       			SYS_NET_IP_PROT_TCP
#define SYS_NET_INDEX0_HOST_NAME        		"192.168.1.1"

#define SYS_NET_TLS_ENABLED



#define SYS_MQTT_PAHO

#define SYS_MQTT_INDEX0_MQTT_PORT        				8883
#define SYS_MQTT_INDEX0_BROKER_NAME        				"a1gqt8sttiign3-ats.iot.us-east-2.amazonaws.com"
#define SYS_MQTT_INDEX0_ENABLE_TLS        				true
#define SYS_MQTT_INDEX0_RECONNECT        				true
#define SYS_MQTT_INDEX0_CLIENT_ID        				""
#define SYS_MQTT_INDEX0_KEEPALIVE_INTERVAL 				30

#define SYS_MQTT_INDEX0_SUB_TOPIC_COUNT					0
#define SYS_MQTT_INDEX0_TOPIC_NAME        				" "
#define SYS_MQTT_INDEX0_SUB_QOS							0
#define SYS_MQTT_INDEX0_ENTRY_VALID        				false



#define SYS_MQTT_CFG_LWT_ENABLE								false


#define SYS_CMD_ENABLE
#define SYS_CMD_DEVICE_MAX_INSTANCES       SYS_CONSOLE_DEVICE_MAX_INSTANCES
#define SYS_CMD_PRINT_BUFFER_SIZE          1024
#define SYS_CMD_BUFFER_DMA_READY

/* Command System Service RTOS Configurations*/
#define SYS_CMD_RTOS_STACK_SIZE                1024
#define SYS_CMD_RTOS_TASK_PRIORITY             1


#define SYS_DEBUG_ENABLE
#define SYS_DEBUG_GLOBAL_ERROR_LEVEL       SYS_ERROR_DEBUG
#define SYS_DEBUG_BUFFER_DMA_READY
#define SYS_DEBUG_USE_CONSOLE


#define SYS_CONSOLE_DEVICE_MAX_INSTANCES   			1
#define SYS_CONSOLE_UART_MAX_INSTANCES 	   			1
#define SYS_CONSOLE_USB_CDC_MAX_INSTANCES 	   		0
#define SYS_CONSOLE_PRINT_BUFFER_SIZE        		512




// *****************************************************************************
// *****************************************************************************
// Section: Driver Configuration
// *****************************************************************************
// *****************************************************************************
/* Memory Driver Global Configuration Options */
#define DRV_MEMORY_INSTANCES_NUMBER          1

/* Memory Driver Instance 0 Configuration */
#define DRV_MEMORY_INDEX_0                   0
#define DRV_MEMORY_CLIENTS_NUMBER_IDX0       2
#define DRV_MEMORY_BUFFER_QUEUE_SIZE_IDX0    1
#define DRV_MEMORY_DEVICE_START_ADDRESS      0x90080000
#define DRV_MEMORY_DEVICE_MEDIA_SIZE         32UL
#define DRV_MEMORY_DEVICE_PROGRAM_SIZE       1024
#define DRV_MEMORY_DEVICE_ERASE_SIZE         4096

/* Memory Driver Instance 0 RTOS Configurations*/
#define DRV_MEMORY_STACK_SIZE_IDX0           1024
#define DRV_MEMORY_PRIORITY_IDX0             1
#define DRV_MEMORY_RTOS_DELAY_IDX0                         10

/*** WiFi PIC32MZW1 Driver Configuration ***/
#define WDRV_PIC32MZW_DEBUG_LEVEL               WDRV_PIC32MZW_DEBUG_TYPE_TRACE



// *****************************************************************************
// *****************************************************************************
// Section: Middleware & Other Library Configuration
// *****************************************************************************
// *****************************************************************************

/*** ICMPv4 Server Configuration ***/
#define TCPIP_STACK_USE_ICMP_SERVER
#define TCPIP_ICMP_ECHO_ALLOW_BROADCASTS    false


/******************************************************************************/
/*wolfSSL TLS Layer Configuration*/
/******************************************************************************/

#define WOLFSSL_ALT_NAMES
#define WOLFSSL_DER_LOAD
#define KEEP_OUR_CERT
#define KEEP_PEER_CERT
#define HAVE_CRL_IO
#define HAVE_IO_TIMEOUT
#define HAVE_FFDHE_2048
#define TFM_NO_ASM
#define WOLFSSL_NO_ASM
#define SIZEOF_LONG_LONG 8
#define WOLFSSL_USER_IO
#define NO_WRITEV
#define MICROCHIP_TCPIP
#define NO_OLD_TLS
#define USE_FAST_MATH
#define NO_WOLFSSL_SERVER
#define WOLFSSL_SMALL_STACK
#define NO_PWDBASED


/*** TCP Configuration ***/
#define TCPIP_TCP_MAX_SEG_SIZE_TX		        	1460
#define TCPIP_TCP_SOCKET_DEFAULT_TX_SIZE			1460
#define TCPIP_TCP_SOCKET_DEFAULT_RX_SIZE			5840
#define TCPIP_TCP_DYNAMIC_OPTIONS             			true
#define TCPIP_TCP_START_TIMEOUT_VAL		        	1000
#define TCPIP_TCP_DELAYED_ACK_TIMEOUT		    		100
#define TCPIP_TCP_FIN_WAIT_2_TIMEOUT		    		5000
#define TCPIP_TCP_KEEP_ALIVE_TIMEOUT		    		10000
#define TCPIP_TCP_CLOSE_WAIT_TIMEOUT		    		0
#define TCPIP_TCP_MAX_RETRIES		            		5
#define TCPIP_TCP_MAX_UNACKED_KEEP_ALIVES			6
#define TCPIP_TCP_MAX_SYN_RETRIES		        	3
#define TCPIP_TCP_AUTO_TRANSMIT_TIMEOUT_VAL			40
#define TCPIP_TCP_WINDOW_UPDATE_TIMEOUT_VAL			200
#define TCPIP_TCP_MAX_SOCKETS		                10
#define TCPIP_TCP_TASK_TICK_RATE		        	5
#define TCPIP_TCP_MSL_TIMEOUT		        	    0
#define TCPIP_TCP_QUIET_TIME		        	    0
#define TCPIP_TCP_COMMANDS   false
#define TCPIP_TCP_EXTERN_PACKET_PROCESS   false



/*** ARP Configuration ***/
#define TCPIP_ARP_CACHE_ENTRIES                 		5
#define TCPIP_ARP_CACHE_DELETE_OLD		        	true
#define TCPIP_ARP_CACHE_SOLVED_ENTRY_TMO			1200
#define TCPIP_ARP_CACHE_PENDING_ENTRY_TMO			60
#define TCPIP_ARP_CACHE_PENDING_RETRY_TMO			2
#define TCPIP_ARP_CACHE_PERMANENT_QUOTA		    		50
#define TCPIP_ARP_CACHE_PURGE_THRESHOLD		    		75
#define TCPIP_ARP_CACHE_PURGE_QUANTA		    		1
#define TCPIP_ARP_CACHE_ENTRY_RETRIES		    		3
#define TCPIP_ARP_GRATUITOUS_PROBE_COUNT			1
#define TCPIP_ARP_TASK_PROCESS_RATE		        	2
#define TCPIP_ARP_PRIMARY_CACHE_ONLY		        	true



/* Network Configuration Index 0 */
#define TCPIP_NETWORK_DEFAULT_INTERFACE_NAME_IDX0	"PIC32MZW1"
#define TCPIP_IF_PIC32MZW1

#define TCPIP_NETWORK_DEFAULT_HOST_NAME_IDX0				"MCHPBOARD_W"
#define TCPIP_NETWORK_DEFAULT_MAC_ADDR_IDX0				0

#define TCPIP_NETWORK_DEFAULT_IP_ADDRESS_IDX0			"0.0.0.0"
#define TCPIP_NETWORK_DEFAULT_IP_MASK_IDX0			"255.255.255.0"
#define TCPIP_NETWORK_DEFAULT_GATEWAY_IDX0			"192.168.1.1"
#define TCPIP_NETWORK_DEFAULT_DNS_IDX0				"192.168.1.1"
#define TCPIP_NETWORK_DEFAULT_SECOND_DNS_IDX0			"0.0.0.0"
#define TCPIP_NETWORK_DEFAULT_POWER_MODE_IDX0			"full"
#define TCPIP_NETWORK_DEFAULT_INTERFACE_FLAGS_IDX0			\
													TCPIP_NETWORK_CONFIG_DHCP_CLIENT_ON |\
													TCPIP_NETWORK_CONFIG_DNS_CLIENT_ON |\
													TCPIP_NETWORK_CONFIG_IP_STATIC
													
#define TCPIP_NETWORK_DEFAULT_MAC_DRIVER_IDX0			WDRV_PIC32MZW1_MACObject



/*** IPv4 Configuration ***/
#define TCPIP_IPV4_EXTERN_PACKET_PROCESS   false



/*** UDP Configuration ***/
#define TCPIP_UDP_MAX_SOCKETS		                	10
#define TCPIP_UDP_SOCKET_DEFAULT_TX_SIZE		    	1460
#define TCPIP_UDP_SOCKET_DEFAULT_TX_QUEUE_LIMIT    	 	3
#define TCPIP_UDP_SOCKET_DEFAULT_RX_QUEUE_LIMIT			3
#define TCPIP_UDP_USE_POOL_BUFFERS   false
#define TCPIP_UDP_USE_TX_CHECKSUM             			true
#define TCPIP_UDP_USE_RX_CHECKSUM             			true
#define TCPIP_UDP_COMMANDS   false
#define TCPIP_UDP_EXTERN_PACKET_PROCESS   false




/*** DNS Client Configuration ***/
#define TCPIP_STACK_USE_DNS
#define TCPIP_DNS_CLIENT_SERVER_TMO					60
#define TCPIP_DNS_CLIENT_TASK_PROCESS_RATE			200
#define TCPIP_DNS_CLIENT_CACHE_ENTRIES				5
#define TCPIP_DNS_CLIENT_CACHE_ENTRY_TMO			0
#define TCPIP_DNS_CLIENT_CACHE_PER_IPV4_ADDRESS		5
#define TCPIP_DNS_CLIENT_CACHE_PER_IPV6_ADDRESS		1
#define TCPIP_DNS_CLIENT_ADDRESS_TYPE			    IP_ADDRESS_TYPE_IPV4
#define TCPIP_DNS_CLIENT_CACHE_DEFAULT_TTL_VAL		1200
#define TCPIP_DNS_CLIENT_CACHE_UNSOLVED_ENTRY_TMO	10
#define TCPIP_DNS_CLIENT_LOOKUP_RETRY_TMO			5
#define TCPIP_DNS_CLIENT_MAX_HOSTNAME_LEN			128
#define TCPIP_DNS_CLIENT_MAX_SELECT_INTERFACES		4
#define TCPIP_DNS_CLIENT_DELETE_OLD_ENTRIES			true
#define TCPIP_DNS_CLIENT_USER_NOTIFICATION   false


/* Number of Endpoints used */
#define DRV_USBFS_ENDPOINTS_NUMBER                        3

/* The USB Device Layer will not initialize the USB Driver */
#define USB_DEVICE_DRIVER_INITIALIZE_EXPLICIT

/* Maximum device layer instances */
#define USB_DEVICE_INSTANCES_NUMBER                         1

/* EP0 size in bytes */
#define USB_DEVICE_EP0_BUFFER_SIZE                          64







/*** DHCP Configuration ***/
#define TCPIP_STACK_USE_DHCP_CLIENT
#define TCPIP_DHCP_TIMEOUT                          2
#define TCPIP_DHCP_TASK_TICK_RATE                   5
#define TCPIP_DHCP_HOST_NAME_SIZE                   20
#define TCPIP_DHCP_CLIENT_CONNECT_PORT              68
#define TCPIP_DHCP_SERVER_LISTEN_PORT               67
#define TCPIP_DHCP_CLIENT_ENABLED                   true
#define TCPIP_DHCP_USE_OPTION_TIME_SERVER           3
#define TCPIP_DHCP_TIME_SERVER_ADDRESSES            3
#define TCPIP_DHCP_USE_OPTION_NTP_SERVER            3
#define TCPIP_DHCP_NTP_SERVER_ADDRESSES             3



/*** tcpip_cmd Configuration ***/
#define TCPIP_STACK_COMMAND_ENABLE
#define TCPIP_STACK_COMMANDS_STORAGE_ENABLE
#define TCPIP_STACK_COMMANDS_ICMP_ECHO_REQUESTS         4
#define TCPIP_STACK_COMMANDS_ICMP_ECHO_REQUEST_DELAY    1000
#define TCPIP_STACK_COMMANDS_ICMP_ECHO_TIMEOUT          5000
#define TCPIP_STACK_COMMANDS_WIFI_ENABLE             	false
#define TCPIP_STACK_COMMANDS_ICMP_ECHO_REQUEST_BUFF_SIZE    2000
#define TCPIP_STACK_COMMANDS_ICMP_ECHO_REQUEST_DATA_SIZE    100


/*** USB Driver Configuration ***/

/* Maximum USB driver instances */
#define DRV_USBFS_INSTANCES_NUMBER                        1

/* Interrupt mode enabled */
#define DRV_USBFS_INTERRUPT_MODE                          true


/* Enables Device Support */
#define DRV_USBFS_DEVICE_SUPPORT                          true
	
/* Disable Host Support */
#define DRV_USBFS_HOST_SUPPORT                            false



/* Alignment for buffers that are submitted to USB Driver*/ 
#define USB_ALIGN  CACHE_ALIGN

/* MPLAB Harmony Net Presentation Layer Definitions*/
#define NET_PRES_NUM_INSTANCE 1
#define NET_PRES_NUM_SOCKETS 4

/* Net Pres RTOS Configurations*/
#define NET_PRES_RTOS_STACK_SIZE                5120
#define NET_PRES_RTOS_TASK_PRIORITY             1
	
#define FREERTOS



/*** TCPIP Heap Configuration ***/

#define TCPIP_STACK_USE_INTERNAL_HEAP
#define TCPIP_STACK_DRAM_SIZE                       62000
#define TCPIP_STACK_DRAM_RUN_LIMIT                  2048

#define TCPIP_STACK_MALLOC_FUNC                     pvPortMalloc

#define TCPIP_STACK_CALLOC_FUNC                     APP_Calloc

#define TCPIP_STACK_FREE_FUNC                       vPortFree



#define TCPIP_STACK_HEAP_USE_FLAGS                   TCPIP_STACK_HEAP_FLAG_ALLOC_UNCACHED

#define TCPIP_STACK_HEAP_USAGE_CONFIG                TCPIP_STACK_HEAP_USE_DEFAULT

#define TCPIP_STACK_SUPPORTED_HEAPS                  1




// *****************************************************************************
// *****************************************************************************
// Section: TCPIP Stack Configuration
// *****************************************************************************
// *****************************************************************************

#define TCPIP_STACK_USE_IPV4
#define TCPIP_STACK_USE_TCP
#define TCPIP_STACK_USE_UDP

#define TCPIP_STACK_TICK_RATE		        		5
#define TCPIP_STACK_SECURE_PORT_ENTRIES             10

#define TCPIP_STACK_ALIAS_INTERFACE_SUPPORT   false

#define TCPIP_PACKET_LOG_ENABLE     0

/* TCP/IP stack event notification */
#define TCPIP_STACK_USE_EVENT_NOTIFICATION
#define TCPIP_STACK_USER_NOTIFICATION   true
#define TCPIP_STACK_DOWN_OPERATION   true
#define TCPIP_STACK_IF_UP_DOWN_OPERATION   true
#define TCPIP_STACK_MAC_DOWN_OPERATION  true
#define TCPIP_STACK_INTERFACE_CHANGE_SIGNALING   true
#define TCPIP_STACK_CONFIGURATION_SAVE_RESTORE   true
#define TCPIP_STACK_EXTERN_PACKET_PROCESS   false




/* TCP/IP RTOS Configurations*/
#define TCPIP_RTOS_STACK_SIZE                1024
#define TCPIP_RTOS_PRIORITY             1



/*** SNTP Configuration ***/
#define TCPIP_STACK_USE_SNTP_CLIENT
#define TCPIP_NTP_DEFAULT_IF		        		"PIC32INT"
#define TCPIP_NTP_VERSION             			    	4
#define TCPIP_NTP_DEFAULT_CONNECTION_TYPE   			IP_ADDRESS_TYPE_IPV4
#define TCPIP_NTP_EPOCH		                		2208988800ul
#define TCPIP_NTP_REPLY_TIMEOUT		        		6
#define TCPIP_NTP_MAX_STRATUM		        		15
#define TCPIP_NTP_TIME_STAMP_TMO				660
#define TCPIP_NTP_SERVER		        		"time.google.com"
#define TCPIP_NTP_SERVER_MAX_LENGTH				30
#define TCPIP_NTP_QUERY_INTERVAL				600
#define TCPIP_NTP_FAST_QUERY_INTERVAL	    			14
#define TCPIP_NTP_TASK_TICK_RATE				1100
#define TCPIP_NTP_RX_QUEUE_LIMIT				2



/*** wolfCrypt Library Configuration ***/
#define MICROCHIP_PIC32
#define MICROCHIP_MPLAB_HARMONY
#define MICROCHIP_MPLAB_HARMONY_3
#define HAVE_MCAPI
#define SIZEOF_LONG_LONG 8
#define WOLFSSL_USER_IO
#define NO_WRITEV
#define NO_FILESYSTEM
#define USE_FAST_MATH
#define NO_BIG_INT
#define HAVE_MCAPI
#define WOLF_CRYPTO_CB  // provide call-back support
#define WOLFSSL_MICROCHIP_PIC32MZ
#define WOLFSSL_HAVE_MCHP_HW_CRYPTO_ECC_HW_BA414E
#define WOLFSSL_HAVE_MCHP_BA414E_CRYPTO
#define WOLFSSL_PIC32MZ_HASH
#define WOLFSSL_PIC32MZ_HASH
#define WOLFSSL_PIC32MZ_HASH
#define HAVE_HKDF
#define NO_DES3
#define WOLFSSL_AES_128
#define WOLFSSL_AES_192
#define WOLFSSL_AES_256
#define WOLFSSL_AES_DIRECT
#define HAVE_AES_DECRYPT
#define HAVE_AES_ECB
#define HAVE_AES_CBC
#define WOLFSSL_AES_COUNTER
#define HAVE_AESGCM
#define NO_RC4
#define NO_HC128
#define NO_RABBIT
#define HAVE_ECC
#define HAVE_X963_KDF
#define HAVE_ECC_ENCRYPT
#define HAVE_DH
#define NO_DSA
#define NO_RSA
#define NO_DEV_RANDOM
#define HAVE_HASHDRBG
#define WC_NO_HARDEN
#define FREERTOS
#define NO_WOLFSSL_MEMORY
#define DEBUG
#define DEBUG_WOLFSSL
#define FP_MAX_BITS 4096
/* Maximum instances of MSD function driver */
#define USB_DEVICE_MSD_INSTANCES_NUMBER     1 

#define USB_DEVICE_MSD_NUM_SECTOR_BUFFERS 1


/* Number of Logical Units */
#define USB_DEVICE_MSD_LUNS_NUMBER      1





// *****************************************************************************
// *****************************************************************************
// Section: Application Configuration
// *****************************************************************************
// *****************************************************************************


//DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

#endif // CONFIGURATION_H
/*******************************************************************************
 End of File
*/
