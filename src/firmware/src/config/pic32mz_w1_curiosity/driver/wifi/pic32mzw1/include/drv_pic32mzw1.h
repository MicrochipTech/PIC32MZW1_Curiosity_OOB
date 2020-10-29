/*******************************************************************************
Copyright (c) 2019 released Microchip Technology Inc. All rights reserved.

Microchip licenses to you the right to use, modify, copy and distribute
Software only when embedded on a Microchip microcontroller or digital signal
controller that is integrated into your product or third party product
(pursuant to the sublicense terms in the accompanying license agreement).

You should refer to the license agreement accompanying this Software for
additional information regarding your rights and obligations.

SOFTWARE AND DOCUMENTATION ARE PROVIDED AS IS WITHOUT WARRANTY OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF
MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE.
IN NO EVENT SHALL MICROCHIP OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER
CONTRACT, NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR
OTHER LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE OR
CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT OF
SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
(INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.
 *******************************************************************************/

#ifndef _DRV_PIC32MZW1_H
#define _DRV_PIC32MZW1_H

#include <stdint.h>
#include <stdbool.h>

#define DRV_PIC32MZW_MAX_SCAN_TIME                  1500
#define DRV_PIC32MZW_MIN_SCAN_TIME                  10
#define DRV_PIC32MZW_DEFAULT_ACTIVE_SCAN_TIME       20
#define DRV_PIC32MZW_DEFAULT_PASSIVE_SCAN_TIME      120
#define DRV_PIC32MZW_AP_NUM_STA_SUPPORTED           8
#define DRV_PIC32MZW_REGDOMAIN_MAX_NAME_LEN         6
#define DRV_PIC32MZW_REGDOMAIN_RES_LEN              14
#define DRV_PIC32MZW_PS_LISTEN_INTERVAL				8

#define DRV_PIC32MZW_POWER_ON_CAL_CONFIG 0x01
#define DRV_PIC32MZW_FACTORY_CAL_CONFIG  0x02
#define DRV_PIC32MZW_GAIN_TABLE_CONFIG   0x04
#define DRV_PIC32MZW_MAC_ADDRESS_CONFIG  0x08

#define DRV_PIC32MZW_LibraryInfo(NAME)  DRV_PIC32MZW_LibraryInfo_##NAME

/*
    Summary:
        WID Types definition.
    Description
        This enumeration provide the wid types(char,short,integer,string and binary data)
*/
typedef enum
{
  DRV_WIFI_WID_CHAR     = 0,
  DRV_WIFI_WID_SHORT    = 1,
  DRV_WIFI_WID_INT      = 2,
  DRV_WIFI_WID_STR      = 3,
  DRV_WIFI_WID_BIN_DATA = 4
} DRV_WIFI_WID_TYPE_T;

/* This Enum contains the various priority levels supported by the Memory    */
/* Manager for allocating Packet Memory. Lower index in the priority table   */
/* indicates higher priority.                                                */

typedef enum {MEM_PRI_CONFIG = 0,
              MEM_PRI_HPTX   = 1,
              MEM_PRI_HPRX   = 2,
              MEM_PRI_RX     = 3,
              MEM_PRI_TX     = 4,
              NUM_MEM_PRI_LEVELS = 5
} MEM_PRIORITY_LEVEL_T;

/* Structure used to maintain packet reservation related information for */
/* different priority levels.                                            */
typedef struct
{
    uint16_t num_resvd;  /* Number of buffers to be Reserved */
    uint16_t num_thresh; /* Threshold for making buffer allocation decisions */
    uint16_t num_allocd; /* Number of buffers currently allocated */
} pktmem_priority_t;

typedef enum
{
    DRV_PIC32MZW_WLAN_EVENT_SCAN_DONE,
} DRV_PIC32MZW_WLAN_EVENT_ID;

/* Bits for WID_11I_SETTINGS and dot11iInfo field of scan results. */
typedef enum
{
    DRV_PIC32MZW_PRIVACY            = 0x0001,  // Not 11i, but included here for convenience
    DRV_PIC32MZW_SKEY               = 0x0002,  // Not 11i, but included here for convenience
    DRV_PIC32MZW_11I_WEP            = 0x0004,
    DRV_PIC32MZW_11I_WEP104         = 0x0008,
    DRV_PIC32MZW_11I_WPAIE          = 0x0010,
    DRV_PIC32MZW_11I_RSNE           = 0x0020,
    DRV_PIC32MZW_11I_CCMP128        = 0x0040,
    DRV_PIC32MZW_11I_TKIP           = 0x0080,
    DRV_PIC32MZW_11I_BIPCMAC128     = 0x0100,
    DRV_PIC32MZW_11I_MFP_REQUIRED   = 0x0200,
    DRV_PIC32MZW_11I_1X             = 0x0400,
    DRV_PIC32MZW_11I_PSK            = 0x0800,
    DRV_PIC32MZW_11I_SAE            = 0x1000,
    DRV_PIC32MZW_AP                 = 0x8000,   // Indicates whether the settings are intended for STA or AP mode
    DRV_PIC32MZW_RSNA_MASK          = 0x1FF0,   // Mask of bits linked to RSNA's
} DRV_PIC32MZW_11I_MASK;

/* Harmony to library calls */

typedef int (*DRV_PIC32MZW_WLAN_EVENT_FPTR)(DRV_PIC32MZW_WLAN_EVENT_ID eventID, void *pEventDataPtr);

void wdrv_pic32mzw_user_main(void);
void wdrv_pic32mzw_process_cfg_message(uint8_t* cfgmsg);
void wdrv_pic32mzw_wlan_send_packet(uint8_t* pBuf, uint16_t pktlen, uint32_t tos, uint8_t offset);
void wdrv_pic32mzw_mac_controller_task(void);
int wdrv_pic32mzw_hook_wlan_event_handle(DRV_PIC32MZW_WLAN_EVENT_FPTR wlan_event_handle);
void wdrv_pic32mzw_mac_isr(unsigned int vector);
void wdrv_pic32mzw_timer_tick_isr(unsigned int param);
void wdrv_pic32mzw_smc_isr(unsigned int param);
uint8_t wdrv_pic32mzw_qmu_get_tx_count(void);

/* Library to Harmony calls */

typedef struct
{
    /* Note this structure is sometimes used with single-byte alignment. In such
     * cases, any fields larger than 8 bits must be accessed byte-per-byte. */
    uint8_t index;
    uint8_t ofTotal;
    uint8_t bssid[6];
    uint8_t ssid[32];
    uint16_t dot11iInfo;
    int8_t  rssi;
    uint8_t bssType;
    uint8_t channel;
} DRV_PIC32MZW_SCAN_RESULTS;

void DRV_PIC32MZW_MACEthernetSendPacket(const uint8_t *const pEthMsg, uint16_t lengthEthMsg, uint8_t hdrOffset);
void* DRV_PIC32MZW_MemAlloc(uint16_t size);
int8_t DRV_PIC32MZW_MemAddUsers(void *pBufferAddr, int count);
int8_t DRV_PIC32MZW_MemFree(void *pBufferAddr);
void* DRV_PIC32MZW_PacketMemAlloc(uint16_t size, MEM_PRIORITY_LEVEL_T priLevel);
void DRV_PIC32MZW_PacketMemFree(void *pPktBuff);
void DRV_PIC32MZW_WIDRxQueuePush(void *pPktBuff);

void DRV_PIC32MZW_MACTimer0Disable();
void DRV_PIC32MZW_MACTimer0Enable();
void DRV_PIC32MZW_MACTimer1Disable();
void DRV_PIC32MZW_MACTimer1Enable();

#endif /* _DRV_PIC32MZW1_H */
