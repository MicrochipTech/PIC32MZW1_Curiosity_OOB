/*******************************************************************************
  PIC32MZW Wireless Driver

  File Name:
    wdrv_pic32mzw.c

  Summary:
    PIC32MZW wireless driver.

  Description:
    PIC32MZW wireless driver.
 *******************************************************************************/

//DOM-IGNORE-BEGIN
/*******************************************************************************
Copyright (C) 2020-21 released Microchip Technology Inc.  All rights reserved.

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
//DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>
#include <stdbool.h>

#include "wdrv_pic32mzw.h"
#include "wdrv_pic32mzw_common.h"
#include "wdrv_pic32mzw_mac.h"
#include "wdrv_pic32mzw_cfg.h"
#include "drv_pic32mzw1_crypto.h"
#include "tcpip/tcpip_mac_object.h"
#include "tcpip/src/link_list.h"
#include "tcpip/src/tcpip_manager_control.h"
#include <sys/kmem.h>

#pragma region name="wlan_mem" origin=0xa0040000 size=0x10000

extern pktmem_priority_t g_pktmem_pri[NUM_MEM_PRI_LEVELS];

bool DRV_PIC32MZW_StoreBSSScanResult(const DRV_PIC32MZW_SCAN_RESULTS *const pScanResult);
bool DRV_PIC32MZW1_Crypto_Random_Init(CRYPT_RNG_CTX *pRngCtx);

// *****************************************************************************
// *****************************************************************************
// Section: PIC32MZW Driver Defines
// *****************************************************************************
// *****************************************************************************

#define TCPIP_THIS_MODULE_ID TCPIP_MODULE_MAC_PIC32MZW1

#define SHARED_PKT_MEM_BUFFER_SIZE          1596

#define ETHERNET_HDR_LEN                    14
#define ETH_ETHERNET_HDR_OFFSET             34

#define ZERO_CP_MIN_MAC_FRAME_OFFSET       (ETH_ETHERNET_HDR_OFFSET + 4)

#define PIC32MZW_CACHE_LINE_SIZE            CACHE_LINE_SIZE

#define PIC32MZW_RSR_PKT_NUM                40

#ifdef DRV_PIC32MZW_TRACK_MEMORY_ALLOC
#define WDRV_PIC32MZW_NUM_TRACK_ENTRIES     256
#endif

// *****************************************************************************
// *****************************************************************************
// Section: Global Data
// *****************************************************************************
// *****************************************************************************

/* This is user configurable function pointer for printf style output from driver. */
#ifndef WDRV_PIC32MZW1_DEVICE_USE_SYS_DEBUG
WDRV_PIC32MZW_DEBUG_PRINT_CALLBACK pfPIC32MZWDebugPrintCb;
#endif

/* This is the driver instance structure. */
static WDRV_PIC32MZW_DCPT pic32mzwDescriptor[2] =
{
    {
        .isInit  = false,
        .sysStat = SYS_STATUS_UNINITIALIZED,
        .isOpen  = false,
        .pCtrl   = NULL,
        .pMac    = NULL,
    },
    {
        .isInit  = false,
        .sysStat = SYS_STATUS_UNINITIALIZED,
        .isOpen  = false,
        .pCtrl   = NULL,
        .pMac    = NULL,
    }
};

/* This is the PIC32MZW1 MAC Object. */
const TCPIP_MAC_OBJECT WDRV_PIC32MZW1_MACObject =
{
    .macId                                  = TCPIP_MODULE_MAC_PIC32MZW1,
#if TCPIP_STACK_VERSION_MAJOR != 7
    .macType                                = TCPIP_MAC_TYPE_WLAN,
#endif
    .macName                                = "PIC32MZW1",
    .TCPIP_MAC_Initialize                   = WDRV_PIC32MZW_Initialize,
#if (TCPIP_STACK_MAC_DOWN_OPERATION != false)
    .TCPIP_MAC_Deinitialize                 = WDRV_PIC32MZW_Deinitialize,
    .TCPIP_MAC_Reinitialize                 = WDRV_PIC32MZW_Reinitialize,
#else
    .TCPIP_MAC_Deinitialize                 = 0,
    .TCPIP_MAC_Reinitialize                 = 0,
#endif  // (TCPIP_STACK_DOWN_OPERATION != 0)
    .TCPIP_MAC_Status                       = WDRV_PIC32MZW_Status,
    .TCPIP_MAC_Tasks                        = WDRV_PIC32MZW_MACTasks,
    .TCPIP_MAC_Open                         = WDRV_PIC32MZW_Open,
    .TCPIP_MAC_Close                        = WDRV_PIC32MZW_Close,
    .TCPIP_MAC_LinkCheck                    = WDRV_PIC32MZW_MACLinkCheck,
    .TCPIP_MAC_RxFilterHashTableEntrySet    = WDRV_PIC32MZW_MACRxFilterHashTableEntrySet,
    .TCPIP_MAC_PowerMode                    = WDRV_PIC32MZW_MACPowerMode,
    .TCPIP_MAC_PacketTx                     = WDRV_PIC32MZW_MACPacketTx,
    .TCPIP_MAC_PacketRx                     = WDRV_PIC32MZW_MACPacketRx,
    .TCPIP_MAC_Process                      = WDRV_PIC32MZW_MACProcess,
    .TCPIP_MAC_StatisticsGet                = WDRV_PIC32MZW_MACStatisticsGet,
    .TCPIP_MAC_ParametersGet                = WDRV_PIC32MZW_MACParametersGet,
    .TCPIP_MAC_RegisterStatisticsGet        = WDRV_PIC32MZW_MACRegisterStatisticsGet,
    .TCPIP_MAC_ConfigGet                    = WDRV_PIC32MZW_MACConfigGet,
    .TCPIP_MAC_EventMaskSet                 = WDRV_PIC32MZW_MACEventMaskSet,
    .TCPIP_MAC_EventAcknowledge             = WDRV_PIC32MZW_MACEventAcknowledge,
    .TCPIP_MAC_EventPendingGet              = WDRV_PIC32MZW_MACEventPendingGet,
};

/* Table to convert auth types to 11i info for DRV_WIFI_WID_11I_INFO. */
static const DRV_PIC32MZW_11I_MASK mapAuthTypeTo11i[] =
{
    /* WDRV_PIC32MZW_AUTH_TYPE_DEFAULT */
    0,
    /* WDRV_PIC32MZW_AUTH_TYPE_OPEN */
    0,
    /* WDRV_PIC32MZW_AUTH_TYPE_WEP */
    DRV_PIC32MZW_PRIVACY,
    /* WDRV_PIC32MZW_AUTH_TYPE_WPAWPA2_PERSONAL */
    DRV_PIC32MZW_PRIVACY
        | DRV_PIC32MZW_11I_WPAIE | DRV_PIC32MZW_11I_TKIP
        | DRV_PIC32MZW_11I_RSNE | DRV_PIC32MZW_11I_CCMP128
        | DRV_PIC32MZW_11I_BIPCMAC128
        | DRV_PIC32MZW_11I_PSK,
    /* WDRV_PIC32MZW_AUTH_TYPE_WPA2_PERSONAL */
    DRV_PIC32MZW_PRIVACY
        | DRV_PIC32MZW_11I_RSNE | DRV_PIC32MZW_11I_CCMP128
        | DRV_PIC32MZW_11I_BIPCMAC128
        | DRV_PIC32MZW_11I_PSK,
#ifdef WDRV_PIC32MZW_WPA3_PERSONAL_SUPPORT
    /* WDRV_PIC32MZW_AUTH_TYPE_WPA2WPA3_PERSONAL */
    DRV_PIC32MZW_PRIVACY
        | DRV_PIC32MZW_11I_RSNE | DRV_PIC32MZW_11I_CCMP128
        | DRV_PIC32MZW_11I_BIPCMAC128
        | DRV_PIC32MZW_11I_PSK | DRV_PIC32MZW_11I_SAE,
    /* WDRV_PIC32MZW_AUTH_TYPE_WPA3_PERSONAL */
    DRV_PIC32MZW_PRIVACY
        | DRV_PIC32MZW_11I_RSNE | DRV_PIC32MZW_11I_CCMP128
        | DRV_PIC32MZW_11I_BIPCMAC128 | DRV_PIC32MZW_11I_MFP_REQUIRED
        | DRV_PIC32MZW_11I_SAE,
#endif
#ifdef WDRV_PIC32MZW_ENTERPRISE_SUPPORT
    /* WDRV_PIC32MZW_AUTH_TYPE_WPAWPA2_ENTERPRISE */
    DRV_PIC32MZW_PRIVACY
        | DRV_PIC32MZW_11I_WPAIE
        | DRV_PIC32MZW_11I_RSNE
        | DRV_PIC32MZW_11I_TKIP
        | DRV_PIC32MZW_11I_CCMP128
        | DRV_PIC32MZW_11I_BIPCMAC128 
        | DRV_PIC32MZW_11I_1X,
    /* WDRV_PIC32MZW_AUTH_TYPE_WPA2_ENTERPRISE */
    DRV_PIC32MZW_PRIVACY
        | DRV_PIC32MZW_11I_RSNE 
        | DRV_PIC32MZW_11I_CCMP128
        | DRV_PIC32MZW_11I_BIPCMAC128
        | DRV_PIC32MZW_11I_1X,
    /* WDRV_PIC32MZW_AUTH_TYPE_WPA2WPA3_ENTERPRISE */
    DRV_PIC32MZW_PRIVACY
        | DRV_PIC32MZW_11I_RSNE 
        | DRV_PIC32MZW_11I_CCMP128
        | DRV_PIC32MZW_11I_BIPCMAC128
        | DRV_PIC32MZW_11I_1X,    
    /* WDRV_PIC32MZW_AUTH_TYPE_WPA3_ENTERPRISE */
    DRV_PIC32MZW_PRIVACY
        | DRV_PIC32MZW_11I_RSNE 
        | DRV_PIC32MZW_11I_CCMP128
        | DRV_PIC32MZW_11I_BIPCMAC128
        | DRV_PIC32MZW_11I_MFP_REQUIRED
        | DRV_PIC32MZW_11I_1X
        | DRV_PIC32MZW_11I_TD,
#endif
};

// *****************************************************************************
// *****************************************************************************
// Section: PIC32MZW MAC Driver Data Types
// *****************************************************************************
// *****************************************************************************

/* This is a structure for maintaining memory allocation information. */
typedef struct _DRV_PIC32MZW_MEM_ALLOC_HDR
{
    struct _DRV_PIC32MZW_MEM_ALLOC_HDR  *pNext;
    void                                *pUnalignedPtr;
    uint16_t                            size;
    uint8_t                             users;
    int8_t                              priLevel;
    void                                *pAllocPtr;
    uint8_t                             memory[0];
} DRV_PIC32MZW_MEM_ALLOC_HDR;

/* This is a structure for holding a queued packet. */
typedef struct
{
    DRV_PIC32MZW_MEM_ALLOC_HDR  hdr;
    uint8_t                     pkt[SHARED_PKT_MEM_BUFFER_SIZE];
} WDRV_PIC32MZW_PKT_LIST_NODE;

/* This is a structure for maintaining a list of queued packets. */
typedef struct
{
    OSAL_SEM_HANDLE_TYPE            semaphore;
    WDRV_PIC32MZW_PKT_LIST_NODE     *pHead;
    WDRV_PIC32MZW_PKT_LIST_NODE     *pTail;
} WDRV_PIC32MZW_PKT_LIST;

#ifdef DRV_PIC32MZW_TRACK_MEMORY_ALLOC
typedef struct
{
    void       *pBufferAddr;
    const char *pFuncName;
    uint32_t    line;
    uint16_t    size;
    uint8_t     users;
} WDRV_PIC32MZW_MEM_ALLOC_ENTRY;
#endif

// *****************************************************************************
// *****************************************************************************
// Section: PIC32MZW MAC Driver Global Data
// *****************************************************************************
// *****************************************************************************

/* This is the Control driver instance descriptor. */
static WDRV_PIC32MZW_CTRLDCPT pic32mzwCtrlDescriptor;

/* This is the MAC driver instance descriptor. */
static WDRV_PIC32MZW_MACDCPT pic32mzwMACDescriptor;

/* This is the queue to hold discarded receive TCP/IP packets. */
static PROTECTED_SINGLE_LIST pic32mzwDiscardQueue;

/* This is the reserved packet store. */
static WDRV_PIC32MZW_PKT_LIST_NODE pic32mzwRsrvPkts[PIC32MZW_RSR_PKT_NUM] __attribute__((coherent, aligned(PIC32MZW_CACHE_LINE_SIZE))) __attribute__((region("wlan_mem")));

/* This is the queue of reserved packets. */
static WDRV_PIC32MZW_PKT_LIST pic32mzwRsrvPktList;

/* This is the firmware to driver receive WID queue. */
static PROTECTED_SINGLE_LIST pic32mzwWIDRxQueue;

/* This is the driver to firmware transmit WID queue. */
static SINGLE_LIST pic32mzwWIDTxQueue;

/* This is the memory allocation mutex. */
static OSAL_MUTEX_HANDLE_TYPE pic32mzwMemMutex;

#ifdef WDRV_PIC32MZW_STATS_ENABLE
/* This is the memory statistics mutex. */
static OSAL_MUTEX_HANDLE_TYPE pic32mzwMemStatsMutex;

/* This is the memory statistics structure. */
static WDRV_PIC32MZW_MAC_MEM_STATISTICS pic32mzMemStatistics;
#endif

#ifdef DRV_PIC32MZW_TRACK_MEMORY_ALLOC
/* This is a flag to indicate if the memory tracker has been initialized. */
static bool pic32mzwMemTrackInit = false;

/* This is the memory tracker entries table. */
static WDRV_PIC32MZW_MEM_ALLOC_ENTRY pic32mzwMemTrackEntries[WDRV_PIC32MZW_NUM_TRACK_ENTRIES];
#endif

// *****************************************************************************
// *****************************************************************************
// Section: PIC32MZW Driver Packet List Implementation
// *****************************************************************************
// *****************************************************************************

//*******************************************************************************
/*
  Function:
    static bool _DRV_PIC32MZW_PktListInit(WDRV_PIC32MZW_PKT_LIST *pPktList)

  Summary:
    Initialises a packet list.

  Description:
    Initialises a packet list to contain no packets.

  Precondition:
    None.

  Parameters:
    pPktList - Pointer to a packet list structure.

  Returns:
    true or false - Indicating success or failure of the operation.

  Remarks:
    None.

*/

static bool _DRV_PIC32MZW_PktListInit(WDRV_PIC32MZW_PKT_LIST *pPktList)
{
    if (NULL == pPktList)
    {
        return false;
    }

    pPktList->pHead = NULL;
    pPktList->pTail = NULL;

    OSAL_SEM_Create(&pPktList->semaphore, OSAL_SEM_TYPE_BINARY, 1, 1);

    return true;
}

//*******************************************************************************
/*
  Function:
    static bool _DRV_PIC32MZW_PktListAdd
    (
        WDRV_PIC32MZW_PKT_LIST *pPktList,
        WDRV_PIC32MZW_PKT_LIST_NODE *pNode
    )

  Summary:
    Adds a packet to a packet list.

  Description:
    Adds a packet to the end of a packet list.

  Precondition:
    _DRV_PIC32MZW_PktListInit must have been called to initialise the list.

  Parameters:
    pPktList - Pointer to a packet list structure.
    pNode    - Pointer to packet list node to add.

  Returns:
    true or false - Indicating success or failure of the operation.

  Remarks:
    None.

*/

static bool _DRV_PIC32MZW_PktListAdd
(
    WDRV_PIC32MZW_PKT_LIST *pPktList,
    WDRV_PIC32MZW_PKT_LIST_NODE *pNode
)
{
    if ((NULL == pPktList) || (NULL == pNode))
    {
        return false;
    }

    if (OSAL_SEM_Pend(&pPktList->semaphore, OSAL_WAIT_FOREVER) != OSAL_RESULT_TRUE)
    {
        return false;
    }

    if (NULL == pPktList->pHead)
    {
        pPktList->pHead = pNode;
    }

    if (NULL != pPktList->pTail)
    {
        pPktList->pTail->hdr.pNext = (DRV_PIC32MZW_MEM_ALLOC_HDR*)pNode;
    }

    pPktList->pTail = pNode;

    pNode->hdr.pNext = NULL;

    if (OSAL_SEM_Post(&pPktList->semaphore) != OSAL_RESULT_TRUE)
    {
        return false;
    }

    return true;
}

//*******************************************************************************
/*
  Function:
    static WDRV_PIC32MZW_PKT_LIST_NODE* _DRV_PIC32MZW_PktListRemove
    (
        WDRV_PIC32MZW_PKT_LIST *pPktList
    )

  Summary:
    Removes a packet from a packet list.

  Description:
    Removes a packet from the head of a packet list.

  Precondition:
    _DRV_PIC32MZW_PktListInit must have been called to initialise the list.

  Parameters:
    pPktList - Pointer to a packet list structure.

  Returns:
    Pointer to removed packet node or NULL for failure.

  Remarks:
    None.

*/

static WDRV_PIC32MZW_PKT_LIST_NODE* _DRV_PIC32MZW_PktListRemove
(
    WDRV_PIC32MZW_PKT_LIST *pPktList
)
{
    WDRV_PIC32MZW_PKT_LIST_NODE *pNode;

    if (NULL == pPktList)
    {
        return NULL;
    }

    if (OSAL_SEM_Pend(&pPktList->semaphore, OSAL_WAIT_FOREVER) != OSAL_RESULT_TRUE)
    {
        return NULL;
    }

    pNode = pPktList->pHead;

    if (NULL != pPktList->pHead)
    {
        pPktList->pHead = (WDRV_PIC32MZW_PKT_LIST_NODE*)pPktList->pHead->hdr.pNext;
    }

    if (NULL == pPktList->pHead)
    {
        pPktList->pTail = NULL;
    }

    if (OSAL_SEM_Post(&pPktList->semaphore) != OSAL_RESULT_TRUE)
    {
        return NULL;
    }

    return pNode;
}

//*******************************************************************************
/*
  Function:
    static bool _DRV_PIC32MZW_PktListDeinit(WDRV_PIC32MZW_PKT_LIST *pPktList)

  Summary:
    Deinitialises a packet list.

  Description:
    Deinitialises a packet list.

  Precondition:
    _DRV_PIC32MZW_PktListInit must have been called to initialise the list.

  Parameters:
    pPktList - Pointer to a packet list structure.

  Returns:
    true or false - Indicating success or failure of the operation.

  Remarks:
    None.

*/

static bool _DRV_PIC32MZW_PktListDeinit(WDRV_PIC32MZW_PKT_LIST *pPktList)
{
    if (NULL == pPktList)
    {
        return false;
    }

    while (NULL != pPktList->pHead)
    {
        _DRV_PIC32MZW_PktListRemove(pPktList);
    }

    OSAL_SEM_Delete(&pPktList->semaphore);

    return true;
}

// *****************************************************************************
// *****************************************************************************
// Section: PIC32MZW Driver Internal Implementation
// *****************************************************************************
// *****************************************************************************

//*******************************************************************************
/*
  Function:
    static DRV_PIC32MZW_MEM_ALLOC_HDR* _DRV_PIC32MZW_MemHdr(void *pBufferAddr)

  Summary:
    Returns the memory allocation header for an allocation.

  Description:
    Returns the memory allocation header for an allocation.

  Precondition:
    None.

  Parameters:
    pBufferAddr - Pointer to the start of the memory allocation.

  Returns:
    Pointer to allocation header.

  Remarks:
    None.

*/

static DRV_PIC32MZW_MEM_ALLOC_HDR* _DRV_PIC32MZW_MemHdr(void *pBufferAddr)
{
    DRV_PIC32MZW_MEM_ALLOC_HDR *pAllocHdr;

    if (NULL == pBufferAddr)
    {
        return NULL;
    }

    pAllocHdr = pBufferAddr;
    pAllocHdr--;

    return pAllocHdr;
}

#ifdef DRV_PIC32MZW_TRACK_MEMORY_ALLOC
//*******************************************************************************
/*
  Function:
    static void _DRV_PIC32MZW_MemTrackerInit(void)

  Summary:
    Initializes a memory allocation tracker.

  Description:
    Resets the memory allocation tracker to an initial state.

  Precondition:
    None.

  Parameters:
    None.

  Returns:
    None.

  Remarks:
    This only initializes once based on a global flag.

    Only included if DRV_PIC32MZW_TRACK_MEMORY_ALLOC defined in firmware library.

*/

static void _DRV_PIC32MZW_MemTrackerInit(void)
{
    if (false == pic32mzwMemTrackInit)
    {
        memset(pic32mzwMemTrackEntries, 0, sizeof(pic32mzwMemTrackEntries));
        pic32mzwMemTrackInit = true;
    }
}

//*******************************************************************************
/*
  Function:
    static void _DRV_PIC32MZW_MemTrackerAdd
    (
        const char *pFuncName,
        uint32_t line,
        void* pBufferAddr,
        uint16_t size
    )

  Summary:
    Add an address to the memory allocation tracker.

  Description:
    Records the function name and line of the call which requested the memory.

  Precondition:
    _DRV_PIC32MZW_MemTrackerInit must have called to initialize the tracker.

  Parameters:
    pFuncName   - Pointer to function name.
    line        - Line number within function.
    pBufferAddr - Pointer to memory allocation to track.
    size        - Size of allocation to track.

  Returns:
    None.

  Remarks:
    Only included if DRV_PIC32MZW_TRACK_MEMORY_ALLOC defined in firmware library.

*/

static void _DRV_PIC32MZW_MemTrackerAdd
(
    const char *pFuncName,
    uint32_t line,
    void* pBufferAddr,
    uint16_t size
)
{
    int i;

    if ((false == pic32mzwMemTrackInit) || (NULL == pFuncName) || (NULL == pBufferAddr))
    {
        return;
    }

    for (i=0; i<WDRV_PIC32MZW_NUM_TRACK_ENTRIES; i++)
    {
        if (NULL == pic32mzwMemTrackEntries[i].pBufferAddr)
        {
            pic32mzwMemTrackEntries[i].pBufferAddr = pBufferAddr;
            pic32mzwMemTrackEntries[i].pFuncName   = pFuncName;
            pic32mzwMemTrackEntries[i].line        = line;
            pic32mzwMemTrackEntries[i].size        = size;
            pic32mzwMemTrackEntries[i].users       = 1;
            break;
        }
    }
}

//*******************************************************************************
/*
  Function:
    static void _DRV_PIC32MZW_MemTrackerRemove(void* pBufferAddr)

  Summary:
    Removes an address from the memory allocation tracker.

  Description:
    Deletes the recorded entry for the memory allocation given.

  Precondition:
    _DRV_PIC32MZW_MemTrackerInit must have called to initialize the tracker.

  Parameters:
    pBufferAddr - Pointer to memory allocation to remove.

  Returns:
    None.

  Remarks:
    Only included if DRV_PIC32MZW_TRACK_MEMORY_ALLOC defined in firmware library.

*/

static void _DRV_PIC32MZW_MemTrackerRemove(void* pBufferAddr)
{
    int i;

    if ((false == pic32mzwMemTrackInit) || (NULL == pBufferAddr))
    {
        return;
    }

    for (i=0; i<WDRV_PIC32MZW_NUM_TRACK_ENTRIES; i++)
    {
        if (pic32mzwMemTrackEntries[i].pBufferAddr == pBufferAddr)
        {
            pic32mzwMemTrackEntries[i].users--;

            if (0 == pic32mzwMemTrackEntries[i].users)
            {
                memset(&pic32mzwMemTrackEntries[i], 0, sizeof(WDRV_PIC32MZW_MEM_ALLOC_ENTRY));
                break;
            }
        }
    }
}

//*******************************************************************************
/*
  Function:
    void WDRV_PIC32MZW_MemTrackerDump(void)

  Summary:
    Dumps the active tacker entries to the debug output.

  Description:
    If an entry is still active within the tracker it is displayed on the debug output.

  Remarks:
    See wdrv_pic32mzw.h for usage information.

*/

void WDRV_PIC32MZW_MemTrackerDump(void)
{
    int i;

    if (false == pic32mzwMemTrackInit)
    {
        return;
    }

    if (true == pic32mzwDescriptor[0].isInit)
    {
        if (OSAL_RESULT_FALSE == OSAL_MUTEX_Lock(&pic32mzwMemMutex, OSAL_WAIT_FOREVER))
        {
            return;
        }
    }

    for (i=0; i<WDRV_PIC32MZW_NUM_TRACK_ENTRIES; i++)
    {
        if (NULL != pic32mzwMemTrackEntries[i].pBufferAddr)
        {
            WDRV_DBG_INFORM_PRINT("MT: %s:%d - %d\r\n", pic32mzwMemTrackEntries[i].pFuncName, pic32mzwMemTrackEntries[i].line, pic32mzwMemTrackEntries[i].size);
        }
    }

    if (true == pic32mzwDescriptor[0].isInit)
    {
        OSAL_MUTEX_Unlock(&pic32mzwMemMutex);
    }
}
#endif

//*******************************************************************************
/*
  Function:
    static WDRV_PIC32MZW_ASSOC_INFO* _WDRV_PIC32MZW_FindAssocInfoAP
    (
        WDRV_PIC32MZW_CTRLDCPT *pCtrl,
        const uint8_t *pMacAddr
    )

  Summary:
    Find an association.

  Description:
    Finds an existing association matching the MAC address supplied or
    finds an empty association if no MAC address supplied.

  Precondition:
    System interface initialization of the PIC32MZW driver.

  Parameters:
    pCtrl    - Pointer to driver control structure.
    pMacAddr - Pointer to MAC address to find or NULL.

  Returns:
    Pointer to association info structure matching.

  Remarks:
    None.

*/

static WDRV_PIC32MZW_ASSOC_INFO* _WDRV_PIC32MZW_FindAssocInfoAP
(
    WDRV_PIC32MZW_CTRLDCPT *pCtrl,
    const uint8_t *pMacAddr
)
{
    int i;
    WDRV_PIC32MZW_ASSOC_INFO *pStaAssocInfo = NULL;

    if (NULL == pCtrl)
    {
        return NULL;
    }

    if (NULL == pMacAddr)
    {
        for (i=0; i<WDRV_PIC32MZW_NUM_ASSOCS; i++)
        {
            if ((DRV_HANDLE_INVALID == pCtrl->assocInfoAP[i].handle) &&
                (false == pCtrl->assocInfoAP[i].peerAddress.valid))
            {
                pStaAssocInfo = &pCtrl->assocInfoAP[i];
                break;
            }
        }
    }
    else
    {
        for (i=0; i<WDRV_PIC32MZW_NUM_ASSOCS; i++)
        {
            if ((DRV_HANDLE_INVALID != pCtrl->assocInfoAP[i].handle) &&
                (true == pCtrl->assocInfoAP[i].peerAddress.valid) &&
                (0 == memcmp(pCtrl->assocInfoAP[i].peerAddress.addr, pMacAddr, WDRV_PIC32MZW_MAC_ADDR_LEN)))
            {
                pStaAssocInfo = &pCtrl->assocInfoAP[i];
                break;
            }
        }
    }

    return pStaAssocInfo;
}

//*******************************************************************************
/*
  Function:
    static bool _WDRV_PIC32MZW_ValidateInitData
    (
        WDRV_PIC32MZW_CTRLDCPT* const pCtrl,
        const WDRV_PIC32MZW_SYS_INIT* const pInitData
    )

  Summary:
    Validates the driver initialization structure.

  Description:
    Checks fields within the driver initialization structure and reports
    if they are valid. Copies the data to the driver descriptor.

  Precondition:
    None.

  Parameters:
    pCtrl     - Pointer to control descriptor.
    pInitData - Pointer to initialization structure.

  Returns:
    Flag indicating if structure is valid.

  Remarks:
    None.

*/

static bool _WDRV_PIC32MZW_ValidateInitData
(
    WDRV_PIC32MZW_CTRLDCPT* const pCtrl,
    const WDRV_PIC32MZW_SYS_INIT* const pInitData
)
{
    int regDomNamelength;

    if (NULL == pCtrl)
    {
        return false;
    }

    if (NULL == pInitData)
    {
        pCtrl->regDomNameLength         = 0;
        pCtrl->powerSaveMode            = WDRV_PIC32MZW_POWERSAVE_RUN_MODE;
        pCtrl->powerSavePICCorrelation  = WDRV_PIC32MZW_POWERSAVE_PIC_ASYNC_MODE;
        return true;
    }

    regDomNamelength = strlen(pInitData->pRegDomName);

    if (regDomNamelength > WDRV_PIC32MZW_REGDOMAIN_MAX_NAME_LEN)
    {
        return false;
    }

    if (pInitData->powerSaveMode > WDRV_PIC32MZW_POWERSAVE_WDS_MODE)
    {
        return false;
    }

    if ((WDRV_PIC32MZW_POWERSAVE_PIC_ASYNC_MODE != pInitData->powerSavePICCorrelation) &&
        (WDRV_PIC32MZW_POWERSAVE_PIC_SYNC_MODE != pInitData->powerSavePICCorrelation))
    {
        return false;
    }

    memset(pCtrl->regDomName, 0, WDRV_PIC32MZW_REGDOMAIN_MAX_NAME_LEN);

    DRV_PIC32MZW1_Crypto_Random_Init(pInitData->pCryptRngCtx);

    if (regDomNamelength > 0)
    {
        memcpy(pCtrl->regDomName, pInitData->pRegDomName, regDomNamelength);
    }

    pCtrl->regDomNameLength        = regDomNamelength;
    pCtrl->powerSaveMode           = pInitData->powerSaveMode;
    pCtrl->powerSavePICCorrelation = pInitData->powerSavePICCorrelation;

    return true;
}

//*******************************************************************************
/*
  Function:
    static bool _WDRV_PIC32MZW_SendInitData(WDRV_PIC32MZW_CTRLDCPT* const pCtrl)

  Summary:
    Send initialization data to firmware.

  Description:
    Packs the initialization data into WID messages and sends to firmware.

  Precondition:
    None.

  Parameters:
    pCtrl - Pointer to control descriptor.

  Returns:
    Flag indicating if send was successful.

  Remarks:
    None.

*/

static bool _WDRV_PIC32MZW_SendInitData(WDRV_PIC32MZW_CTRLDCPT* const pCtrl)
{
    DRV_PIC32MZW_WIDCTX wids;
    OSAL_CRITSECT_DATA_TYPE critSect;

    if (NULL == pCtrl)
    {
        return false;
    }

    /* Allocate memory for the WIDs. */
    DRV_PIC32MZW_MultiWIDInit(&wids, 32);

    if (pCtrl->regDomNameLength > 0)
    {
        DRV_PIC32MZW_MultiWIDAddData(&wids, DRV_WIFI_WID_REG_DOMAIN, (uint8_t*)pCtrl->regDomName, WDRV_PIC32MZW_REGDOMAIN_MAX_NAME_LEN);
    }

    DRV_PIC32MZW_MultiWIDAddValue(&wids, DRV_WIFI_WID_QOS_ENABLE, 1);
    DRV_PIC32MZW_MultiWIDAddValue(&wids, DRV_WIFI_WID_POWER_MANAGEMENT, pCtrl->powerSaveMode);
    DRV_PIC32MZW_MultiWIDAddValue(&wids, DRV_WIFI_WID_PS_CORRELATION, pCtrl->powerSavePICCorrelation);

    critSect = OSAL_CRIT_Enter(OSAL_CRIT_TYPE_LOW);

    if (false == DRV_PIC32MZW_MultiWid_Write(&wids))
    {
        OSAL_CRIT_Leave(OSAL_CRIT_TYPE_LOW, critSect);

        DRV_PIC32MZW_MultiWIDDestroy(&wids);

        WDRV_DBG_ERROR_PRINT("WID init failed\r\n");
        return false;
    }

    OSAL_CRIT_Leave(OSAL_CRIT_TYPE_LOW, critSect);

    return true;
}

//*******************************************************************************
/*
  Function:
    static bool _WDRV_PIC32MZW_SendInitQuery(WDRV_PIC32MZW_CTRLDCPT* const pCtrl)

  Summary:
    Send initialization query to firmware.

  Description:
    Packs the initialization query into WID messages and sends to firmware.

  Precondition:
    None.

  Parameters:
    pCtrl - Pointer to control descriptor.

  Returns:
    Flag indicating if send was successful.

  Remarks:
    None.

*/

static bool _WDRV_PIC32MZW_SendInitQuery(WDRV_PIC32MZW_CTRLDCPT* const pCtrl)
{
    DRV_PIC32MZW_WIDCTX wids;
    OSAL_CRITSECT_DATA_TYPE critSect;

    if (NULL == pCtrl)
    {
        return false;
    }

    /* Allocate memory for the WIDs. */
    DRV_PIC32MZW_MultiWIDInit(&wids, 32);

    DRV_PIC32MZW_MultiWIDAddQuery(&wids, DRV_WIFI_WID_MAC_ADDR);

    critSect = OSAL_CRIT_Enter(OSAL_CRIT_TYPE_LOW);

    if (false == DRV_PIC32MZW_MultiWid_Write(&wids))
    {
        OSAL_CRIT_Leave(OSAL_CRIT_TYPE_LOW, critSect);

        DRV_PIC32MZW_MultiWIDDestroy(&wids);

        WDRV_DBG_ERROR_PRINT("WID query failed\r\n");
        return false;
    }

    OSAL_CRIT_Leave(OSAL_CRIT_TYPE_LOW, critSect);

    return true;
}

//*******************************************************************************
/*
  Function:
    static bool _WDRV_PIC32MZW_SendMACInitData(WDRV_PIC32MZW_CTRLDCPT* const pCtrl)

  Summary:
    Send initialization MAC data to firmware.

  Description:
    Packs the initialization MAC data into WID messages and sends to firmware.

  Precondition:
    None.

  Parameters:
    pCtrl - Pointer to control descriptor.

  Returns:
    Flag indicating if send was successful.

  Remarks:
    None.

*/

static bool _WDRV_PIC32MZW_SendMACInitData(WDRV_PIC32MZW_CTRLDCPT* const pCtrl)
{
    DRV_PIC32MZW_WIDCTX wids;
    OSAL_CRITSECT_DATA_TYPE critSect;

    if (NULL == pCtrl)
    {
        return false;
    }

    /* Allocate memory for the WIDs. */
    DRV_PIC32MZW_MultiWIDInit(&wids, 32);

    if (true == pCtrl->macAddr.valid)
    {
        DRV_PIC32MZW_MultiWIDAddData(&wids, DRV_WIFI_WID_MAC_ADDR, pCtrl->macAddr.addr, 6);
    }

    critSect = OSAL_CRIT_Enter(OSAL_CRIT_TYPE_LOW);

    if (false == DRV_PIC32MZW_MultiWid_Write(&wids))
    {
        OSAL_CRIT_Leave(OSAL_CRIT_TYPE_LOW, critSect);

        DRV_PIC32MZW_MultiWIDDestroy(&wids);

        WDRV_DBG_ERROR_PRINT("WID MAC init failed\r\n");
        return false;
    }

    OSAL_CRIT_Leave(OSAL_CRIT_TYPE_LOW, critSect);

    return true;
}

// *****************************************************************************
// *****************************************************************************
// Section: PIC32MZW Driver Information Implementation
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
// *****************************************************************************
// Section: PIC32MZW Driver System Implementation
// *****************************************************************************
// *****************************************************************************

//*******************************************************************************
/*
  Function:
    SYS_MODULE_OBJ WDRV_PIC32MZW_Initialize
    (
        const SYS_MODULE_INDEX index,
        const SYS_MODULE_INIT *const init
    )

  Summary:
    System interface initialization of the PIC32MZW driver.

  Description:
    This is the function that initializes the PIC32MZW driver. It is called by
    the system.

  Remarks:
    See wdrv_pic32mzw_api.h for usage information.

*/

SYS_MODULE_OBJ WDRV_PIC32MZW_Initialize
(
    const SYS_MODULE_INDEX index,
    const SYS_MODULE_INIT *const init
)
{
    WDRV_PIC32MZW_DCPT *pDcpt;

    if (WDRV_PIC32MZW_SYS_IDX_0 == index)
    {
        const WDRV_PIC32MZW_SYS_INIT* const pInitData = (const WDRV_PIC32MZW_SYS_INIT* const)init;
        DRV_PIC32MZW_INIT drvInitData;
        int i;

#ifdef DRV_PIC32MZW_TRACK_MEMORY_ALLOC
        _DRV_PIC32MZW_MemTrackerInit();
#endif
        pDcpt = &pic32mzwDescriptor[0];

        if (true == pDcpt->isInit)
        {
            return (SYS_MODULE_OBJ)pDcpt;
        }

        if (false == _WDRV_PIC32MZW_ValidateInitData(&pic32mzwCtrlDescriptor, pInitData))
        {
            return SYS_MODULE_OBJ_INVALID;
        }

        PMUCLKCTRLbits.WLDOOFF = 0;

#ifndef WDRV_PIC32MZW1_DEVICE_USE_SYS_DEBUG
        pfPIC32MZWDebugPrintCb = NULL;
#endif
        pic32mzwCtrlDescriptor.handle = DRV_HANDLE_INVALID;
#ifdef WDRV_PIC32MZW_ENTERPRISE_SUPPORT
        pic32mzwCtrlDescriptor.tlsHandle = DRV_PIC32MZW1_TLS_HANDLE_INVALID;
#endif
        OSAL_SEM_Create(&pic32mzwCtrlDescriptor.drvAccessSemaphore, OSAL_SEM_TYPE_BINARY, 1, 1);

        OSAL_SEM_Create(&pic32mzwCtrlDescriptor.drvEventSemaphore, OSAL_SEM_TYPE_COUNTING, 10, 0);

        SYS_INT_SourceEnable(INT_SOURCE_RFMAC);
        SYS_INT_SourceEnable(INT_SOURCE_RFTM0);
        SYS_INT_SourceEnable(INT_SOURCE_RFSMC);

        TCPIP_Helper_ProtectedSingleListInitialize(&pic32mzwWIDRxQueue);
        TCPIP_Helper_SingleListInitialize(&pic32mzwWIDTxQueue);

        OSAL_MUTEX_Create(&pic32mzwMemMutex);

        pic32mzwCtrlDescriptor.rfMacConfigStatus = 0;
        
        pic32mzwCtrlDescriptor.vendorIEMask = 0;

#ifdef WDRV_PIC32MZW_STATS_ENABLE
        memset(&pic32mzMemStatistics, 0, sizeof(pic32mzMemStatistics));
        OSAL_MUTEX_Create(&pic32mzwMemStatsMutex);
#endif
        drvInitData.alarm_1ms = WDRV_PIC32MZW_ALARM_PERIOD_1MS;
        drvInitData.alarm_max = WDRV_PIC32MZW_ALARM_PERIOD_MAX;

        pic32mzwCtrlDescriptor.macAddr.valid = false;
        memset(pic32mzwCtrlDescriptor.macAddr.addr, 0, WDRV_PIC32MZW_MAC_ADDR_LEN);

        pic32mzwCtrlDescriptor.isAP             = false;
        pic32mzwCtrlDescriptor.connectedState   = WDRV_PIC32MZW_CONN_STATE_DISCONNECTED;
        pic32mzwCtrlDescriptor.scanInProgress   = false;
        pic32mzwCtrlDescriptor.opChannel        = WDRV_PIC32MZW_CID_ANY;
        
        pic32mzwCtrlDescriptor.powerSaveMode           = WDRV_PIC32MZW_POWERSAVE_RUN_MODE;
        pic32mzwCtrlDescriptor.powerSavePICCorrelation = WDRV_PIC32MZW_POWERSAVE_PIC_ASYNC_MODE;

        pic32mzwCtrlDescriptor.assocInfoSTA.handle              = DRV_HANDLE_INVALID;
        pic32mzwCtrlDescriptor.assocInfoSTA.rssi                = 0;
        pic32mzwCtrlDescriptor.assocInfoSTA.peerAddress.valid   = false;
        pic32mzwCtrlDescriptor.assocInfoSTA.transitionDisable   = false;
        pic32mzwCtrlDescriptor.assocInfoSTA.assocID             = 1;

        for (i=0; i<WDRV_PIC32MZW_NUM_ASSOCS; i++)
        {
            pic32mzwCtrlDescriptor.assocInfoAP[i].handle            = DRV_HANDLE_INVALID;
            pic32mzwCtrlDescriptor.assocInfoAP[i].peerAddress.valid = false;
            pic32mzwCtrlDescriptor.assocInfoAP[i].transitionDisable = false;
            pic32mzwCtrlDescriptor.assocInfoAP[i].assocID           = -1;
        }

        wdrv_pic32mzw_init(&drvInitData);
    }
    else if (TCPIP_MODULE_MAC_PIC32MZW1 == index)
    {
        int i;
        const TCPIP_MAC_MODULE_CTRL *pStackInitData;
        bool initMacAddrValid = false;

        if (NULL == init)
        {
            return SYS_MODULE_OBJ_INVALID;
        }

        pStackInitData = ((TCPIP_MAC_INIT *)init)->macControl;

        if (NULL == pStackInitData)
        {
            return SYS_MODULE_OBJ_INVALID;
        }

        pDcpt = &pic32mzwDescriptor[1];

        if (true == pDcpt->isInit)
        {
            return (SYS_MODULE_OBJ)pDcpt;
        }

        TCPIP_Helper_ProtectedSingleListInitialize(&pic32mzwMACDescriptor.ethRxPktList);

        TCPIP_Helper_ProtectedSingleListInitialize(&pic32mzwDiscardQueue);

        if (true == _DRV_PIC32MZW_PktListInit(&pic32mzwRsrvPktList))
        {
            for (i=0; i<PIC32MZW_RSR_PKT_NUM; i++)
            {
                _DRV_PIC32MZW_PktListAdd(&pic32mzwRsrvPktList, &pic32mzwRsrvPkts[i]);
            }
        }

        pic32mzwMACDescriptor.handle       = DRV_HANDLE_INVALID;

        pic32mzwMACDescriptor.eventF       = pStackInitData->eventF;
        pic32mzwMACDescriptor.pktAllocF    = pStackInitData->pktAllocF;
        pic32mzwMACDescriptor.pktFreeF     = pStackInitData->pktFreeF;
        pic32mzwMACDescriptor.pktAckF      = pStackInitData->pktAckF;
        pic32mzwMACDescriptor.eventParam   = pStackInitData->eventParam;
        pic32mzwMACDescriptor.eventMask    = 0;
        pic32mzwMACDescriptor.events       = 0;
        OSAL_SEM_Create(&pic32mzwMACDescriptor.eventSemaphore, OSAL_SEM_TYPE_BINARY, 1, 1);

        for (i=0; i<6; i++)
        {
            if (0 != pStackInitData->ifPhyAddress.v[i])
            {
                initMacAddrValid = true;
            }
        }

        if (true == initMacAddrValid)
        {
            if ((false == pic32mzwCtrlDescriptor.macAddr.valid) || (0 != memcmp(pStackInitData->ifPhyAddress.v, pic32mzwCtrlDescriptor.macAddr.addr, 6)))
            {
                memcpy(pic32mzwCtrlDescriptor.macAddr.addr, pStackInitData->ifPhyAddress.v, 6);
                pic32mzwCtrlDescriptor.macAddr.valid = true;
            }
        }
    }
    else
    {
        return SYS_MODULE_OBJ_INVALID;
    }

    /* Set initial state. */
    pDcpt->isInit  = true;
    pDcpt->isOpen  = false;
    pDcpt->sysStat = SYS_STATUS_BUSY;
    pDcpt->pCtrl   = &pic32mzwCtrlDescriptor;
    pDcpt->pMac    = &pic32mzwMACDescriptor;

    return (SYS_MODULE_OBJ)pDcpt;
}

//*******************************************************************************
/*
  Function:
    void WDRV_PIC32MZW_Deinitialize(SYS_MODULE_OBJ object)

  Summary:
    PIC32MZW driver deinitialization function.

  Description:
    This is the function that deinitializes the PIC32MZW.
    It is called by the system.

  Remarks:
    See wdrv_pic32mzw_api.h for usage information.

*/

void WDRV_PIC32MZW_Deinitialize(SYS_MODULE_OBJ object)
{
    WDRV_PIC32MZW_DCPT *const pDcpt = (WDRV_PIC32MZW_DCPT *const)object;

    if ((SYS_MODULE_OBJ_INVALID == object) || (NULL == pDcpt))
    {
        return;
    }

    if (false == pDcpt->isInit)
    {
        return;
    }

    pDcpt->sysStat = SYS_STATUS_UNINITIALIZED;

    if (pDcpt == &pic32mzwDescriptor[0])
    {
        SYS_INT_SourceDisable(INT_SOURCE_RFMAC);
        SYS_INT_SourceDisable(INT_SOURCE_RFTM0);
        SYS_INT_SourceDisable(INT_SOURCE_RFSMC);

        OSAL_SEM_Post(&pic32mzwCtrlDescriptor.drvEventSemaphore);
    }
#if (TCPIP_STACK_MAC_DOWN_OPERATION != false)
    else if (pDcpt == &pic32mzwDescriptor[1])
    {
        OSAL_SEM_Delete(&pic32mzwMACDescriptor.eventSemaphore);

        pic32mzwMACDescriptor.eventF       = NULL;
        pic32mzwMACDescriptor.pktAllocF    = NULL;
        pic32mzwMACDescriptor.pktFreeF     = NULL;
        pic32mzwMACDescriptor.pktAckF      = NULL;

        _DRV_PIC32MZW_PktListDeinit(&pic32mzwRsrvPktList);

        pDcpt->isInit = false;
    }
#endif  // (TCPIP_STACK_MAC_DOWN_OPERATION != false)

    /* Clear internal state. */
    pDcpt->isOpen = false;
}

//*******************************************************************************
/*
  Function:
    void WDRV_PIC32MZW_Reinitialize
    (
        SYS_MODULE_OBJ object,
        const SYS_MODULE_INIT *const init
    )

  Summary:
    PIC32MZW driver reinitialization function.

  Description:
    This is the function that re-initializes the PIC32MZW.
    It is called by the system.

  Remarks:
    See wdrv_pic32mzw_api.h for usage information.

*/

void WDRV_PIC32MZW_Reinitialize
(
    SYS_MODULE_OBJ object,
    const SYS_MODULE_INIT *const init
)
{
    WDRV_PIC32MZW_DCPT *const pDcpt = (WDRV_PIC32MZW_DCPT *const)object;

    if ((SYS_MODULE_OBJ_INVALID == object) || (NULL == pDcpt) || (NULL == pDcpt->pCtrl))
    {
        return;
    }

    if (false == pDcpt->isInit)
    {
        return;
    }

    if (pDcpt == &pic32mzwDescriptor[0])
    {
        const WDRV_PIC32MZW_SYS_INIT* const pInitData = (const WDRV_PIC32MZW_SYS_INIT* const)init;

        if (NULL != pInitData)
        {
            if (false == _WDRV_PIC32MZW_ValidateInitData(pDcpt->pCtrl, pInitData))
            {
                return;
            }

            if (false == _WDRV_PIC32MZW_SendInitData(&pic32mzwCtrlDescriptor))
            {
                return;
            }

            if (false == _WDRV_PIC32MZW_SendInitQuery(&pic32mzwCtrlDescriptor))
            {
                return;
            }
        }
    }
#if (TCPIP_STACK_MAC_DOWN_OPERATION != false)
    else if (pDcpt == &pic32mzwDescriptor[1])
    {
    }
#endif  // (TCPIP_STACK_MAC_DOWN_OPERATION != false)
}

//*******************************************************************************
/*
  Function:
    SYS_STATUS WDRV_PIC32MZW_Status(SYS_MODULE_OBJ object)

  Summary:
    Provides the current status of the PIC32MZW driver module.

  Description:
    This function provides the current status of the PIC32MZW driver module.

  Remarks:
    See wdrv_pic32mzw_api.h for usage information.

*/

SYS_STATUS WDRV_PIC32MZW_Status(SYS_MODULE_OBJ object)
{
    WDRV_PIC32MZW_DCPT *const pDcpt = (WDRV_PIC32MZW_DCPT *const)object;

    if ((SYS_MODULE_OBJ_INVALID == object) || (NULL == pDcpt))
    {
        return SYS_STATUS_ERROR;
    }

    if (true == pDcpt->isInit)
    {
        if (SYS_STATUS_UNINITIALIZED == pDcpt->sysStat)
        {
            return SYS_STATUS_BUSY;
        }

        if (pDcpt == &pic32mzwDescriptor[0])
        {
            if ((TCPIP_Helper_SingleListCount(&pic32mzwWIDTxQueue) > 0) ||
                    (TCPIP_Helper_ProtectedSingleListCount(&pic32mzwWIDRxQueue) > 0))
            {
                return SYS_STATUS_BUSY;
            }
        }
    }

    return pDcpt->sysStat;
}

//*******************************************************************************
/*
  Function:
    WDRV_PIC32MZW_SYS_STATUS WDRV_PIC32MZW_StatusExt(SYS_MODULE_OBJ object)

  Summary:
    Provides the extended system status of the PIC32MZW driver module.

  Description:
    This function provides the extended system status of the PIC32MZW driver
    module.

  Remarks:
    See wdrv_pic32mzw.h for usage information.

*/

WDRV_PIC32MZW_SYS_STATUS WDRV_PIC32MZW_StatusExt(SYS_MODULE_OBJ object)
{
    WDRV_PIC32MZW_DCPT *const pDcpt = (WDRV_PIC32MZW_DCPT *const)object;

    if ((SYS_MODULE_OBJ_INVALID == object) || (NULL == pDcpt))
    {
        return SYS_STATUS_ERROR;
    }

    if (NULL == pDcpt->pCtrl)
    {
        return SYS_STATUS_ERROR;
    }

    if (SYS_STATUS_READY_EXTENDED == pDcpt->sysStat)
    {
       return pDcpt->pCtrl->extSysStat;
    }

    /* If not in extended state, just return normal status. */
    return (WDRV_PIC32MZW_SYS_STATUS)pDcpt->sysStat;
}

//*******************************************************************************
/*
  Function:
    void WDRV_PIC32MZW_MACTasks(SYS_MODULE_OBJ object)

  Summary:
    Maintains the PIC32MZW MAC drivers state machine.

  Description:
    This function is used to maintain the driver's internal state machine.

  Remarks:
    See wdrv_pic32mzw_api.h for usage information.

*/

void WDRV_PIC32MZW_MACTasks(SYS_MODULE_OBJ object)
{
    WDRV_PIC32MZW_DCPT *const pDcpt = (WDRV_PIC32MZW_DCPT *const)object;

    if ((SYS_MODULE_OBJ_INVALID == object) || (NULL == pDcpt))
    {
        return;
    }

    switch (pDcpt->sysStat)
    {
        /* Uninitialised state. */
        case SYS_STATUS_UNINITIALIZED:
        {
            if (true == pDcpt->isInit)
            {
                pDcpt->isInit = false;
            }

            break;
        }

        case SYS_STATUS_BUSY:
        {
            if (false == pic32mzwCtrlDescriptor.macAddr.valid)
            {
                break;
            }

            if (SYS_STATUS_READY == pic32mzwDescriptor[0].sysStat)
            {
                if (false == _WDRV_PIC32MZW_SendMACInitData(&pic32mzwCtrlDescriptor))
                {
                    break;
                }

                pDcpt->sysStat = SYS_STATUS_READY;
            }
            break;
        }

        /* Running steady state. */
        case SYS_STATUS_READY:
        {
            break;
        }

        /* Error state.*/
        case SYS_STATUS_ERROR:
        {
            break;
        }

        default:
        {
            pDcpt->sysStat = SYS_STATUS_ERROR;
            break;
        }
    }
}

//*******************************************************************************
/*
  Function:
    void WDRV_PIC32MZW_Tasks(SYS_MODULE_OBJ object)

  Summary:
    Maintains the PIC32MZW drivers state machine.

  Description:
    This function is used to maintain the driver's internal state machine.

  Remarks:
    See wdrv_pic32mzw_api.h for usage information.

*/

void WDRV_PIC32MZW_Tasks(SYS_MODULE_OBJ object)
{
    WDRV_PIC32MZW_DCPT *const pDcpt = (WDRV_PIC32MZW_DCPT *const)object;
    DRV_PIC32MZW_MEM_ALLOC_HDR *pAllocHdr;

    if ((SYS_MODULE_OBJ_INVALID == object) || (NULL == pDcpt) || (NULL == pDcpt->pMac))
    {
        return;
    }

    switch (pDcpt->sysStat)
    {
        /* Uninitialised state. */
        case SYS_STATUS_UNINITIALIZED:
        {
            if (true == pDcpt->isInit)
            {
                if (OSAL_RESULT_TRUE == OSAL_SEM_Pend(&pic32mzwCtrlDescriptor.drvAccessSemaphore, 0))
                {
                    wdrv_pic32mzw_user_stop();
                    OSAL_SEM_Post(&pic32mzwCtrlDescriptor.drvAccessSemaphore);
                }

                while (TCPIP_Helper_ProtectedSingleListCount(&pic32mzwWIDRxQueue) > 0)
                {
                    pAllocHdr = (DRV_PIC32MZW_MEM_ALLOC_HDR*)TCPIP_Helper_ProtectedSingleListHeadRemove(&pic32mzwWIDRxQueue);

                    if (NULL != pAllocHdr)
                    {
                        DRV_PIC32MZW_ProcessHostRsp(pAllocHdr->memory);
                    }
                }

                OSAL_SEM_Delete(&pic32mzwCtrlDescriptor.drvEventSemaphore);
                OSAL_SEM_Delete(&pic32mzwCtrlDescriptor.drvAccessSemaphore);

                OSAL_MUTEX_Delete(&pic32mzwMemMutex);
#ifdef WDRV_PIC32MZW_STATS_ENABLE
                OSAL_MUTEX_Delete(&pic32mzwMemStatsMutex);
#endif
                PMUCLKCTRLbits.WLDOOFF = 1;

                pDcpt->isInit = false;
            }

            break;
        }

        case SYS_STATUS_BUSY:
        {
            if (OSAL_RESULT_TRUE == OSAL_SEM_Pend(&pic32mzwCtrlDescriptor.drvAccessSemaphore, 0))
            {
                wdrv_pic32mzw_user_main();
                OSAL_SEM_Post(&pic32mzwCtrlDescriptor.drvAccessSemaphore);

                pDcpt->sysStat = SYS_STATUS_READY_EXTENDED;

                if (false == _WDRV_PIC32MZW_SendInitData(&pic32mzwCtrlDescriptor))
                {
                    break;
                }

                if (false == _WDRV_PIC32MZW_SendInitQuery(&pic32mzwCtrlDescriptor))
                {
                    break;
                }

                if (pic32mzwCtrlDescriptor.regDomNameLength > 0)
                {
                    pic32mzwCtrlDescriptor.extSysStat = WDRV_PIC32MZW_SYS_STATUS_RF_INIT_BUSY;
                }
                else
                {
                    pic32mzwCtrlDescriptor.extSysStat = WDRV_PIC32MZW_SYS_STATUS_RF_CONF_MISSING;
                }
            }

            break;
        }

        /* Running steady state. */
        case SYS_STATUS_READY:
        case SYS_STATUS_READY_EXTENDED:
        {
            int numDiscard;

            OSAL_SEM_Pend(&pic32mzwCtrlDescriptor.drvEventSemaphore, OSAL_WAIT_FOREVER);

            if (SYS_STATUS_UNINITIALIZED == pDcpt->sysStat)
            {
                break;
            }

            if (OSAL_RESULT_TRUE == OSAL_SEM_Pend(&pic32mzwCtrlDescriptor.drvAccessSemaphore, 0))
            {
                if (TCPIP_Helper_SingleListCount(&pic32mzwWIDTxQueue) > 0)
                {
                    OSAL_CRITSECT_DATA_TYPE critSect;

                    critSect = OSAL_CRIT_Enter(OSAL_CRIT_TYPE_LOW);

                    pAllocHdr = (DRV_PIC32MZW_MEM_ALLOC_HDR*)TCPIP_Helper_SingleListHeadRemove(&pic32mzwWIDTxQueue);

                    OSAL_CRIT_Leave(OSAL_CRIT_TYPE_LOW, critSect);

                    if (NULL != pAllocHdr)
                    {
                        wdrv_pic32mzw_process_cfg_message(pAllocHdr->memory);
                    }
                }

                wdrv_pic32mzw_mac_controller_task();

                OSAL_SEM_Post(&pic32mzwCtrlDescriptor.drvAccessSemaphore);
            }

            if (TCPIP_Helper_ProtectedSingleListCount(&pic32mzwWIDRxQueue) > 0)
            {
                pAllocHdr = (DRV_PIC32MZW_MEM_ALLOC_HDR*)TCPIP_Helper_ProtectedSingleListHeadRemove(&pic32mzwWIDRxQueue);

                if (NULL != pAllocHdr)
                {
                    DRV_PIC32MZW_ProcessHostRsp(pAllocHdr->memory);
                }
            }

            numDiscard = TCPIP_Helper_ProtectedSingleListCount(&pic32mzwDiscardQueue);

            while (numDiscard--)
            {
                TCPIP_MAC_PACKET* ptrPacket;

                ptrPacket = (TCPIP_MAC_PACKET*)TCPIP_Helper_ProtectedSingleListHeadRemove(&pic32mzwDiscardQueue);

                if (NULL != ptrPacket)
                {
                    if (0 == (ptrPacket->pktFlags & TCPIP_MAC_PKT_FLAG_QUEUED))
                    {
                        if (NULL != pDcpt->pMac->pktFreeF)
                        {
                            pDcpt->pMac->pktFreeF(ptrPacket);
                        }
                    }
                    else
                    {
                        TCPIP_Helper_ProtectedSingleListTailAdd(&pic32mzwDiscardQueue, (SGL_LIST_NODE*)ptrPacket);
                    }
                }
            }

            break;
        }

        /* Error state.*/
        case SYS_STATUS_ERROR:
        {
            break;
        }

        default:
        {
            pDcpt->sysStat = SYS_STATUS_ERROR;
            break;
        }
    }
}

// *****************************************************************************
// *****************************************************************************
// Section: PIC32MZW Driver Client Implementation
// *****************************************************************************
// *****************************************************************************

//*******************************************************************************
/*
  Function:
    void WDRV_PIC32MZW_DebugRegisterCallback
    (
        WDRV_PIC32MZW_DEBUG_PRINT_CALLBACK const pfDebugPrintCallback
    )

  Summary:
    Register callback for debug serial stream.

  Description:
    The debug serial stream provides a printf-like stream of messages from within
    the PIC32MZW driver. The caller can provide a function to be called when
    output is available.

  Remarks:
    See wdrv_pic32mzw.h for usage information.

 */

#ifndef WDRV_PIC32MZW1_DEVICE_USE_SYS_DEBUG
void WDRV_PIC32MZW_DebugRegisterCallback
(
    WDRV_PIC32MZW_DEBUG_PRINT_CALLBACK const pfDebugPrintCallback
)
{
    pfPIC32MZWDebugPrintCb = pfDebugPrintCallback;
}
#endif

//*******************************************************************************
/*
  Function:
    DRV_HANDLE WDRV_PIC32MZW_Open(const SYS_MODULE_INDEX index, const DRV_IO_INTENT intent)

  Summary:
    Opens an instance of the PIC32MZW driver.

  Description:
    Opens an instance of the PIC32MZW driver and returns a handle which must be
    used when calling other driver functions.

  Remarks:
    See wdrv_pic32mzw.h for usage information.

*/

DRV_HANDLE WDRV_PIC32MZW_Open(const SYS_MODULE_INDEX index, const DRV_IO_INTENT intent)
{
    WDRV_PIC32MZW_DCPT *pDcpt = NULL;

    if (WDRV_PIC32MZW_SYS_IDX_0 == index)
    {
        pDcpt = &pic32mzwDescriptor[0];

        if (NULL == pDcpt->pCtrl)
        {
            return DRV_HANDLE_INVALID;
        }
    }
    else if (TCPIP_MODULE_MAC_PIC32MZW1 == index)
    {
        pDcpt = &pic32mzwDescriptor[1];

        if (NULL == pDcpt->pMac)
        {
            return DRV_HANDLE_INVALID;
        }
    }
    else
    {
        return DRV_HANDLE_INVALID;
    }

    /* Check that the driver has been initialised. */
    if (false == pDcpt->isInit)
    {
        return DRV_HANDLE_INVALID;
    }

    /* Check if the driver has already been opened. */
    if (true == pDcpt->isOpen)
    {
        return DRV_HANDLE_INVALID;
    }

    pDcpt->isOpen = true;

    if (WDRV_PIC32MZW_SYS_IDX_0 == index)
    {
        pDcpt->pCtrl->handle                = (DRV_HANDLE)pDcpt;
        pDcpt->pCtrl->scanActiveScanTime    = DRV_PIC32MZW_DEFAULT_ACTIVE_SCAN_TIME;
        pDcpt->pCtrl->scanPassiveListenTime = DRV_PIC32MZW_DEFAULT_PASSIVE_SCAN_TIME;
        pDcpt->pCtrl->scanNumSlots          = DRV_PIC32MZW_DEFAULT_SCAN_NUM_SLOT;
        pDcpt->pCtrl->scanNumProbes         = DRV_PIC32MZW_DEFAULT_SCAN_NUM_PROBE;
        pDcpt->pCtrl->pfBSSFindNotifyCB     = NULL;
        pDcpt->pCtrl->pfConnectNotifyCB     = NULL;
        pDcpt->pCtrl->pfAssociationRSSICB   = NULL;
        pDcpt->pCtrl->pfRegDomCB            = NULL;
        pDcpt->pCtrl->pfVendorIERxCB        = NULL;
        pDcpt->pCtrl->pfPSNotifyCB          = NULL;
    }
    else
    {
        pDcpt->pMac->handle = (DRV_HANDLE)pDcpt;
    }

    return (DRV_HANDLE)pDcpt;
}

//*******************************************************************************
/*
  Function:
    void WDRV_PIC32MZW_Close(DRV_HANDLE handle)

  Summary:
    Closes an instance of the PIC32MZW driver.

  Description:
    This is the function that closes an instance of the MAC.
    All per client data is released and the handle can no longer be used
    after this function is called.

  Remarks:
    See wdrv_pic32mzw.h for usage information.

*/

void WDRV_PIC32MZW_Close(DRV_HANDLE handle)
{
    WDRV_PIC32MZW_DCPT *const pDcpt = (WDRV_PIC32MZW_DCPT *const)handle;

    /* Ensure the driver handle is valid. */
    if ((DRV_HANDLE_INVALID == handle) || (NULL == pDcpt) || (NULL == pDcpt->pCtrl))
    {
        return;
    }

    if (handle == pDcpt->pCtrl->handle)
    {
        pDcpt->pCtrl->pfBSSFindNotifyCB     = NULL;
        pDcpt->pCtrl->pfConnectNotifyCB     = NULL;
        pDcpt->pCtrl->pfAssociationRSSICB   = NULL;
        pDcpt->pCtrl->pfRegDomCB            = NULL;
        pDcpt->pCtrl->pfVendorIERxCB        = NULL;
        pDcpt->pCtrl->pfPSNotifyCB          = NULL;
    }

    pDcpt->isOpen = false;
}
//*******************************************************************************
/*
  Function:
    WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_PMKCacheFlush

  Summary:
    Flush the PMK cache.

  Description:
    Removes all entries from the local PMK cache.

  Remarks:
    See wdrv_pic32mzw.h for usage information.

*/

WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_PMKCacheFlush
(
    DRV_HANDLE handle
)
{
    WDRV_PIC32MZW_DCPT *const pDcpt = (WDRV_PIC32MZW_DCPT *const)handle;
    DRV_PIC32MZW_WIDCTX wids;
    OSAL_CRITSECT_DATA_TYPE critSect;

    /* Ensure the driver handle is valid. */
    if ((DRV_HANDLE_INVALID == handle) || (NULL == pDcpt) || (NULL == pDcpt->pCtrl))
    {
        return WDRV_PIC32MZW_STATUS_INVALID_ARG;
    }

    /* Ensure the driver instance has been opened for use. */
    if ((false == pDcpt->isOpen) || (NULL == pDcpt->pCtrl) || (DRV_HANDLE_INVALID == pDcpt->pCtrl->handle))
    {
        return WDRV_PIC32MZW_STATUS_NOT_OPEN;
    }

    /* Allocate memory for the WIDs. */
    DRV_PIC32MZW_MultiWIDInit(&wids, 16);

    /* Flush PMK cache. */
    DRV_PIC32MZW_MultiWIDAddValue(&wids, DRV_WIFI_WID_PMK_CACHE_FLUSH, 1);

    critSect = OSAL_CRIT_Enter(OSAL_CRIT_TYPE_LOW);

    /* Write the WIDs. */
    if (false == DRV_PIC32MZW_MultiWid_Write(&wids))
    {
        OSAL_CRIT_Leave(OSAL_CRIT_TYPE_LOW, critSect);

        DRV_PIC32MZW_MultiWIDDestroy(&wids);

        return WDRV_PIC32MZW_STATUS_REQUEST_ERROR;
    }

    OSAL_CRIT_Leave(OSAL_CRIT_TYPE_LOW, critSect);

    return WDRV_PIC32MZW_STATUS_OK;
}

//*******************************************************************************
/*
  Function:
    void WDRV_PIC32MZW_GetStatistics
    (
        DRV_HANDLE handle,
        WDRV_PIC32MZW_MAC_MEM_STATISTICS *pStats
    );

  Summary:
    Retrieves the static data of the PIC32MZW.

  Description:
    Retrieves the static data of the PIC32MZW..

  Remarks:
    See wdrv_pic32mzw.h for usage information.

 */

#ifdef WDRV_PIC32MZW_STATS_ENABLE
WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_GetStatistics
(
    DRV_HANDLE handle,
    WDRV_PIC32MZW_MAC_MEM_STATISTICS *pStats
)
{
    WDRV_PIC32MZW_DCPT *const pDcpt = (WDRV_PIC32MZW_DCPT *const)handle;

    if ((DRV_HANDLE_INVALID == handle) || (NULL == pDcpt) || (NULL == pStats))
    {
        return WDRV_PIC32MZW_STATUS_INVALID_ARG;
    }

    if (OSAL_RESULT_TRUE == OSAL_MUTEX_Lock(&pic32mzwMemStatsMutex, OSAL_WAIT_FOREVER))
    {
        memcpy(pStats, &pic32mzMemStatistics, sizeof(WDRV_PIC32MZW_MAC_MEM_STATISTICS));
        OSAL_MUTEX_Unlock(&pic32mzwMemStatsMutex);
    }

    return WDRV_PIC32MZW_STATUS_OK;
}
#endif

// *****************************************************************************
// *****************************************************************************
// Section: PIC32MZW MAC Driver Implementation
// *****************************************************************************
// *****************************************************************************

//*******************************************************************************
/*
  Function:
    bool WDRV_PIC32MZW_MACLinkCheck(DRV_HANDLE handle)

  Summary:
    Indicates the state of the network link.

  Description:
    Returns a flag indicating if the network link is active or not.

  Remarks:
    See wdrv_pic32mzw_mac.h for usage information.

*/

bool WDRV_PIC32MZW_MACLinkCheck(DRV_HANDLE handle)
{
    WDRV_PIC32MZW_DCPT *const pDcpt = (WDRV_PIC32MZW_DCPT *const)handle;

    /* Ensure the driver handle is valid. */
    if ((DRV_HANDLE_INVALID == handle) || (NULL == pDcpt) || (NULL == pDcpt->pCtrl))
    {
        return false;
    }

    return (WDRV_PIC32MZW_CONN_STATE_CONNECTED == pDcpt->pCtrl->connectedState);
}

//*******************************************************************************
/*
  Function:
    TCPIP_MAC_RES WDRV_PIC32MZW_MACRxFilterHashTableEntrySet
    (
        DRV_HANDLE handle,
        const TCPIP_MAC_ADDR* DestMACAddr
    )

  Summary:
    Adds an entry to the receive multicast filter.

  Description:

  Remarks:
    See wdrv_pic32mzw_mac.h for usage information.

*/

TCPIP_MAC_RES WDRV_PIC32MZW_MACRxFilterHashTableEntrySet
(
    DRV_HANDLE handle,
    const TCPIP_MAC_ADDR* DestMACAddr
)
{
    if ((DRV_HANDLE_INVALID == handle) || (NULL == DestMACAddr))
    {
        return TCPIP_MAC_RES_OP_ERR;
    }

    return TCPIP_MAC_RES_OK;
}

//*******************************************************************************
/*
  Function:
    bool WDRV_PIC32MZW_MACPowerMode(DRV_HANDLE handle, TCPIP_MAC_POWER_MODE pwrMode)

  Summary:
    Change the power mode.

  Description:
    Not currently supported.

  Remarks:
    See wdrv_pic32mzw_mac.h for usage information.

*/

bool WDRV_PIC32MZW_MACPowerMode(DRV_HANDLE handle, TCPIP_MAC_POWER_MODE pwrMode)
{
    return true;
}

//*******************************************************************************
/*
  Function:
    TCPIP_MAC_RES WDRV_PIC32MZW_MACPacketTx(DRV_HANDLE handle, TCPIP_MAC_PACKET* ptrPacket)

  Summary:
    Send an Ethernet frame via the PIC32MZW.

  Description:
    Takes an Ethernet frame from the TCP/IP stack and schedules it with the
      PIC32MZW.

  Remarks:
    See wdrv_pic32mzw_mac.h for usage information.

*/

TCPIP_MAC_RES WDRV_PIC32MZW_MACPacketTx(DRV_HANDLE handle, TCPIP_MAC_PACKET* ptrPacket)
{
    WDRV_PIC32MZW_DCPT *const pDcpt = (WDRV_PIC32MZW_DCPT *const)handle;
    uint8_t *payLoadPtr;
    int pktLen = 0;
    uint8_t pktTos = 0;

    if ((DRV_HANDLE_INVALID == handle) || (NULL == pDcpt) || (NULL == pDcpt->pMac))
    {
        return TCPIP_MAC_RES_PACKET_ERR;
    }

    if ((NULL == ptrPacket) || (NULL == ptrPacket->pDSeg))
    {
        return TCPIP_MAC_RES_PACKET_ERR;
    }

    if (NULL == pDcpt->pMac->pktAckF)
    {
        return TCPIP_MAC_RES_PACKET_ERR;
    }

    TCPIP_MAC_ETHERNET_HEADER *pEthHdr;

    pEthHdr = (TCPIP_MAC_ETHERNET_HEADER*)ptrPacket->pMacLayer;

    switch (TCPIP_Helper_ntohs(pEthHdr->Type))
    {
        case TCPIP_ETHER_TYPE_IPV4:
        {
            IPV4_HEADER* pHdr;

            pHdr = (IPV4_HEADER*)ptrPacket->pNetLayer;

            if (pHdr->TypeOfService.reliability)
            {
                pktTos = 32;
            }
            else if (pHdr->TypeOfService.throughput)
            {
                pktTos = 192;
            }
            else if (pHdr->TypeOfService.delay)
            {
                pktTos = 160;
            }

            break;
        }

        case TCPIP_ETHER_TYPE_IPV6:
        {
            break;
        }

        case TCPIP_ETHER_TYPE_ARP:
        {
            break;
        }

        default:
        {
            break;
        }
    }

#ifdef WDRV_PIC32MZW_MAC_TX_PKT_INSPECT_HOOK
    WDRV_PIC32MZW_MAC_TX_PKT_INSPECT_HOOK(ptrPacket);
#endif

    uint8_t *pktbuf;
    TCPIP_MAC_DATA_SEGMENT *pDSeg;

    pDSeg = ptrPacket->pDSeg;

    while (NULL != pDSeg)
    {
        pktLen += pDSeg->segLen;

        pDSeg = pDSeg->next;
    }

    if (pktLen > SHARED_PKT_MEM_BUFFER_SIZE)
    {
        WDRV_DBG_TRACE_PRINT("MAC TX: payload too big (%d)\r\n", pktLen);

        return TCPIP_MAC_RES_OP_ERR;
    }

    pktbuf = payLoadPtr = DRV_PIC32MZW_PacketMemAlloc(DRV_PIC32MZW_ALLOC_OPT_PARAMS
            ETH_ETHERNET_HDR_OFFSET + pktLen, MEM_PRI_TX);

    if (NULL == pktbuf)
    {
        WDRV_DBG_TRACE_PRINT("MAC TX: malloc fail\r\n");

        return TCPIP_MAC_RES_OP_ERR;
    }

    pktbuf += ETH_ETHERNET_HDR_OFFSET;

    pDSeg = ptrPacket->pDSeg;

    while (NULL != pDSeg)
    {
        memcpy(pktbuf, pDSeg->segLoad, pDSeg->segLen);

        pktbuf += pDSeg->segLen;

        pDSeg = pDSeg->next;
    }

    pDcpt->pMac->pktAckF(ptrPacket, TCPIP_MAC_PKT_ACK_TX_OK, TCPIP_THIS_MODULE_ID);

    if (OSAL_RESULT_TRUE == OSAL_SEM_Pend(&pic32mzwCtrlDescriptor.drvAccessSemaphore, OSAL_WAIT_FOREVER))
    {
#ifdef WDRV_PIC32MZW_STATS_ENABLE
        if (OSAL_RESULT_TRUE == OSAL_MUTEX_Lock(&pic32mzwMemStatsMutex, OSAL_WAIT_FOREVER))
        {
            pic32mzMemStatistics.pkt.tx++;
            OSAL_MUTEX_Unlock(&pic32mzwMemStatsMutex);
        }
#endif
        wdrv_pic32mzw_wlan_send_packet(payLoadPtr, pktLen, pktTos, 0);
        OSAL_SEM_Post(&pic32mzwCtrlDescriptor.drvAccessSemaphore);

        OSAL_SEM_Post(&pic32mzwCtrlDescriptor.drvEventSemaphore);
    }
    else
    {
        WDRV_DBG_ERROR_PRINT("Send packet failed to lock driver semaphore\r\n");
    }

    return TCPIP_MAC_RES_OK;
}

//*******************************************************************************
/*
  Function:
    TCPIP_MAC_PACKET* WDRV_PIC32MZW_MACPacketRx
    (
        DRV_HANDLE handle,
        TCPIP_MAC_RES* pRes,
        TCPIP_MAC_PACKET_RX_STAT* pPktStat
    )

  Summary:
    Retrieve an Ethernet frame.

  Description:
    Called by the TCP/IP to retrieve the next received Ethernet frame.

  Remarks:
    See wdrv_pic32mzw_mac.h for usage information.

*/

TCPIP_MAC_PACKET* WDRV_PIC32MZW_MACPacketRx
(
    DRV_HANDLE handle,
    TCPIP_MAC_RES* pRes,
    TCPIP_MAC_PACKET_RX_STAT* pPktStat
)
{
    WDRV_PIC32MZW_DCPT *const pDcpt = (WDRV_PIC32MZW_DCPT *const)handle;
    TCPIP_MAC_PACKET* ptrPacket;

    if ((DRV_HANDLE_INVALID == handle) || (NULL == pDcpt) || (NULL == pDcpt->pMac))
    {
        return NULL;
    }

    ptrPacket = (TCPIP_MAC_PACKET*)TCPIP_Helper_ProtectedSingleListHeadRemove(&pDcpt->pMac->ethRxPktList);

    if (NULL != ptrPacket)
    {
#ifdef WDRV_PIC32MZW_MAC_RX_PKT_INSPECT_HOOK
        WDRV_PIC32MZW_MAC_RX_PKT_INSPECT_HOOK(ptrPacket);
#endif

#ifdef WDRV_PIC32MZW_STATS_ENABLE
        if (OSAL_RESULT_TRUE == OSAL_MUTEX_Lock(&pic32mzwMemStatsMutex, OSAL_WAIT_FOREVER))
        {
            pic32mzMemStatistics.pkt.rx++;
            OSAL_MUTEX_Unlock(&pic32mzwMemStatsMutex);
        }
#endif

        ptrPacket->next = NULL;
    }

    return ptrPacket;
}

//*******************************************************************************
/*
  Function:
    TCPIP_MAC_RES WDRV_PIC32MZW_MACProcess(DRV_HANDLE handle)

  Summary:
    Regular update to MAC state machine.

  Description:
    Called by the TCP/IP to update the internal state machine.

  Remarks:
    See wdrv_pic32mzw_mac.h for usage information.

*/

TCPIP_MAC_RES WDRV_PIC32MZW_MACProcess(DRV_HANDLE handle)
{
    if (DRV_HANDLE_INVALID == handle)
    {
        return TCPIP_MAC_RES_OP_ERR;
    }

    return TCPIP_MAC_RES_OK;
}

//*******************************************************************************
/*
  Function:
    TCPIP_MAC_RES WDRV_PIC32MZW_MACStatisticsGet
    (
        DRV_HANDLE handle,
        TCPIP_MAC_RX_STATISTICS* pRxStatistics,
        TCPIP_MAC_TX_STATISTICS* pTxStatistics
    )

  Summary:
    Return statistics.

  Description:

  Remarks:
    See wdrv_pic32mzw_mac.h for usage information.

*/

TCPIP_MAC_RES WDRV_PIC32MZW_MACStatisticsGet
(
    DRV_HANDLE handle,
    TCPIP_MAC_RX_STATISTICS* pRxStatistics,
    TCPIP_MAC_TX_STATISTICS* pTxStatistics
)
{
    if (DRV_HANDLE_INVALID == handle)
    {
        return TCPIP_MAC_RES_OP_ERR;
    }

    if (NULL != pRxStatistics)
    {
        memset(pRxStatistics, 0, sizeof(TCPIP_MAC_RX_STATISTICS));
    }

    if (NULL != pTxStatistics)
    {
        memset(pTxStatistics, 0, sizeof(TCPIP_MAC_TX_STATISTICS));

        pTxStatistics->nTxPendBuffers = wdrv_pic32mzw_qmu_get_tx_count();
    }

    return TCPIP_MAC_RES_OK;
}

//*******************************************************************************
/*
  Function:
    TCPIP_MAC_RES WDRV_PIC32MZW_MACParametersGet
    (
        DRV_HANDLE handle,
        TCPIP_MAC_PARAMETERS* pMacParams
    )

  Summary:
    Retrieve MAC parameter.

  Description:

  Remarks:
    See wdrv_pic32mzw_mac.h for usage information.

*/

TCPIP_MAC_RES WDRV_PIC32MZW_MACParametersGet
(
    DRV_HANDLE handle,
    TCPIP_MAC_PARAMETERS* pMacParams
)
{
    WDRV_PIC32MZW_DCPT *const pDcpt = (WDRV_PIC32MZW_DCPT *const)handle;

    /* Ensure the driver handle and user pointer is valid. */
    if ((DRV_HANDLE_INVALID == handle) || (NULL == pDcpt) || (NULL == pMacParams))
    {
        return TCPIP_MAC_RES_IS_BUSY;
    }

    if (SYS_STATUS_READY == pDcpt->sysStat)
    {
        if (NULL != pMacParams)
        {
            if (true == pic32mzwCtrlDescriptor.macAddr.valid)
            {
                memcpy(pMacParams->ifPhyAddress.v, pic32mzwCtrlDescriptor.macAddr.addr, 6);
            }
            else
            {
                memset(pMacParams->ifPhyAddress.v, 0, 6);
            }

            pMacParams->processFlags = (TCPIP_MAC_PROCESS_FLAG_RX | TCPIP_MAC_PROCESS_FLAG_TX);
            pMacParams->macType = TCPIP_MAC_TYPE_WLAN;
            pMacParams->linkMtu = TCPIP_MAC_LINK_MTU_WLAN;
        }

        return TCPIP_MAC_RES_OK;
    }

    return TCPIP_MAC_RES_IS_BUSY;
}

//*******************************************************************************
/*
  Function:
    TCPIP_MAC_RES WDRV_PIC32MZW_MACRegisterStatisticsGet
    (
        DRV_HANDLE handle,
        TCPIP_MAC_STATISTICS_REG_ENTRY* pRegEntries,
        int nEntries,
        int* pHwEntries
    )

  Summary:

  Description:

  Remarks:
    See wdrv_pic32mzw_mac.h for usage information.

*/

TCPIP_MAC_RES WDRV_PIC32MZW_MACRegisterStatisticsGet
(
    DRV_HANDLE handle,
    TCPIP_MAC_STATISTICS_REG_ENTRY* pRegEntries,
    int nEntries,
    int* pHwEntries
)
{
    return TCPIP_MAC_RES_OP_ERR;
}

//*******************************************************************************
/*
  Function:
    size_t WDRV_PIC32MZW_MACConfigGet
    (
        DRV_HANDLE handle,
        void* configBuff,
        size_t buffSize,
        size_t* pConfigSize
    )

  Summary:

  Description:

  Remarks:
    See wdrv_pic32mzw_mac.h for usage information.

*/

size_t WDRV_PIC32MZW_MACConfigGet
(
    DRV_HANDLE handle,
    void* configBuff,
    size_t buffSize,
    size_t* pConfigSize
)
{
    if (NULL != pConfigSize)
    {
        *pConfigSize = sizeof(TCPIP_MODULE_MAC_PIC32MZW1_CONFIG);
    }

    if ((NULL != configBuff) && (buffSize >= sizeof(TCPIP_MODULE_MAC_PIC32MZW1_CONFIG)))
    {
        return sizeof(TCPIP_MODULE_MAC_PIC32MZW1_CONFIG);
    }

    return 0;
}

//*******************************************************************************
/*
  Function:
    bool WDRV_PIC32MZW_MACEventMaskSet
    (
        DRV_HANDLE handle,
        TCPIP_MAC_EVENT macEvents,
        bool enable
    )

  Summary:
    Set or clear the event mask.

  Description:
    Sets or clears particular events within the event mask.

  Remarks:
    See wdrv_pic32mzw_mac.h for usage information.

*/

bool WDRV_PIC32MZW_MACEventMaskSet
(
    DRV_HANDLE handle,
    TCPIP_MAC_EVENT macEvents,
    bool enable
)
{
    WDRV_PIC32MZW_DCPT *const pDcpt = (WDRV_PIC32MZW_DCPT *const)handle;

    /* Ensure the driver handle is valid. */
    if ((DRV_HANDLE_INVALID == handle) || (NULL == pDcpt) || (NULL == pDcpt->pMac))
    {
        return false;
    }

    if (OSAL_RESULT_TRUE == OSAL_SEM_Pend(&pDcpt->pMac->eventSemaphore, OSAL_WAIT_FOREVER))
    {
        if (true == enable)
        {
            pDcpt->pMac->eventMask |= macEvents;
        }
        else
        {
            pDcpt->pMac->eventMask &= ~macEvents;
        }

        OSAL_SEM_Post(&pDcpt->pMac->eventSemaphore);

        return true;
    }
    else
    {
        WDRV_DBG_ERROR_PRINT("Event mask failed to lock event semaphore\r\n");

        return false;
    }
}

//*******************************************************************************
/*
  Function:
    bool WDRV_PIC32MZW_MACEventAcknowledge(DRV_HANDLE handle, TCPIP_MAC_EVENT macEvents)

  Summary:
    Acknowledge an event.

  Description:
    Indicates that certain events are to be acknowledged and cleared.

  Remarks:
    See wdrv_pic32mzw_mac.h for usage information.

*/

bool WDRV_PIC32MZW_MACEventAcknowledge(DRV_HANDLE handle, TCPIP_MAC_EVENT macEvents)
{
    WDRV_PIC32MZW_DCPT *const pDcpt = (WDRV_PIC32MZW_DCPT *const)handle;

    /* Ensure the driver handle is valid. */
    if ((DRV_HANDLE_INVALID == handle) || (NULL == pDcpt) || (NULL == pDcpt->pMac))
    {
        return false;
    }

    if (OSAL_RESULT_TRUE == OSAL_SEM_Pend(&pDcpt->pMac->eventSemaphore, OSAL_WAIT_FOREVER))
    {
        pDcpt->pMac->events &= ~macEvents;
        OSAL_SEM_Post(&pDcpt->pMac->eventSemaphore);

        return true;
    }
    else
    {
        WDRV_DBG_ERROR_PRINT("Event ACK failed to lock event semaphore\r\n");

        return false;
    }
}

//*******************************************************************************
/*
  Function:
    TCPIP_MAC_EVENT WDRV_PIC32MZW_MACEventPendingGet(DRV_HANDLE handle)

  Summary:
    Retrieve the current events.

  Description:
    Returns the current event state.

  Remarks:
    See wdrv_pic32mzw_mac.h for usage information.

*/

TCPIP_MAC_EVENT WDRV_PIC32MZW_MACEventPendingGet(DRV_HANDLE handle)
{
    WDRV_PIC32MZW_DCPT *const pDcpt = (WDRV_PIC32MZW_DCPT *const)handle;
    TCPIP_MAC_EVENT events;

    /* Ensure the driver handle is valid. */
    if ((DRV_HANDLE_INVALID == handle) || (NULL == pDcpt) || (NULL == pDcpt->pMac))
    {
        return 0;
    }

    if (OSAL_RESULT_TRUE == OSAL_SEM_Pend(&pDcpt->pMac->eventSemaphore, OSAL_WAIT_FOREVER))
    {
        events = pDcpt->pMac->events;
        OSAL_SEM_Post(&pDcpt->pMac->eventSemaphore);

        return events;
    }
    else
    {
        WDRV_DBG_ERROR_PRINT("Event get failed to lock event semaphore\r\n");

        return TCPIP_MAC_EV_NONE;
    }
}

// *****************************************************************************
// *****************************************************************************
// Section: PIC32MZW Information Implementation
// *****************************************************************************
// *****************************************************************************

//*******************************************************************************
/*
  Function:
    WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_InfoDeviceMACAddressGet
    (
        DRV_HANDLE handle,
        uint8_t *const pMACAddress
    )

  Summary:
    Retrieves the MAC address of the PIC32MZW.

  Description:
    Retrieves the current working MAC address.

  Remarks:
    See wdrv_pic32mzw.h for usage information.

*/

WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_InfoDeviceMACAddressGet
(
    DRV_HANDLE handle,
    uint8_t *const pMACAddress
)
{
    const WDRV_PIC32MZW_DCPT *const pDcpt = (const WDRV_PIC32MZW_DCPT *const)handle;

    /* Ensure the driver handle and user pointer is valid. */
    if ((DRV_HANDLE_INVALID == handle) || (NULL == pDcpt) || (NULL == pMACAddress))
    {
        return WDRV_PIC32MZW_STATUS_INVALID_ARG;
    }

    /* Ensure the driver instance has been opened for use. */
    if (false == pDcpt->isOpen)
    {
        return WDRV_PIC32MZW_STATUS_NOT_OPEN;
    }

    if (false == pic32mzwCtrlDescriptor.macAddr.valid)
    {
        return WDRV_PIC32MZW_STATUS_NOT_OPEN;
    }

    memcpy(pMACAddress, pic32mzwCtrlDescriptor.macAddr.addr, 6);

    return WDRV_PIC32MZW_STATUS_OK;
}

//*******************************************************************************
/*
  Function:
    WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_InfoEnabledChannelsGet
    (
        DRV_HANDLE handle,
        WDRV_PIC32MZW_CHANNEL24_MASK *const pChannelMask
    )

  Summary:
    Retrieves the enabled channels of the PIC32MZW.

  Description:
    Retrieves the enabled channels for the set regulatory domain.

  Remarks:
    See wdrv_pic32mzw.h for usage information.

*/

WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_InfoEnabledChannelsGet
(
    DRV_HANDLE handle,
    WDRV_PIC32MZW_CHANNEL24_MASK *const pChannelMask
)
{
    const WDRV_PIC32MZW_DCPT *const pDcpt = (const WDRV_PIC32MZW_DCPT *const)handle;

    /* Ensure the driver handle and user pointer is valid. */
    if ((DRV_HANDLE_INVALID == handle) || (NULL == pDcpt) || (NULL == pChannelMask))
    {
        return WDRV_PIC32MZW_STATUS_INVALID_ARG;
    }

    /* Ensure pointer is valid */
    if (NULL == pDcpt->pCtrl)
    {
        return WDRV_PIC32MZW_STATUS_INVALID_ARG;
    }

    /* Ensure the driver instance has been opened for use. */
    if (false == pDcpt->isOpen)
    {
        return WDRV_PIC32MZW_STATUS_NOT_OPEN;
    }

    *pChannelMask = pDcpt->pCtrl->scanChannelMask24;

    return WDRV_PIC32MZW_STATUS_OK;
}

//*******************************************************************************
/*
  Function:
    WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_InfoOpChanGet
    (
        DRV_HANDLE handle,
        uint8_t *const pMACAddress
    )

  Summary:
    Retrieves the operating channel of the PIC32MZW.

  Description:
    Retrieves the current operating channel.

  Remarks:
    See wdrv_pic32mzw.h for usage information.

*/

WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_InfoOpChanGet
(
    DRV_HANDLE handle,
    WDRV_PIC32MZW_CHANNEL_ID *const pOpChan
)
{
    const WDRV_PIC32MZW_DCPT *const pDcpt = (const WDRV_PIC32MZW_DCPT *const)handle;

    /* Ensure the driver handle and user pointer is valid. */
    if ((DRV_HANDLE_INVALID == handle) || (NULL == pDcpt) || (NULL == pOpChan))
    {
        return WDRV_PIC32MZW_STATUS_INVALID_ARG;
    }

    /* Ensure the driver instance has been opened for use. */
    if (false == pDcpt->isOpen)
    {
        return WDRV_PIC32MZW_STATUS_NOT_OPEN;
    }

    if (WDRV_PIC32MZW_CONN_STATE_CONNECTED != pDcpt->pCtrl->connectedState)
    {
        return WDRV_PIC32MZW_STATUS_NOT_CONNECTED;
    }

    *pOpChan = pDcpt->pCtrl->opChannel;

    return WDRV_PIC32MZW_STATUS_OK;
}

//*******************************************************************************
/*
  Function:
    WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_InfoRfMacConfigGet
    (
        DRV_HANDLE handle,
        WDRV_PIC32MZW_RF_MAC_CONFIG *const pRfMacConfig
    )

  Summary:
    Retrieves the RF and MAC configuration of the PIC32MZW.

  Description:
    Retrieves the current RF and MAC configuration.

  Remarks:
    See wdrv_pic32mzw.h for usage information.

*/

WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_InfoRfMacConfigGet
(
    DRV_HANDLE handle,
    WDRV_PIC32MZW_RF_MAC_CONFIG *const pRfMacConfig
)
{
    const WDRV_PIC32MZW_DCPT *const pDcpt = (const WDRV_PIC32MZW_DCPT *const)handle;

    /* Ensure the driver handle and user pointer is valid. */
    if ((DRV_HANDLE_INVALID == handle) || (NULL == pDcpt) || (NULL == pRfMacConfig))
    {
        return WDRV_PIC32MZW_STATUS_INVALID_ARG;
    }

    /* Ensure pointer is valid */
    if (NULL == pDcpt->pCtrl)
    {
        return WDRV_PIC32MZW_STATUS_INVALID_ARG;
    }

    /* Ensure the driver instance has been opened for use. */
    if (false == pDcpt->isOpen)
    {
        return WDRV_PIC32MZW_STATUS_NOT_OPEN;
    }

    if (pDcpt->pCtrl->rfMacConfigStatus & DRV_PIC32MZW_POWER_ON_CAL_CONFIG)
    {
        pRfMacConfig->powerOnCalIsValid = true;
    }
    else
    {
        pRfMacConfig->powerOnCalIsValid = false;
    }

    if (pDcpt->pCtrl->rfMacConfigStatus & DRV_PIC32MZW_FACTORY_CAL_CONFIG)
    {
        pRfMacConfig->factoryCalIsValid = true;
    }
    else
    {
        pRfMacConfig->factoryCalIsValid = false;
    }

    if (pDcpt->pCtrl->rfMacConfigStatus & DRV_PIC32MZW_GAIN_TABLE_CONFIG)
    {
        pRfMacConfig->gainTableIsValid = true;
    }
    else
    {
        pRfMacConfig->gainTableIsValid = false;
    }

    if (pDcpt->pCtrl->rfMacConfigStatus & DRV_PIC32MZW_MAC_ADDRESS_CONFIG)
    {
        pRfMacConfig->macAddressIsValid = true;
    }
    else
    {
        pRfMacConfig->macAddressIsValid = false;
    }

    return WDRV_PIC32MZW_STATUS_OK;
}

// *****************************************************************************
// *****************************************************************************
// Section: PIC32MZW WID Processing Implementation
// *****************************************************************************
// *****************************************************************************

//*******************************************************************************
/*
  Function:
    void WDRV_PIC32MZW_WIDProcess(uint16_t wid, uint16_t length, uint8_t *pData)

  Summary:
    WID callback.

  Description:
    Callback to handle WIDs from firmware:
      DRV_WIFI_WID_ASSOC_STAT
      DRV_WIFI_WID_GET_SCAN_RESULTS

  Remarks:
    See wdrv_pic32mzw_api.h for usage information.

*/
void WDRV_PIC32MZW_WIDProcess(uint16_t wid, uint16_t length, const uint8_t *const pData)
{
    WDRV_PIC32MZW_DCPT *pDcpt = &pic32mzwDescriptor[0];
    WDRV_PIC32MZW_CTRLDCPT *pCtrl;
    DRV_PIC32MZW_WIDCTX wids;

    if ((NULL == pData) || (NULL == pDcpt->pCtrl))
    {
        return;
    }

    if (false == pDcpt->isInit)
    {
        return;
    }

    pCtrl = pDcpt->pCtrl;

    switch(wid)
    {
        case DRV_WIFI_WID_RSSI:
        {
            if (length < 1)
            {
                break;
            }

            if (false == pCtrl->isAP)
            {
                /* Store locally. */
                if (0 != *pData)
                {
                    pCtrl->assocInfoSTA.rssi = *pData;
                }

                if (NULL != pCtrl->pfAssociationRSSICB)
                {
                    /* Pass RSSI value to user application if callback supplied.
                       the callback is cleared after this operation has completed. */
                    pCtrl->pfAssociationRSSICB((DRV_HANDLE)pDcpt, (WDRV_PIC32MZW_ASSOC_HANDLE)&pCtrl->assocInfoSTA, pCtrl->assocInfoSTA.rssi);

                    pCtrl->pfAssociationRSSICB = NULL;
                }
            }
            break;
        }

        case DRV_WIFI_WID_ASSOC_STAT:
        {
            if (length < 1)
            {
                break;
            }

            if ((0 == *pData) && (WDRV_PIC32MZW_CONN_STATE_DISCONNECTED != pCtrl->connectedState))
            {
                WDRV_PIC32MZW_CONN_STATE ConnState = WDRV_PIC32MZW_CONN_STATE_FAILED;
                pCtrl->opChannel = WDRV_PIC32MZW_CID_ANY;

                if (WDRV_PIC32MZW_CONN_STATE_CONNECTED == pCtrl->connectedState)
                {
                    ConnState = WDRV_PIC32MZW_CONN_STATE_DISCONNECTED;
                }

                pCtrl->connectedState = WDRV_PIC32MZW_CONN_STATE_DISCONNECTED;

                if (false == pCtrl->isAP)
                {
#ifdef WDRV_PIC32MZW_ENTERPRISE_SUPPORT
                    if (DRV_PIC32MZW1_TLS_HANDLE_INVALID != pCtrl->tlsHandle)
                    {
                        /* De-init the TLS module as its not required anymore */
                        if (true == DRV_PIC32MZW1_TLS_DeInit(pCtrl->tlsHandle))
                        {
                            pCtrl->tlsHandle = DRV_PIC32MZW1_TLS_HANDLE_INVALID;
                        }
                    }
#endif /* WDRV_PIC32MZW_ENTERPRISE_SUPPORT */
                    if (NULL != pCtrl->pfConnectNotifyCB)
                    {
                        /* Update user application via callback if set. */

                        pCtrl->pfConnectNotifyCB((DRV_HANDLE)pDcpt, (WDRV_PIC32MZW_ASSOC_HANDLE)&pCtrl->assocInfoSTA, ConnState);
                    }
                }
                else
                {
                    pCtrl->isAP = false;
                }
            }
            else if ((1 == *pData) && (WDRV_PIC32MZW_CONN_STATE_CONNECTED != pCtrl->connectedState))
            {
                pCtrl->connectedState = WDRV_PIC32MZW_CONN_STATE_CONNECTED;

                if (false == pCtrl->isAP)
                {
                    OSAL_CRITSECT_DATA_TYPE critSect;

                    pCtrl->assocInfoSTA.handle            = (DRV_HANDLE)pCtrl;
                    pCtrl->assocInfoSTA.peerAddress.valid = false;
                    pCtrl->assocInfoSTA.rssi              = 0;
                    pCtrl->assocInfoSTA.assocID           = 1;

                    memset(&pCtrl->assocInfoSTA.peerAddress.addr, 0, WDRV_PIC32MZW_MAC_ADDR_LEN);

                    DRV_PIC32MZW_MultiWIDInit(&wids, 32);
                    DRV_PIC32MZW_MultiWIDAddQuery(&wids, DRV_WIFI_WID_RSSI);
                    DRV_PIC32MZW_MultiWIDAddQuery(&wids, DRV_WIFI_WID_BSSID);
                    DRV_PIC32MZW_MultiWIDAddQuery(&wids, DRV_WIFI_WID_CURR_OPER_CHANNEL);

                    critSect = OSAL_CRIT_Enter(OSAL_CRIT_TYPE_LOW);

                    if (false == DRV_PIC32MZW_MultiWid_Write(&wids))
                    {
                        OSAL_CRIT_Leave(OSAL_CRIT_TYPE_LOW, critSect);

                        DRV_PIC32MZW_MultiWIDDestroy(&wids);
                    }
                    else
                    {
                        OSAL_CRIT_Leave(OSAL_CRIT_TYPE_LOW, critSect);
                    }
#ifdef WDRV_PIC32MZW_ENTERPRISE_SUPPORT
                    if (DRV_PIC32MZW1_TLS_HANDLE_INVALID != pCtrl->tlsHandle)
                    {
                        /* De-init the TLS module as its not required anymore */
                        if (true == DRV_PIC32MZW1_TLS_DeInit(pCtrl->tlsHandle))
                        {
                            pCtrl->tlsHandle = DRV_PIC32MZW1_TLS_HANDLE_INVALID;
                        }
                    }
#endif /* WDRV_PIC32MZW_ENTERPRISE_SUPPORT */
                    if (NULL != pCtrl->pfConnectNotifyCB)
                    {
                        /* Update user application via callback if set. */

                        pCtrl->pfConnectNotifyCB((DRV_HANDLE)pDcpt, (WDRV_PIC32MZW_ASSOC_HANDLE)&pCtrl->assocInfoSTA, WDRV_PIC32MZW_CONN_STATE_CONNECTED);
                    }
                }
            }

            break;
        }

        case DRV_WIFI_WID_CURR_OPER_CHANNEL:
        {
            pCtrl->opChannel = *pData;
            break;
        }

        case DRV_WIFI_WID_BSSID:
        {
            if (length < 6)
            {
                break;
            }

            if (false == pCtrl->isAP)
            {
                memcpy(&pCtrl->assocInfoSTA.peerAddress.addr, pData, WDRV_PIC32MZW_MAC_ADDR_LEN);
                pCtrl->assocInfoSTA.peerAddress.valid = true;
            }

            break;
        }

        case DRV_WIFI_WID_WPA3_TD_AP:
        {
            if (length < 1)
            {
                break;
            }

            if (false == pCtrl->isAP)
            {
                pCtrl->assocInfoSTA.transitionDisable = (1 == *pData) ? true : false;
            }
            break;
        }

        case DRV_WIFI_WID_MAC_ADDR:
        {
            if (length < 6)
            {
                break;
            }

            memcpy(&pCtrl->macAddr.addr, pData, WDRV_PIC32MZW_MAC_ADDR_LEN);
            pCtrl->macAddr.valid = true;
            break;
        }

        case DRV_WIFI_WID_GET_SCAN_RESULTS:
        {
            const DRV_PIC32MZW_SCAN_RESULTS *const pScanRes = (const DRV_PIC32MZW_SCAN_RESULTS *const)pData;

            if (NULL != pScanRes)
            {
                if((NULL != pCtrl->pfBSSFindNotifyCB) && (0 == pScanRes->ofTotal))
                {
                    pCtrl->pfBSSFindNotifyCB(pCtrl->handle, 0, 0, NULL);
                    pCtrl->pfBSSFindNotifyCB = NULL;
                    break;
                }

                DRV_PIC32MZW_StoreBSSScanResult(pScanRes);

                if (0 == pScanRes->index)
                {
                    pCtrl->scanInProgress = false;
                }
            }

            if (NULL != pCtrl->pfBSSFindNotifyCB)
            {
                /* Reuse find next function by pre-decrementing scan index. */
                pCtrl->scanIndex--;

                WDRV_PIC32MZW_BSSFindNext(pCtrl->handle, pCtrl->pfBSSFindNotifyCB);
            }

            break;
        }

        case DRV_WIFI_WID_STA_JOIN_INFO:
        {
            WDRV_PIC32MZW_CONN_STATE connState;
            WDRV_PIC32MZW_ASSOC_INFO *pStaAssocInfo;

            if (false == pCtrl->isAP)
            {
                break;
            }

            pStaAssocInfo = _WDRV_PIC32MZW_FindAssocInfoAP(pCtrl, &pData[1]);

            if (0 == *pData)
            {
                connState = WDRV_PIC32MZW_CONN_STATE_DISCONNECTED;
            }
            else
            {
                connState = WDRV_PIC32MZW_CONN_STATE_CONNECTED;

                if (NULL != pStaAssocInfo)
                {
                    WDRV_DBG_ERROR_PRINT("JOIN: Association found for new connection\r\n");
                }
                else
                {
                    pStaAssocInfo = _WDRV_PIC32MZW_FindAssocInfoAP(pCtrl, NULL);
                }

                if (NULL != pStaAssocInfo)
                {
                    pStaAssocInfo->handle            = (DRV_HANDLE)pCtrl;
                    pStaAssocInfo->peerAddress.valid = true;
                    pStaAssocInfo->authType          = WDRV_PIC32MZW_AUTH_TYPE_DEFAULT;
                    pStaAssocInfo->rssi              = 0;
                    pStaAssocInfo->assocID           = pData[7];

                    memcpy(&pStaAssocInfo->peerAddress.addr, &pData[1], WDRV_PIC32MZW_MAC_ADDR_LEN);
                }
                else
                {
                    WDRV_DBG_ERROR_PRINT("JOIN: New association failed\r\n");
                }
            }

            if (NULL != pStaAssocInfo)
            {
                if (NULL != pCtrl->pfConnectNotifyCB)
                {
                    /* Update user application via callback if set. */

                    pCtrl->pfConnectNotifyCB((DRV_HANDLE)pDcpt, (WDRV_PIC32MZW_ASSOC_HANDLE)pStaAssocInfo, connState);
                }

                if (0 == *pData)
                {
                    pStaAssocInfo->handle = DRV_HANDLE_INVALID;
                    pStaAssocInfo->peerAddress.valid = false;
                    pStaAssocInfo->assocID = -1;
                }
            }
            else
            {
                WDRV_DBG_ERROR_PRINT("JOIN: No association found\r\n");
            }

            break;
        }

        case DRV_WIFI_WID_USER_STATUS_INFO:
        {
            break;
        }

        case DRV_WIFI_WID_REG_DOMAIN_INFO:
        {
            const uint8_t *pChannel = NULL;
            WDRV_PIC32MZW_CHANNEL24_MASK channelMask;

            if (length < DRV_PIC32MZW_REGDOMAIN_RES_LEN)
            {
                break;
            }

            pChannel = &pData[8 + WDRV_PIC32MZW_REGDOMAIN_MAX_NAME_LEN];

            channelMask = *(pChannel++);
            channelMask |= *(pChannel++) << 8;

            pCtrl->scanChannelMask24 = channelMask;

            if (NULL != pCtrl->pfRegDomCB)
            {
                if ((0 == pData[0]) || (0 == pData[1]) || (1 > pData[3]) || (WDRV_PIC32MZW_REGDOMAIN_MAX_NAME_LEN < pData[3]))
                {
                    pCtrl->pfRegDomCB((DRV_HANDLE)pDcpt, 0, 0, false, NULL);
                }
                else
                {
                    bool current;
                    const uint8_t *pVer = NULL;
                    WDRV_PIC32MZW_REGDOMAIN_INFO regDomInfo;

                    memset(&regDomInfo, 0, sizeof(WDRV_PIC32MZW_REGDOMAIN_INFO));

                    if (1 == pData[2])
                    {
                        current = true;
                    }
                    else
                    {
                        current = false;
                    }

                    regDomInfo.regDomainLen = pData[3];
                    memcpy(regDomInfo.regDomain, &pData[4], pData[3]);

                    pVer = &pData[4 + WDRV_PIC32MZW_REGDOMAIN_MAX_NAME_LEN];

                    regDomInfo.version.major = *(pVer++);
                    regDomInfo.version.major |= *(pVer++) << 8;
                    regDomInfo.version.minor = *(pVer++);
                    regDomInfo.version.minor |= *(pVer++) << 8;

                    regDomInfo.channelMask = channelMask;

                    pCtrl->pfRegDomCB((DRV_HANDLE)pDcpt, pData[0], pData[1], current, &regDomInfo);
                }
            }

            break;
        }

        case DRV_WIFI_WID_RF_MAC_CONFIG_STATUS:
        {
            if (SYS_STATUS_READY_EXTENDED == pDcpt->sysStat)
            {
                pCtrl->rfMacConfigStatus = *pData;

                if (WDRV_PIC32MZW_RF_MAC_MIN_REQ_CONFIG == (pCtrl->rfMacConfigStatus & WDRV_PIC32MZW_RF_MAC_MIN_REQ_CONFIG))
                {
                    pDcpt->pCtrl->extSysStat = WDRV_PIC32MZW_SYS_STATUS_RF_READY;
                    pDcpt->sysStat = SYS_STATUS_READY;
                }
                else
                {
                    pDcpt->pCtrl->extSysStat = WDRV_PIC32MZW_SYS_STATUS_RF_CONF_MISSING;
                }
            }

            break;
        }
        
        case DRV_WIFI_WID_VSIE_RX_DATA:
        {
            uint8_t frameType;
            
            if (length < DRV_PIC32MZW_IE_DATA_SIZE_FIELD_LEN)
            {
                break;
            }
            
            if (NULL != pCtrl->pfVendorIERxCB)
            {
                WDRV_PIC32MZW_VENDORIE_INFO vendorIEInfo;

                memcpy(vendorIEInfo.sa, &pData[0], 6);

                vendorIEInfo.rssi = pData[6];
                
                frameType = pData[7];
                
                if (0x80 == frameType)
                {
                    vendorIEInfo.frameType = WDRV_PIC32MZW_VENDOR_IE_BEACON;
                }
                else if (0x40 == frameType)
                {
                    vendorIEInfo.frameType = WDRV_PIC32MZW_VENDOR_IE_PROBE_REQ;
                }
                else if (0x50 == frameType)
                {
                    vendorIEInfo.frameType = WDRV_PIC32MZW_VENDOR_IE_PROBE_RSP;
                }
                else
                {
                    break;
                }
            
                pCtrl->pfVendorIERxCB((DRV_HANDLE)pDcpt, &vendorIEInfo, &pData[DRV_PIC32MZW_IE_DATA_OFFSET_RX], (length - DRV_PIC32MZW_IE_DATA_OFFSET_RX));
            }
            break;
        }
		case DRV_WIFI_WID_POWER_MANAGEMENT_INFO:
        {
            WDRV_PIC32MZW_POWERSAVE_MODE psMode;
            bool bSleepEntry;
            uint32_t u32SleepDurationMs;
            
            if (length < DRV_PIC32MZW_PS_INFO_LEN)
            {
                break;
            }
                
            psMode = (WDRV_PIC32MZW_POWERSAVE_MODE) pData[0];
            
            /* Update the current PS mode */
            if (pCtrl->powerSaveMode != psMode)
            {
                pCtrl->powerSaveMode = psMode;
            }
            
            if(1 == pData[1])
            {
                bSleepEntry = true;
            }
            else
            {
                bSleepEntry = false;
            }
            
            u32SleepDurationMs = pData[2];
            u32SleepDurationMs |= pData[3] << 8;
            u32SleepDurationMs |= pData[4] << 16;
            u32SleepDurationMs |= pData[5] << 24;
            
            if (NULL != pCtrl->pfPSNotifyCB)
            {
                pCtrl->pfPSNotifyCB((DRV_HANDLE)pDcpt, psMode, bSleepEntry, u32SleepDurationMs);
            }
            break;
        }
      
        default:
        {
            break;
        }
    }
}

// *****************************************************************************
// *****************************************************************************
// Section: PIC32MZW Interrupt Implementation
// *****************************************************************************
// *****************************************************************************

//*******************************************************************************
/*
  Function:
    void WDRV_PIC32MZW_TasksRFMACISR(void)

  Summary:
    PIC32MZW RF MAC interrupt service routine.

  Description:
    PIC32MZW RF MAC interrupt service routine.

  Remarks:
    See wdrv_pic32mzw_api.h for usage information.

*/

void WDRV_PIC32MZW_TasksRFMACISR(void)
{
    OSAL_SEM_PostISR(&pic32mzwCtrlDescriptor.drvEventSemaphore);
    wdrv_pic32mzw_mac_isr(1);
    IFS2bits.RFMACIF = 0;
}

//*******************************************************************************
/*
  Function:
    void WDRV_PIC32MZW_TasksRFTimer0ISR(void)

  Summary:
    PIC32MZW RF Timer 0 interrupt service routine.

  Description:
    PIC32MZW RF Timer 0 interrupt service routine.

  Remarks:
    See wdrv_pic32mzw_api.h for usage information.

*/

void WDRV_PIC32MZW_TasksRFTimer0ISR(void)
{
    OSAL_SEM_PostISR(&pic32mzwCtrlDescriptor.drvEventSemaphore);
    wdrv_pic32mzw_timer_tick_isr(0);
    IFS2bits.RFTM0IF = 0;
}

//*******************************************************************************
/*
  Function:
    void WDRV_PIC32MZW_TasksRFSMCISR(void)

  Summary:
    PIC32MZW RF SMC interrupt service routine.

  Description:
    PIC32MZW RF powersave interrupt service routine for WSM and WDS sleep modes.

  Remarks:
    See wdrv_pic32mzw_api.h for usage information.

*/

void WDRV_PIC32MZW_TasksRFSMCISR(void)
{
    OSAL_SEM_PostISR(&pic32mzwCtrlDescriptor.drvEventSemaphore);
    wdrv_pic32mzw_smc_isr(1);
    IFS2bits.RFSMCIF = 0;
}

// *****************************************************************************
// *****************************************************************************
// Section: PIC32MZW MAC Driver Internal Callback Implementation
// *****************************************************************************
// *****************************************************************************

//*******************************************************************************
/*
  Function:
    static bool _DRV_PIC32MZW_AllocPktCallback
    (
        TCPIP_MAC_PACKET* ptrPacket,
        const void* ackParam
    )

  Summary:
    Allocated packet completion callback.

  Description:
    Packets allocated as general memory are freed when this callback is invoked.

  Precondition:
    A packet previously allocated by _DRV_PIC32MZW_AllocPkt.

  Parameters:
    ptrPacket - Pointer to packet structure.
    ackParam  - Pointer to priority tracking structure.

  Returns:
    true or false indicating if operation was successful.

  Remarks:
    None.
*/

static bool _DRV_PIC32MZW_AllocPktCallback
(
    TCPIP_MAC_PACKET* ptrPacket,
    const void* ackParam
)
{
    DRV_PIC32MZW_MEM_ALLOC_HDR *pAllocHdr;

    if (NULL == ptrPacket)
    {
        return false;
    }

    pAllocHdr = (DRV_PIC32MZW_MEM_ALLOC_HDR*)ackParam;

    if (NULL != pAllocHdr)
    {
        if (0 == DRV_PIC32MZW_MemFree(DRV_PIC32MZW_ALLOC_OPT_PARAMS pAllocHdr->memory))
        {
            return false;
        }
    }

    ptrPacket->ackParam = NULL;

    TCPIP_Helper_ProtectedSingleListTailAdd(&pic32mzwDiscardQueue, (SGL_LIST_NODE*)ptrPacket);

    return true;
}

// *****************************************************************************
// *****************************************************************************
// Section: PIC32MZW MAC Driver Internal Implementation
// *****************************************************************************
// *****************************************************************************

//*******************************************************************************
/*
  Function:
    void DRV_PIC32MZW_MACEthernetSendPacket
    (
        const uint8_t *const pEthMsg,
        uint16_t lengthEthMsg,
        uint8_t hdrOffset
    )

  Summary:
    Send an Ethernet frame to the TCP/IP stack.

  Description:
    Queues a new Ethernet frame for the TCP/IP stack from the WiFi firmware.

  Precondition:
    TCP/IP stack and WiFi driver must be initialized.

  Parameters:
    pEthMsg      - Pointer to Ethernet packet.
    lengthEthMsg - Length of Ethernet packet.
    hdrOffset    - Offset to packet header.

  Returns:
    None.

  Remarks:
    None.
*/

void DRV_PIC32MZW_MACEthernetSendPacket
(
    const uint8_t *const pEthMsg,
    uint16_t lengthEthMsg,
    uint8_t hdrOffset
)
{
    TCPIP_MAC_PACKET *ptrPacket = NULL;
    TCPIP_MAC_EVENT events;
    DRV_PIC32MZW_MEM_ALLOC_HDR *pAllocHdr = NULL;
    void *pBufferAddr;

    if (NULL == pEthMsg)
    {
        return;
    }

    pBufferAddr = (void*)&pEthMsg[-hdrOffset];

    if (NULL != pic32mzwMACDescriptor.pktAllocF)
    {
        ptrPacket = pic32mzwMACDescriptor.pktAllocF(sizeof(TCPIP_MAC_PACKET), lengthEthMsg-ETHERNET_HDR_LEN, 0);
    }

    if (NULL == ptrPacket)
    {
        DRV_PIC32MZW_MemFree(DRV_PIC32MZW_ALLOC_OPT_PARAMS pBufferAddr);
        return;
    }

    memcpy(ptrPacket->pMacLayer, pEthMsg, lengthEthMsg);

    DRV_PIC32MZW_MemFree(DRV_PIC32MZW_ALLOC_OPT_PARAMS pBufferAddr);

    ptrPacket->pDSeg->segLen = lengthEthMsg - ETHERNET_HDR_LEN;
    ptrPacket->pktFlags |= TCPIP_MAC_PKT_FLAG_QUEUED;
    ptrPacket->tStamp = SYS_TMR_TickCountGet();

    ptrPacket->ackFunc = _DRV_PIC32MZW_AllocPktCallback;
    ptrPacket->ackParam = pAllocHdr;

    /* Store packet in FIFO and signal stack that packet ready to process. */
    TCPIP_Helper_ProtectedSingleListTailAdd(&pic32mzwMACDescriptor.ethRxPktList, (SGL_LIST_NODE*)ptrPacket);

    /* Notify stack of received packet. */
    if (OSAL_RESULT_TRUE == OSAL_SEM_Pend(&pic32mzwMACDescriptor.eventSemaphore, OSAL_WAIT_FOREVER))
    {
        events = pic32mzwMACDescriptor.events | ~pic32mzwMACDescriptor.eventMask;
        pic32mzwMACDescriptor.events |= TCPIP_EV_RX_DONE;
        OSAL_SEM_Post(&pic32mzwMACDescriptor.eventSemaphore);

        if (0 == (events & TCPIP_EV_RX_DONE))
        {
            pic32mzwMACDescriptor.eventF(TCPIP_EV_RX_DONE, pic32mzwMACDescriptor.eventParam);
        }
    }
    else
    {
        WDRV_DBG_ERROR_PRINT("MAC receive failed to lock event semaphore\r\n");
    }
}

//*******************************************************************************
/*
  Function:
    void* DRV_PIC32MZW_MemAlloc(uint16_t size)

  Summary:
    Allocate general memory from packet pool.

  Description:
    Allocates memory from the packet pool for general use.

  Precondition:
    TCP/IP stack must be initialized.

  Parameters:
    size - Size of memory to allocate.

  Returns:
    Pointer to allocated memory, or NULL for error.

  Remarks:
    None.
*/

void* DRV_PIC32MZW_MemAlloc(DRV_PIC32MZW_ALLOC_OPT_ARGS uint16_t size)
{
    DRV_PIC32MZW_MEM_ALLOC_HDR *pAllocHdr;
    uint8_t *pUnalignedPtr;
    uint8_t *pAlignedPtr;
    uint16_t alignedSize;

    if (OSAL_RESULT_FALSE == OSAL_MUTEX_Lock(&pic32mzwMemMutex, OSAL_WAIT_FOREVER))
    {
        return NULL;
    }

    alignedSize = (((size + sizeof(DRV_PIC32MZW_MEM_ALLOC_HDR)) + ((2*PIC32MZW_CACHE_LINE_SIZE)-1)) / PIC32MZW_CACHE_LINE_SIZE) * PIC32MZW_CACHE_LINE_SIZE;

    pUnalignedPtr = OSAL_Malloc(alignedSize);

    if (NULL == pUnalignedPtr)
    {
        OSAL_MUTEX_Unlock(&pic32mzwMemMutex);
        return NULL;
    }

    pAlignedPtr = (uint8_t*)(((uint32_t)pUnalignedPtr + (PIC32MZW_CACHE_LINE_SIZE-1)) & ~(PIC32MZW_CACHE_LINE_SIZE-1));

    if (IS_KVA0(pAlignedPtr))
    {
        uint8_t *pCacheLinePtr = pAlignedPtr;
        int numLines = alignedSize / PIC32MZW_CACHE_LINE_SIZE;

        while (numLines--)
        {
            __asm__ __volatile__ ("cache 0x15, 0(%0)" ::"r"(pCacheLinePtr));
            pCacheLinePtr += PIC32MZW_CACHE_LINE_SIZE;
        }

        __asm__ __volatile__ ("sync");

        pAlignedPtr = KVA0_TO_KVA1(pAlignedPtr);
    }

    pAllocHdr = (DRV_PIC32MZW_MEM_ALLOC_HDR*)pAlignedPtr;

    pAllocHdr->pNext         = NULL;
    pAllocHdr->size          = size;
    pAllocHdr->users         = 1;
    pAllocHdr->priLevel      = -1;
    pAllocHdr->pAllocPtr     = NULL;
    pAllocHdr->pUnalignedPtr = pUnalignedPtr;

#ifdef WDRV_PIC32MZW_STATS_ENABLE
    if (OSAL_RESULT_TRUE == OSAL_MUTEX_Lock(&pic32mzwMemStatsMutex, OSAL_WAIT_FOREVER))
    {
        pic32mzMemStatistics.mem.alloc++;
        pic32mzMemStatistics.mem.totalNumAlloc++;
        pic32mzMemStatistics.mem.allocSize += size;
        pic32mzMemStatistics.mem.totalSizeAlloc += size;

        OSAL_MUTEX_Unlock(&pic32mzwMemStatsMutex);
    }
#endif

#ifdef DRV_PIC32MZW_TRACK_MEMORY_ALLOC
    _DRV_PIC32MZW_MemTrackerAdd(DRV_PIC32MZW_ALLOC_OPT_PARAMS_V pAllocHdr->memory, size);
#endif

    OSAL_MUTEX_Unlock(&pic32mzwMemMutex);

    return pAllocHdr->memory;
}

//*******************************************************************************
/*
  Function:
    int8_t DRV_PIC32MZW_MemFree(void *pBufferAddr)

  Summary:
    Free general memory back to packet pool.

  Description:
    Frees memory back to the packet pool.

  Precondition:
    TCP/IP stack must be initialized.

  Parameters:
    pBufferAddr - Pointer to memory buffer to free.

  Returns:
    0 for failure, 1 for success

  Remarks:
    None.
*/

int8_t DRV_PIC32MZW_MemFree(DRV_PIC32MZW_ALLOC_OPT_ARGS void *pBufferAddr)
{
    DRV_PIC32MZW_MEM_ALLOC_HDR *pAllocHdr;

    if (NULL == pBufferAddr)
    {
        return 0;
    }

    pAllocHdr = _DRV_PIC32MZW_MemHdr(pBufferAddr);

    if (NULL == pAllocHdr)
    {
        return 0;
    }

    if (OSAL_RESULT_FALSE == OSAL_MUTEX_Lock(&pic32mzwMemMutex, OSAL_WAIT_FOREVER))
    {
        return 0;
    }

    pAllocHdr->users--;

    if (pAllocHdr->users > 0)
    {
        OSAL_MUTEX_Unlock(&pic32mzwMemMutex);
        return 0;
    }

    if (-1 != pAllocHdr->priLevel)
    {
        g_pktmem_pri[pAllocHdr->priLevel].num_allocd--;

#ifdef WDRV_PIC32MZW_STATS_ENABLE
        if (OSAL_RESULT_TRUE == OSAL_MUTEX_Lock(&pic32mzwMemStatsMutex, OSAL_WAIT_FOREVER))
        {
            pic32mzMemStatistics.pri[pAllocHdr->priLevel].free++;
            pic32mzMemStatistics.pri[pAllocHdr->priLevel].totalNumAlloc--;
            pic32mzMemStatistics.pri[pAllocHdr->priLevel].freeSize += pAllocHdr->size;
            pic32mzMemStatistics.pri[pAllocHdr->priLevel].totalSizeAlloc -= pAllocHdr->size;
            OSAL_MUTEX_Unlock(&pic32mzwMemStatsMutex);
        }
#endif

        if (((void*)pAllocHdr >= (void*)0xa0040000) && ((void*)pAllocHdr <= (void*)0xa0050000))
        {
            if (true == pic32mzwDescriptor[1].isInit)
            {
                _DRV_PIC32MZW_PktListAdd(&pic32mzwRsrvPktList, (WDRV_PIC32MZW_PKT_LIST_NODE*)pAllocHdr);
            }

#ifdef DRV_PIC32MZW_TRACK_MEMORY_ALLOC
            _DRV_PIC32MZW_MemTrackerRemove(pBufferAddr);
#endif
            OSAL_MUTEX_Unlock(&pic32mzwMemMutex);
            return 1;
        }
    }

#ifdef WDRV_PIC32MZW_STATS_ENABLE
    if (OSAL_RESULT_TRUE == OSAL_MUTEX_Lock(&pic32mzwMemStatsMutex, OSAL_WAIT_FOREVER))
    {
        pic32mzMemStatistics.mem.free++;
        pic32mzMemStatistics.mem.totalNumAlloc--;
        pic32mzMemStatistics.mem.freeSize += pAllocHdr->size;
        pic32mzMemStatistics.mem.totalSizeAlloc -= pAllocHdr->size;
        OSAL_MUTEX_Unlock(&pic32mzwMemStatsMutex);
    }
#endif

#ifdef DRV_PIC32MZW_TRACK_MEMORY_ALLOC
    _DRV_PIC32MZW_MemTrackerRemove(pBufferAddr);
#endif

    OSAL_Free(pAllocHdr->pUnalignedPtr);

    OSAL_MUTEX_Unlock(&pic32mzwMemMutex);

    return 1;
}

//*******************************************************************************
/*
  Function:
    int8_t DRV_PIC32MZW_MemAddUsers(void *pBufferAddr, int count)

  Summary:
    Increases the user count of a memory allocation.

  Description:
    Increases the user count of a memory allocation.

  Precondition:
    TCP/IP stack must be initialized.

  Parameters:
    pBufferAddr - Pointer to memory buffer to update.
    count       - Number of users to add.

  Returns:
    0 for failure, 1 for success

  Remarks:
    None.
*/

int8_t DRV_PIC32MZW_MemAddUsers(DRV_PIC32MZW_ALLOC_OPT_ARGS void *pBufferAddr, int count)
{
    DRV_PIC32MZW_MEM_ALLOC_HDR *pAllocHdr;

    if (NULL == pBufferAddr)
    {
        return 0;
    }

    pAllocHdr = _DRV_PIC32MZW_MemHdr(pBufferAddr);

    if (NULL == pAllocHdr)
    {
        return 0;
    }

    if (OSAL_RESULT_FALSE == OSAL_MUTEX_Lock(&pic32mzwMemMutex, OSAL_WAIT_FOREVER))
    {
        return 0;
    }

    pAllocHdr->users++;

    OSAL_MUTEX_Unlock(&pic32mzwMemMutex);

    return 1;
}

//*******************************************************************************
/*
  Function:
    void* DRV_PIC32MZW_PacketMemAlloc(uint16_t size, MEM_PRIORITY_LEVEL_T priLevel)

  Summary:
    Allocates packet memory.

  Description:
    Allocates a new packet from the TCP/IP memory, unless the priLevel indicates
    this is a high priority receive packet, then it is allocated from the high
    priority receive packet queue.

  Precondition:
    TCP/IP stack must be initialized.

  Parameters:
    size     - Size of memory to allocate.
    priLevel - Priority level, see MEM_PRIORITY_LEVEL_T.

  Returns:
    Pointer to allocated memory, or NULL for error.

  Remarks:
    None.
*/

void* DRV_PIC32MZW_PacketMemAlloc(DRV_PIC32MZW_ALLOC_OPT_ARGS uint16_t size, MEM_PRIORITY_LEVEL_T priLevel)
{
    void *pBufferAddr;
    DRV_PIC32MZW_MEM_ALLOC_HDR *pAllocHdr;
    WDRV_PIC32MZW_PKT_LIST_NODE *pNode = NULL;

    if (priLevel >= NUM_MEM_PRI_LEVELS)
    {
        WDRV_DBG_ERROR_PRINT("PktMemAlloc: Invalid priority level %d\r\n", priLevel);
        return NULL;
    }

    if ((true == pic32mzwDescriptor[1].isInit) && (size <= SHARED_PKT_MEM_BUFFER_SIZE))
    {
        pNode = _DRV_PIC32MZW_PktListRemove(&pic32mzwRsrvPktList);
    }

    if (NULL != pNode)
    {
        pBufferAddr = pNode->hdr.memory;

        pNode->hdr.pNext     = NULL;
        pNode->hdr.size      = size;
        pNode->hdr.users     = 1;
        pNode->hdr.pAllocPtr = NULL;
#ifdef DRV_PIC32MZW_TRACK_MEMORY_ALLOC
        _DRV_PIC32MZW_MemTrackerAdd(DRV_PIC32MZW_ALLOC_OPT_PARAMS_V pBufferAddr, size);
#endif
    }
    else
    {
        pBufferAddr = DRV_PIC32MZW_MemAlloc(DRV_PIC32MZW_ALLOC_OPT_PARAMS_V size);

        if (NULL == pBufferAddr)
        {
#ifdef WDRV_PIC32MZW_STATS_ENABLE
            /* Rate limit error output */
            if (OSAL_RESULT_TRUE == OSAL_MUTEX_Lock(&pic32mzwMemStatsMutex, OSAL_WAIT_FOREVER))
            {
                uint32_t gen;

                gen = ++pic32mzMemStatistics.err.gen;
                OSAL_MUTEX_Unlock(&pic32mzwMemStatsMutex);

                if (gen > 10)
                {
                    return NULL;
                }
            }
#endif
            WDRV_DBG_ERROR_PRINT("PktMemAlloc: Alloc NULL\r\n");
            return NULL;
        }
    }

    pAllocHdr = _DRV_PIC32MZW_MemHdr(pBufferAddr);

    if (NULL == pAllocHdr)
    {
        return NULL;
    }

    pAllocHdr->priLevel = priLevel;

    g_pktmem_pri[priLevel].num_allocd++;

#ifdef WDRV_PIC32MZW_STATS_ENABLE
    if (OSAL_RESULT_TRUE == OSAL_MUTEX_Lock(&pic32mzwMemStatsMutex, OSAL_WAIT_FOREVER))
    {
        pic32mzMemStatistics.pri[priLevel].alloc++;
        pic32mzMemStatistics.pri[priLevel].totalNumAlloc++;
        pic32mzMemStatistics.pri[priLevel].allocSize += size;
        pic32mzMemStatistics.pri[priLevel].totalSizeAlloc += size;
        OSAL_MUTEX_Unlock(&pic32mzwMemStatsMutex);
    }
#endif

    return pBufferAddr;
}

//*******************************************************************************
/*
  Function:
    void DRV_PIC32MZW_PacketMemFree(void *pPktBuff)

  Summary:
    Frees packet memory.

  Description:
    Frees memory from the packet memory.

  Precondition:
    TCP/IP stack must be initialized.

  Parameters:
    pPktBuff - Pointer to packet memory buffer to be freed.

  Returns:
    None.

  Remarks:
    None.
*/

void DRV_PIC32MZW_PacketMemFree(DRV_PIC32MZW_ALLOC_OPT_ARGS void *pPktBuff)
{
    DRV_PIC32MZW_MEM_ALLOC_HDR *pAllocHdr;

    if (NULL == pPktBuff)
    {
        return;
    }

    pAllocHdr = _DRV_PIC32MZW_MemHdr(pPktBuff);

    if (NULL == pAllocHdr)
    {
        return;
    }

    if (OSAL_RESULT_FALSE == OSAL_MUTEX_Lock(&pic32mzwMemMutex, OSAL_WAIT_FOREVER))
    {
        return;
    }

    if ((NULL != pAllocHdr->pAllocPtr) && (NULL != pic32mzwMACDescriptor.pktAckF))
    {
        OSAL_MUTEX_Unlock(&pic32mzwMemMutex);

        pic32mzwMACDescriptor.pktAckF(pAllocHdr->pAllocPtr, 0, TCPIP_THIS_MODULE_ID);
    }
    else
    {
        OSAL_MUTEX_Unlock(&pic32mzwMemMutex);

        DRV_PIC32MZW_MemFree(DRV_PIC32MZW_ALLOC_OPT_PARAMS pPktBuff);
    }
}

//*******************************************************************************
/*
  Function:
    void DRV_PIC32MZW_WIDRxQueuePush(void *pPktBuff)

  Summary:
    Pushes a WID packet buffer into the receive queue.

  Description:
    Pushes a WID packet buffer from the firmware into the receive queue for
    later processing.

  Precondition:
    None.

  Parameters:
    pPktBuff - Pointer to packet memory buffer to be pushed.

  Returns:
    None.

  Remarks:
    None.
*/

void DRV_PIC32MZW_WIDRxQueuePush(void *pPktBuff)
{
    DRV_PIC32MZW_MEM_ALLOC_HDR *pAllocHdr;

    if (NULL == pPktBuff)
    {
        return;
    }

    pAllocHdr = _DRV_PIC32MZW_MemHdr(pPktBuff);

    if (NULL == pAllocHdr)
    {
        return;
    }

    TCPIP_Helper_ProtectedSingleListTailAdd(&pic32mzwWIDRxQueue, (SGL_LIST_NODE*)pAllocHdr);
}

//*******************************************************************************
/*
  Function:
    void DRV_PIC32MZW_WIDTxQueuePush(void *pPktBuff)

  Summary:
    Pushes a WID packet buffer into the transmit queue.

  Description:
    Pushes a WID packet buffer from the driver into the transmit queue for
    later processing.

  Precondition:
    None.

  Parameters:
    pPktBuff - Pointer to packet memory buffer to be pushed.

  Returns:
    None.

  Remarks:
    None.
*/

void DRV_PIC32MZW_WIDTxQueuePush(void *pPktBuff)
{
    DRV_PIC32MZW_MEM_ALLOC_HDR *pAllocHdr;

    if (NULL == pPktBuff)
    {
        return;
    }

    pAllocHdr = _DRV_PIC32MZW_MemHdr(pPktBuff);

    if (NULL == pAllocHdr)
    {
        return;
    }

    TCPIP_Helper_SingleListTailAdd(&pic32mzwWIDTxQueue, (SGL_LIST_NODE*)pAllocHdr);
    OSAL_SEM_Post(&pic32mzwCtrlDescriptor.drvEventSemaphore);
}

//*******************************************************************************
/*
  Function:
    DRV_PIC32MZW_11I_MASK DRV_PIC32MZW_Get11iMask
    (
        WDRV_PIC32MZW_AUTH_TYPE authType,
        WDRV_PIC32MZW_AUTH_MOD_MASK authMod
    )

  Summary:
    Convert authentication type and modifiers to 11i info.

  Description:

  Precondition:
    None.

  Parameters:
    authType - Auth type to convert.
    authMod  - Modifiers to the authentication type.

  Returns:
    11i info mapped from auth type and modifiers.

  Remarks:
    None.
*/

DRV_PIC32MZW_11I_MASK DRV_PIC32MZW_Get11iMask
(
    WDRV_PIC32MZW_AUTH_TYPE authType,
    WDRV_PIC32MZW_AUTH_MOD_MASK authMod
)
{
    DRV_PIC32MZW_11I_MASK dot11iInfo;

    if (authType >= WDRV_PIC32MZW_AUTH_TYPE_MAX)
    {
        return 0;
    }

    /* Convert auth type to 11i info. */
    dot11iInfo = mapAuthTypeTo11i[authType];

    /* Apply any relevant modifiers. */
    if (authMod & WDRV_PIC32MZW_AUTH_MOD_MFP_REQ)
    {
        if (
                (WDRV_PIC32MZW_AUTH_TYPE_WPA2_PERSONAL == authType)
#ifdef WDRV_PIC32MZW_WPA3_PERSONAL_SUPPORT
            ||  (WDRV_PIC32MZW_AUTH_TYPE_WPA2WPA3_PERSONAL == authType)
#endif
#ifdef WDRV_PIC32MZW_ENTERPRISE_SUPPORT
            ||  (WDRV_PIC32MZW_AUTH_TYPE_WPA2_ENTERPRISE == authType)
            ||  (WDRV_PIC32MZW_AUTH_TYPE_WPA2WPA3_ENTERPRISE == authType)    
#endif
        )
        {
            dot11iInfo |= DRV_PIC32MZW_11I_BIPCMAC128 | DRV_PIC32MZW_11I_MFP_REQUIRED;
        }
    }
    else if (authMod & WDRV_PIC32MZW_AUTH_MOD_MFP_OFF)
    {
        if (
                (WDRV_PIC32MZW_AUTH_TYPE_WPAWPA2_PERSONAL == authType)
            ||  (WDRV_PIC32MZW_AUTH_TYPE_WPA2_PERSONAL == authType)
#ifdef WDRV_PIC32MZW_ENTERPRISE_SUPPORT
            ||  (WDRV_PIC32MZW_AUTH_TYPE_WPAWPA2_ENTERPRISE == authType)
            ||  (WDRV_PIC32MZW_AUTH_TYPE_WPA2_ENTERPRISE == authType)    
#endif
        )
        {
            dot11iInfo &= ~DRV_PIC32MZW_11I_BIPCMAC128;
        }
    }

    if (authMod & WDRV_PIC32MZW_AUTH_MOD_SHARED_KEY)
    {
        if (WDRV_PIC32MZW_AUTH_TYPE_WEP == authType)
        {
            dot11iInfo |= DRV_PIC32MZW_SKEY;
        }
    }

    if (authMod & (WDRV_PIC32MZW_AUTH_MOD_AP_TD))
    {
#ifdef WDRV_PIC32MZW_WPA3_PERSONAL_SUPPORT
        if (
                (WDRV_PIC32MZW_AUTH_TYPE_WPA2WPA3_PERSONAL == authType)
            ||  (WDRV_PIC32MZW_AUTH_TYPE_WPA3_PERSONAL == authType)
        )
        {
            dot11iInfo |= DRV_PIC32MZW_11I_TD;
        }
#endif
    }

    if (authMod & (WDRV_PIC32MZW_AUTH_MOD_STA_TD))
    {
#ifdef WDRV_PIC32MZW_WPA3_PERSONAL_SUPPORT
        if (dot11iInfo & DRV_PIC32MZW_11I_SAE)
        {
            dot11iInfo |= DRV_PIC32MZW_11I_MFP_REQUIRED;
            dot11iInfo |= DRV_PIC32MZW_11I_TD;
            dot11iInfo &= ~DRV_PIC32MZW_11I_PSK;
        }
#endif
#ifdef WDRV_PIC32MZW_ENTERPRISE_SUPPORT
        if (
                (dot11iInfo & DRV_PIC32MZW_11I_1X)
            &&  (dot11iInfo & DRV_PIC32MZW_11I_BIPCMAC128)
        )
        {
            dot11iInfo |= DRV_PIC32MZW_11I_MFP_REQUIRED;
            dot11iInfo |= DRV_PIC32MZW_11I_TD;
            dot11iInfo &= ~DRV_PIC32MZW_11I_TKIP;
        }
#endif
    }

    return dot11iInfo;
}

//*******************************************************************************
/*
  Function:
    void DRV_PIC32MZW_CryptoCallbackPush
    (
        DRV_PIC32MZW1_CRYPTO_CB fw_cb,
        DRV_PIC32MZW1_CRYPTO_STATUS_T status,
        uintptr_t context
    )

  Summary:
    Calls a firmware crypto callback with the relevant status and context.

  Description:
    Calls into the firmware with exclusive firmware access (drvAccessSemaphore),
    then kicks the main task (drvEventSemaphore) to run the normal firmware
    context.

  Precondition:
    None.

  Parameters:
    fw_cb   - Crypto callback provided by firmware.
    status  - Callback status.
    context - Context provided by firmware.

  Returns:
    None.

  Remarks:
    Caller must ensure parameters are valid.
*/

void DRV_PIC32MZW_CryptoCallbackPush
(
    DRV_PIC32MZW1_CRYPTO_CB fw_cb,
    DRV_PIC32MZW1_CRYPTO_STATUS_T status,
    uintptr_t context
)
{
    if (OSAL_RESULT_TRUE == OSAL_SEM_Pend(&pic32mzwCtrlDescriptor.drvAccessSemaphore, OSAL_WAIT_FOREVER))
    {
        fw_cb(status, context);
        OSAL_SEM_Post(&pic32mzwCtrlDescriptor.drvAccessSemaphore);
        OSAL_SEM_Post(&pic32mzwCtrlDescriptor.drvEventSemaphore);
    }
}
