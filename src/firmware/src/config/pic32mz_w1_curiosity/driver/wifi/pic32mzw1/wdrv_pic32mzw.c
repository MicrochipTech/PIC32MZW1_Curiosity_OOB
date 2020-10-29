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
Copyright (C) 2020 released Microchip Technology Inc.  All rights reserved.

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

#include "system_config.h"
#include "system_definitions.h"
#include "tcpip/tcpip_mac_object.h"
#include "tcpip/src/link_list.h"
#include "tcpip/src/tcpip_manager_control.h"
#include "wdrv_pic32mzw_api.h"
#include "wdrv_pic32mzw.h"
#include "wdrv_pic32mzw_debug.h"
#include "wdrv_pic32mzw_mac.h"
#include "wdrv_pic32mzw_cfg.h"
#include <sys/kmem.h>

extern uint8_t g_macaddress[6];
extern pktmem_priority_t g_pktmem_pri[NUM_MEM_PRI_LEVELS];

bool DRV_PIC32MZW_StoreBSSScanResult(const DRV_PIC32MZW_SCAN_RESULTS *const pScanResult);
bool DRV_PIC32MZW1_Crypto_Random_Init(CRYPT_RNG_CTX *pRngCtx);

// *****************************************************************************
// *****************************************************************************
// Section: PIC32MZW Driver Defines
// *****************************************************************************
// *****************************************************************************

#define TCPIP_THIS_MODULE_ID TCPIP_MODULE_MAC_PIC32MZW1

#define NUM_HIGH_PRI_PKTS                   5

#define SHARED_PKT_MEM_BUFFER_SIZE          1596

#define ETHERNET_HDR_LEN                    14
#define ETH_ETHERNET_HDR_OFFSET             34

#define ZERO_CP_MIN_MAC_FRAME_OFFSET       (ETH_ETHERNET_HDR_OFFSET + 4)

#define PIC32MZW_CACHE_LINE_SIZE            16

// *****************************************************************************
// *****************************************************************************
// Section: Global Data
// *****************************************************************************
// *****************************************************************************

/* This is user configurable function pointer for printf style output from driver. */
WDRV_PIC32MZW_DEBUG_PRINT_CALLBACK pfPIC32MZWDebugPrintCb;

/* This is the driver instance structure. */
static WDRV_PIC32MZW_DCPT pic32mzwDescriptor[2] =
{
    {
        .isInit = false,
        .sysStat = SYS_STATUS_UNINITIALIZED,
    },
    {
        .isInit = false,
        .sysStat = SYS_STATUS_UNINITIALIZED,
    }
};

/* This is the PIC32MZW1 MAC Object. */
const TCPIP_MAC_OBJECT WDRV_PIC32MZW1_MACObject =
{
    TCPIP_MODULE_MAC_PIC32MZW1,
    "PIC32MZW1",
    WDRV_PIC32MZW_Initialize,
    WDRV_PIC32MZW_Deinitialize,
    WDRV_PIC32MZW_Reinitialize,
    WDRV_PIC32MZW_Status,
    WDRV_PIC32MZW_MACTasks,
    WDRV_PIC32MZW_Open,
    WDRV_PIC32MZW_Close,
    WDRV_PIC32MZW_MACLinkCheck,
    WDRV_PIC32MZW_MACRxFilterHashTableEntrySet,
    WDRV_PIC32MZW_MACPowerMode,
    WDRV_PIC32MZW_MACPacketTx,
    WDRV_PIC32MZW_MACPacketRx,
    WDRV_PIC32MZW_MACProcess,
    WDRV_PIC32MZW_MACStatisticsGet,
    WDRV_PIC32MZW_MACParametersGet,
    WDRV_PIC32MZW_MACRegisterStatisticsGet,
    WDRV_PIC32MZW_MACConfigGet,
    WDRV_PIC32MZW_MACEventMaskSet,
    WDRV_PIC32MZW_MACEventAcknowledge,
    WDRV_PIC32MZW_MACEventPendingGet,
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
#ifdef WDRV_PIC32MZW_WPA3_SUPPORT
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

// *****************************************************************************
// *****************************************************************************
// Section: PIC32MZW MAC Driver Global Data
// *****************************************************************************
// *****************************************************************************

/* This is the Control driver instance descriptor. */
static WDRV_PIC32MZW_CTRLDCPT pic32mzwCtrlDescriptor;

/* This is the MAC driver instance descriptor. */
static WDRV_PIC32MZW_MACDCPT pic32mzwMACDescriptor;

/* This is the queue for high priority receive packets. */
static PROTECTED_SINGLE_LIST pic32mzwHighPriRxQueue;

/* This is the queue to hold discarded receive TCP/IP packets. */
static PROTECTED_SINGLE_LIST pic32mzwDiscardQueue;

/* This is the high priority receive packet storage. */
static WDRV_PIC32MZW_PKT_LIST_NODE pic32mzwHighPriPktList[NUM_HIGH_PRI_PKTS] __attribute__((coherent, aligned(PIC32MZW_CACHE_LINE_SIZE)));

/* This is the firmware to driver receive WID queue. */
static PROTECTED_SINGLE_LIST pic32mzwWIDRxQueue;

/* This is the driver to firmware transmit WID queue. */
static PROTECTED_SINGLE_LIST pic32mzwWIDTxQueue;

/* This is the memory allocation mutex. */
static OSAL_MUTEX_HANDLE_TYPE pic32mzwMemMutex;

#ifdef WDRV_PIC32MZW_STATS_ENABLE
/* This is the memory statistics mutex. */
static OSAL_MUTEX_HANDLE_TYPE pic32mzwMemStatsMutex;

/* This is the memory statistics structure. */
static WDRV_PIC32MZW_MAC_MEM_STATISTICS pic32mzMemStatistics;
#endif

// *****************************************************************************
// *****************************************************************************
// Section: PIC32MZW Driver Internal Implementation
// *****************************************************************************
// *****************************************************************************

//*******************************************************************************
/*
  Function:
    DRV_PIC32MZW_MEM_ALLOC_HDR* _DRV_PIC32MZW_MemHdr(void *pBufferAddr)

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

DRV_PIC32MZW_MEM_ALLOC_HDR* _DRV_PIC32MZW_MemHdr(void *pBufferAddr)
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
    pMacAddr - Pointer to MAC address to find or NULL

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
    if ((NULL == pDcpt) || (NULL == pRfMacConfig))
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
// Section: PIC32MZW Driver Callback Implementation
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

        pDcpt = &pic32mzwDescriptor[0];

        if (true == pDcpt->isInit)
        {
            return (SYS_MODULE_OBJ)pDcpt;
        }

        pfPIC32MZWDebugPrintCb = NULL;

        pic32mzwCtrlDescriptor.handle = DRV_HANDLE_INVALID;

        OSAL_SEM_Create(&pic32mzwCtrlDescriptor.drvAccessSemaphore, OSAL_SEM_TYPE_BINARY, 1, 1);

        OSAL_SEM_Create(&pic32mzwCtrlDescriptor.drvEventSemaphore, OSAL_SEM_TYPE_COUNTING, 10, 0);

        SYS_INT_SourceEnable(INT_SOURCE_RFMAC);
        SYS_INT_SourceEnable(INT_SOURCE_RFTM0);
        SYS_INT_SourceEnable(INT_SOURCE_RFSMC);

        TCPIP_Helper_ProtectedSingleListInitialize(&pic32mzwWIDRxQueue);
        TCPIP_Helper_ProtectedSingleListInitialize(&pic32mzwWIDTxQueue);

        OSAL_MUTEX_Create(&pic32mzwMemMutex);

        DRV_PIC32MZW1_Crypto_Random_Init(pInitData->pCryptRngCtx);

        pic32mzwCtrlDescriptor.rfMacConfigStatus = 0;

        pic32mzwCtrlDescriptor.regDomNameLength = 0;
        memset(pic32mzwCtrlDescriptor.regDomName, 0, WDRV_PIC32MZW_REGDOMAIN_MAX_NAME_LEN);

        if (NULL != pInitData->pRegDomName)
        {
            int length = strlen(pInitData->pRegDomName);

            if ((length > 0) && (length < WDRV_PIC32MZW_REGDOMAIN_MAX_NAME_LEN+1))
            {
                pic32mzwCtrlDescriptor.regDomNameLength = length;
                memcpy(pic32mzwCtrlDescriptor.regDomName, pInitData->pRegDomName, length);
            }
        }

#ifdef WDRV_PIC32MZW_STATS_ENABLE
        memset(&pic32mzMemStatistics, 0, sizeof(pic32mzMemStatistics));
        OSAL_MUTEX_Create(&pic32mzwMemStatsMutex);
#endif
    }
    else if (TCPIP_MODULE_MAC_PIC32MZW1 == index)
    {
        int i;
        const TCPIP_MAC_MODULE_CTRL* const stackData = ((TCPIP_MAC_INIT*)init)->macControl;

        if (NULL == stackData)
        {
            return SYS_MODULE_OBJ_INVALID;
        }

        pDcpt = &pic32mzwDescriptor[1];

        if (true == pDcpt->isInit)
        {
            return (SYS_MODULE_OBJ)pDcpt;
        }

        if (NULL != stackData)
        {
            if (TCPIP_MODULE_MAC_PIC32MZW1 != stackData->moduleId)
            {
                return SYS_MODULE_OBJ_INVALID;
            }

            TCPIP_Helper_ProtectedSingleListInitialize(&pic32mzwMACDescriptor.ethRxPktList);

            TCPIP_Helper_ProtectedSingleListInitialize(&pic32mzwHighPriRxQueue);

            TCPIP_Helper_ProtectedSingleListInitialize(&pic32mzwDiscardQueue);

            for (i=0; i<NUM_HIGH_PRI_PKTS; i++)
            {
                SGL_LIST_NODE* pListNode;

                pListNode = (SGL_LIST_NODE*)&pic32mzwHighPriPktList[i];

                if (IS_KVA0(pListNode))
                {
                    pListNode = KVA0_TO_KVA1(pListNode);
                }

                TCPIP_Helper_ProtectedSingleListTailAdd(&pic32mzwHighPriRxQueue, pListNode);
            }

            pic32mzwMACDescriptor.handle       = DRV_HANDLE_INVALID;

            pic32mzwMACDescriptor.eventF       = stackData->eventF;
            pic32mzwMACDescriptor.pktAllocF    = stackData->pktAllocF;
            pic32mzwMACDescriptor.pktFreeF     = stackData->pktFreeF;
            pic32mzwMACDescriptor.pktAckF      = stackData->pktAckF;
            pic32mzwMACDescriptor.eventParam   = stackData->eventParam;
            pic32mzwMACDescriptor.eventMask    = 0;
            pic32mzwMACDescriptor.events       = 0;
            OSAL_SEM_Create(&pic32mzwMACDescriptor.eventSemaphore, OSAL_SEM_TYPE_BINARY, 1, 1);
        }
    }
    else
    {
        return (SYS_MODULE_OBJ)NULL;
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

    pDcpt->sysStat = SYS_STATUS_UNINITIALIZED;

    if (pDcpt == &pic32mzwDescriptor[0])
    {
        SYS_INT_SourceDisable(INT_SOURCE_RFMAC);
        SYS_INT_SourceDisable(INT_SOURCE_RFTM0);
        SYS_INT_SourceDisable(INT_SOURCE_RFSMC);

        OSAL_SEM_Post(&pic32mzwCtrlDescriptor.drvEventSemaphore);
    }
    else if (pDcpt == &pic32mzwDescriptor[1])
    {
        OSAL_SEM_Delete(&pic32mzwMACDescriptor.eventSemaphore);

        pic32mzwMACDescriptor.eventF       = NULL;
        pic32mzwMACDescriptor.pktAllocF    = NULL;
        pic32mzwMACDescriptor.pktFreeF     = NULL;
        pic32mzwMACDescriptor.pktAckF      = NULL;
    }

    /* Clear internal state. */
    pDcpt->isOpen  = false;
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
    return ((WDRV_PIC32MZW_DCPT *)object)->sysStat;
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

    if (NULL == pDcpt)
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
            pDcpt->sysStat = SYS_STATUS_READY;
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

    switch (pDcpt->sysStat)
    {
        /* Uninitialised state. */
        case SYS_STATUS_UNINITIALIZED:
        {
            if (true == pDcpt->isInit)
            {
                OSAL_SEM_Delete(&pic32mzwCtrlDescriptor.drvEventSemaphore);
                OSAL_SEM_Delete(&pic32mzwCtrlDescriptor.drvAccessSemaphore);

                OSAL_MUTEX_Delete(&pic32mzwMemMutex);
#ifdef WDRV_PIC32MZW_STATS_ENABLE
                OSAL_MUTEX_Delete(&pic32mzwMemStatsMutex);
#endif
                pDcpt->isInit = false;
            }

            break;
        }

        case SYS_STATUS_BUSY:
        {
            int length;

            if (OSAL_RESULT_TRUE == OSAL_SEM_Pend(&pic32mzwCtrlDescriptor.drvAccessSemaphore, 0))
            {
                wdrv_pic32mzw_user_main();
                OSAL_SEM_Post(&pic32mzwCtrlDescriptor.drvAccessSemaphore);
                pDcpt->sysStat = SYS_STATUS_READY_EXTENDED;

                length = pDcpt->pCtrl->regDomNameLength;

                if ((length > 0) && (length < (WDRV_PIC32MZW_REGDOMAIN_MAX_NAME_LEN+1)))
                {
                    DRV_PIC32MZW_WIDCTX wids;

                    pDcpt->pCtrl->extSysStat = WDRV_PIC32MZW_SYS_STATUS_RF_INIT_BUSY;

                    /* Allocate memory for the WIDs. */
                    DRV_PIC32MZW_MultiWIDInit(&wids, 32);

                    DRV_PIC32MZW_MultiWIDAddData(&wids, DRV_WIFI_WID_REG_DOMAIN, (uint8_t*)pDcpt->pCtrl->regDomName, WDRV_PIC32MZW_REGDOMAIN_MAX_NAME_LEN);

                    /* Write the wids. */
                    if (false == DRV_PIC32MZW_MultiWid_Write(&wids))
                    {
                        WDRV_DBG_ERROR_PRINT("Reg Domain: Set failed\r\n");
                        return;
                    }
                }
                else
                {
                    pDcpt->pCtrl->extSysStat = WDRV_PIC32MZW_SYS_STATUS_RF_CONF_MISSING;
                }

            }
            break;
        }

        /* Running steady state. */
        case SYS_STATUS_READY:
        case SYS_STATUS_READY_EXTENDED:
        {
            DRV_PIC32MZW_MEM_ALLOC_HDR *pAllocHdr;
            int numDiscard;

            OSAL_SEM_Pend(&pic32mzwCtrlDescriptor.drvEventSemaphore, OSAL_WAIT_FOREVER);

            if (OSAL_RESULT_TRUE == OSAL_SEM_Pend(&pic32mzwCtrlDescriptor.drvAccessSemaphore, 0))
            {
                if (TCPIP_Helper_ProtectedSingleListCount(&pic32mzwWIDTxQueue) > 0)
                {
                    pAllocHdr = (DRV_PIC32MZW_MEM_ALLOC_HDR*)TCPIP_Helper_ProtectedSingleListHeadRemove(&pic32mzwWIDTxQueue);

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
                        if (NULL != pic32mzwMACDescriptor.pktFreeF)
                        {
                            pic32mzwMACDescriptor.pktFreeF(ptrPacket);
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

void WDRV_PIC32MZW_DebugRegisterCallback
(
    WDRV_PIC32MZW_DEBUG_PRINT_CALLBACK const pfDebugPrintCallback
)
{
    pfPIC32MZWDebugPrintCb = pfDebugPrintCallback;
}

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
    }
    else if (TCPIP_MODULE_MAC_PIC32MZW1 == index)
    {
        pDcpt = &pic32mzwDescriptor[1];
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

    if (false == pDcpt->isOpen)
    {
        pDcpt->isOpen = true;

        if (WDRV_PIC32MZW_SYS_IDX_0 == index)
        {
            int i;

            pDcpt->pCtrl->handle                = (DRV_HANDLE)pDcpt;
            pDcpt->pCtrl->isAP                  = false;
            pDcpt->pCtrl->connectedState        = WDRV_PIC32MZW_CONN_STATE_DISCONNECTED;
            pDcpt->pCtrl->scanInProgress        = false;
            pDcpt->pCtrl->scanActiveScanTime    = DRV_PIC32MZW_DEFAULT_ACTIVE_SCAN_TIME;
            pDcpt->pCtrl->scanPassiveListenTime = DRV_PIC32MZW_DEFAULT_PASSIVE_SCAN_TIME;
            pDcpt->pCtrl->opChannel             = WDRV_PIC32MZW_CID_ANY;
            pDcpt->pCtrl->scanChannelMask24     = WDRV_PIC32MZW_CM_2_4G_DEFAULT;
            pDcpt->pCtrl->pfBSSFindNotifyCB     = NULL;
            pDcpt->pCtrl->pfConnectNotifyCB     = NULL;
            pDcpt->pCtrl->pfAssociationRSSICB   = NULL;
            pDcpt->pCtrl->pfRegDomCB            = NULL;

            pDcpt->pCtrl->assocInfoSTA.handle = DRV_HANDLE_INVALID;
            pDcpt->pCtrl->assocInfoSTA.rssi   = 0;
            pDcpt->pCtrl->assocInfoSTA.peerAddress.valid = false;

            for (i=0; i<WDRV_PIC32MZW_NUM_ASSOCS; i++)
            {
                pDcpt->pCtrl->assocInfoAP[i].handle = DRV_HANDLE_INVALID;
                pDcpt->pCtrl->assocInfoAP[i].peerAddress.valid = false;
            }
        }
        else
        {
            pDcpt->pMac->handle = (DRV_HANDLE)pDcpt;
        }

        return (DRV_HANDLE)pDcpt;
    }

    return DRV_HANDLE_INVALID;
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
    int i;

    /* Ensure the driver handle is valid. */
    if (NULL == pDcpt)
    {
        return;
    }

    if (handle == pDcpt->pCtrl->handle)
    {
        pDcpt->pCtrl->connectedState = WDRV_PIC32MZW_CONN_STATE_DISCONNECTED;

        pDcpt->pCtrl->assocInfoSTA.handle = DRV_HANDLE_INVALID;

        for (i=0; i<WDRV_PIC32MZW_NUM_ASSOCS; i++)
        {
            pDcpt->pCtrl->assocInfoAP[i].handle = DRV_HANDLE_INVALID;
        }
    }

    pDcpt->isOpen = false;
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

    if ((NULL == pDcpt) || (NULL == pStats))
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
    bool WDRV_PIC32MZW_MACLinkCheck(DRV_HANDLE hMac)

  Summary:
    Indicates the state of the network link.

  Description:
    Returns a flag indicating if the network link is active or not.

  Remarks:
    See wdrv_pic32mzw_mac.h for usage information.

*/

bool WDRV_PIC32MZW_MACLinkCheck(DRV_HANDLE hMac)
{
    WDRV_PIC32MZW_DCPT *const pDcpt = (WDRV_PIC32MZW_DCPT *const)hMac;

    if ((NULL == pDcpt) || (NULL == pDcpt->pCtrl))
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
        DRV_HANDLE hMac,
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
    DRV_HANDLE hMac,
    const TCPIP_MAC_ADDR* DestMACAddr
)
{
    return TCPIP_MAC_RES_OK;
}

//*******************************************************************************
/*
  Function:
    bool WDRV_PIC32MZW_MACPowerMode(DRV_HANDLE hMac, TCPIP_MAC_POWER_MODE pwrMode)

  Summary:
    Change the power mode.

  Description:
    Not currently supported.

  Remarks:
    See wdrv_pic32mzw_mac.h for usage information.

*/

bool WDRV_PIC32MZW_MACPowerMode(DRV_HANDLE hMac, TCPIP_MAC_POWER_MODE pwrMode)
{
    return true;
}

//*******************************************************************************
/*
  Function:
    TCPIP_MAC_RES WDRV_PIC32MZW_MACPacketTx(DRV_HANDLE hMac, TCPIP_MAC_PACKET* ptrPacket)

  Summary:
    Send an Ethernet frame via the PIC32MZW.

  Description:
    Takes an Ethernet frame from the TCP/IP stack and schedules it with the
      PIC32MZW.

  Remarks:
    See wdrv_pic32mzw_mac.h for usage information.

*/

TCPIP_MAC_RES WDRV_PIC32MZW_MACPacketTx(DRV_HANDLE hMac, TCPIP_MAC_PACKET* ptrPacket)
{
    WDRV_PIC32MZW_DCPT *const pDcpt = (WDRV_PIC32MZW_DCPT *const)hMac;
    uint8_t *payLoadPtr;
    int pktLen = 0;
    uint8_t pktTos = 0;

    if ((NULL == pDcpt) || (NULL == pDcpt->pMac))
    {
        return false;
    }

    if (NULL == ptrPacket)
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

    if ((NULL == ptrPacket->pDSeg->next) && (ptrPacket->pDSeg->segLoadOffset >= ZERO_CP_MIN_MAC_FRAME_OFFSET))
    {
        uint32_t* p = (uint32_t*)(ptrPacket->pDSeg->segLoad - ptrPacket->pDSeg->segLoadOffset);

        *p++ = (uint32_t)ptrPacket;  //Save the packet pointer

        payLoadPtr = (uint8_t*)p;

        pktLen = ptrPacket->pDSeg->segLen;

        ptrPacket->pktFlags |= TCPIP_MAC_PKT_FLAG_QUEUED;
    }
    else
    {
        uint8_t *pktbuf;
        TCPIP_MAC_DATA_SEGMENT* pDseg;

        pktLen = 0;
        pDseg  = ptrPacket->pDSeg;

        while (NULL != pDseg)
        {
            pktLen += pDseg->segLen;

            pDseg = pDseg->next;
        };

        if (pktLen > SHARED_PKT_MEM_BUFFER_SIZE)
        {
            if (NULL != pDcpt->pMac->pktAckF)
            {
                pDcpt->pMac->pktAckF(ptrPacket,
                    TCPIP_MAC_PKT_ACK_MAC_REJECT_ERR, TCPIP_THIS_MODULE_ID);
            }

            WDRV_DBG_TRACE_PRINT("MAC TX: payload too big (%d)\r\n", pktLen);

            return TCPIP_MAC_RES_OP_ERR;
        }

        pktbuf = payLoadPtr = DRV_PIC32MZW_PacketMemAlloc(
                ETH_ETHERNET_HDR_OFFSET + pktLen, MEM_PRI_TX);

        if (NULL == pktbuf)
        {
            if (NULL != pDcpt->pMac->pktAckF)
            {
                pDcpt->pMac->pktAckF(ptrPacket,
                    TCPIP_MAC_PKT_ACK_MAC_REJECT_ERR, TCPIP_THIS_MODULE_ID);
            }

            WDRV_DBG_TRACE_PRINT("MAC TX: malloc fail\r\n");

            return TCPIP_MAC_RES_OP_ERR;
        }

        pktbuf += ETH_ETHERNET_HDR_OFFSET;

        pDseg = ptrPacket->pDSeg;

        while (NULL != pDseg)
        {
            memcpy(pktbuf, pDseg->segLoad, pDseg->segLen);

            pktbuf += pDseg->segLen;

            pDseg = pDseg->next;
        };

        if (NULL != pDcpt->pMac->pktAckF)
        {
            pDcpt->pMac->pktAckF(ptrPacket, TCPIP_MAC_PKT_ACK_TX_OK, TCPIP_THIS_MODULE_ID);
        }
    }

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
        DRV_HANDLE hMac,
        TCPIP_MAC_RES* pRes,
        const TCPIP_MAC_PACKET_RX_STAT** ppPktStat
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
    DRV_HANDLE hMac,
    TCPIP_MAC_RES* pRes,
    const TCPIP_MAC_PACKET_RX_STAT** ppPktStat
)
{
    TCPIP_MAC_PACKET* ptrPacket;

    ptrPacket = (TCPIP_MAC_PACKET*)TCPIP_Helper_ProtectedSingleListHeadRemove(&pic32mzwMACDescriptor.ethRxPktList);

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
    TCPIP_MAC_RES WDRV_PIC32MZW_MACProcess(DRV_HANDLE hMac)

  Summary:
    Regular update to MAC state machine.

  Description:
    Called by the TCP/IP to update the internal state machine.

  Remarks:
    See wdrv_pic32mzw_mac.h for usage information.

*/

TCPIP_MAC_RES WDRV_PIC32MZW_MACProcess(DRV_HANDLE hMac)
{
    return TCPIP_MAC_RES_OK;
}

//*******************************************************************************
/*
  Function:
    TCPIP_MAC_RES WDRV_PIC32MZW_MACStatisticsGet
    (
        DRV_HANDLE hMac,
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
    DRV_HANDLE hMac,
    TCPIP_MAC_RX_STATISTICS* pRxStatistics,
    TCPIP_MAC_TX_STATISTICS* pTxStatistics
)
{
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
        DRV_HANDLE hMac,
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
    DRV_HANDLE hMac,
    TCPIP_MAC_PARAMETERS* pMacParams
)
{
    WDRV_PIC32MZW_DCPT *const pDcpt = (WDRV_PIC32MZW_DCPT *const)hMac;

    /* Ensure the driver handle and user pointer is valid. */
    if ((NULL == pDcpt) || (NULL == pMacParams))
    {
        return TCPIP_MAC_RES_IS_BUSY;
    }

    if (SYS_STATUS_READY == pDcpt->sysStat)
    {
        if (NULL != pMacParams)
        {
            if (OSAL_RESULT_TRUE == OSAL_SEM_Pend(&pic32mzwCtrlDescriptor.drvAccessSemaphore, OSAL_WAIT_FOREVER))
            {
                memcpy(pMacParams->ifPhyAddress.v, g_macaddress, 6);
                OSAL_SEM_Post(&pic32mzwCtrlDescriptor.drvAccessSemaphore);
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
        DRV_HANDLE hMac,
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
    DRV_HANDLE hMac,
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
        TCPIP_MODULE_MAC_ID modId,
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
    TCPIP_MODULE_MAC_ID modId,
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
        DRV_HANDLE hMac,
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
    DRV_HANDLE hMac,
    TCPIP_MAC_EVENT macEvents,
    bool enable
)
{
    if (OSAL_RESULT_TRUE == OSAL_SEM_Pend(&pic32mzwMACDescriptor.eventSemaphore, OSAL_WAIT_FOREVER))
    {
        if (true == enable)
        {
            pic32mzwMACDescriptor.eventMask |= macEvents;
        }
        else
        {
            pic32mzwMACDescriptor.eventMask &= ~macEvents;
        }

        OSAL_SEM_Post(&pic32mzwMACDescriptor.eventSemaphore);

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
    bool WDRV_PIC32MZW_MACEventAcknowledge(DRV_HANDLE hMac, TCPIP_MAC_EVENT macEvents)

  Summary:
    Acknowledge an event.

  Description:
    Indicates that certain events are to be acknowledged and cleared.

  Remarks:
    See wdrv_pic32mzw_mac.h for usage information.

*/

bool WDRV_PIC32MZW_MACEventAcknowledge(DRV_HANDLE hMac, TCPIP_MAC_EVENT macEvents)
{
    if (OSAL_RESULT_TRUE == OSAL_SEM_Pend(&pic32mzwMACDescriptor.eventSemaphore, OSAL_WAIT_FOREVER))
    {
        pic32mzwMACDescriptor.events &= ~macEvents;
        OSAL_SEM_Post(&pic32mzwMACDescriptor.eventSemaphore);

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
    TCPIP_MAC_EVENT WDRV_PIC32MZW_MACEventPendingGet(DRV_HANDLE hMac)

  Summary:
    Retrieve the current events.

  Description:
    Returns the current event state.

  Remarks:
    See wdrv_pic32mzw_mac.h for usage information.

*/

TCPIP_MAC_EVENT WDRV_PIC32MZW_MACEventPendingGet(DRV_HANDLE hMac)
{
    TCPIP_MAC_EVENT events;

    if (OSAL_RESULT_TRUE == OSAL_SEM_Pend(&pic32mzwMACDescriptor.eventSemaphore, OSAL_WAIT_FOREVER))
    {
        events = pic32mzwMACDescriptor.events;
        OSAL_SEM_Post(&pic32mzwMACDescriptor.eventSemaphore);

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
    if ((NULL == pDcpt) || (NULL == pMACAddress))
    {
        return WDRV_PIC32MZW_STATUS_INVALID_ARG;
    }

    /* Ensure the driver instance has been opened for use. */
    if (false == pDcpt->isOpen)
    {
        return WDRV_PIC32MZW_STATUS_NOT_OPEN;
    }

    if (OSAL_RESULT_TRUE == OSAL_SEM_Pend(&pic32mzwCtrlDescriptor.drvAccessSemaphore, OSAL_WAIT_FOREVER))
    {
        memcpy(pMACAddress, g_macaddress, 6);
        OSAL_SEM_Post(&pic32mzwCtrlDescriptor.drvAccessSemaphore);
    }

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
    if ((NULL == pDcpt) || (NULL == pOpChan))
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

    if (false == pDcpt->isInit)
    {
        return;
    }

    WDRV_DBG_VERBOSE_PRINT("%s: 0x%04x %d\r\n", __FUNCTION__, wid, length);

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
                pCtrl->assocInfoSTA.rssi = *pData;

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

            WDRV_DBG_VERBOSE_PRINT("WIFI_WID_ASSOC_STAT: %d, %d\r\n", *pData, pCtrl->connectedState);

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
                    pCtrl->assocInfoSTA.handle            = (DRV_HANDLE)pCtrl;
                    pCtrl->assocInfoSTA.peerAddress.valid = false;
                    pCtrl->assocInfoSTA.authType          = WDRV_PIC32MZW_AUTH_TYPE_DEFAULT;
                    pCtrl->assocInfoSTA.rssi              = 0;

                    memset(&pCtrl->assocInfoSTA.peerAddress.addr, 0, WDRV_PIC32MZW_MAC_ADDR_LEN);

                    DRV_PIC32MZW_MultiWIDInit(&wids, 32);
                    DRV_PIC32MZW_MultiWIDAddQuery(&wids, DRV_WIFI_WID_RSSI);
                    DRV_PIC32MZW_MultiWIDAddQuery(&wids, DRV_WIFI_WID_BSSID);
                    DRV_PIC32MZW_MultiWIDAddQuery(&wids, DRV_WIFI_WID_CURR_OPER_CHANNEL);
                    DRV_PIC32MZW_MultiWid_Write(&wids);

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
            if (length < DRV_PIC32MZW_REGDOMAIN_RES_LEN)
            {
                break;
            }

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

                    pCtrl->pfRegDomCB((DRV_HANDLE)pDcpt, pData[0], pData[1], current, &regDomInfo);
                }
            }

            break;
        }

        case DRV_WIFI_WID_RF_MAC_CONFIG_STATUS:
        {
            if (SYS_STATUS_READY_EXTENDED == pDcpt->sysStat)
            {
                if (NULL != pData)
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
                else
                {
                    pDcpt->pCtrl->extSysStat = WDRV_PIC32MZW_SYS_STATUS_RF_CONF_MISSING;
                }
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
        if (0 == DRV_PIC32MZW_MemFree(pAllocHdr->memory))
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
    TCPIP_MAC_PACKET *ptrPacket;
    TCPIP_MAC_EVENT events;
    DRV_PIC32MZW_MEM_ALLOC_HDR *pAllocHdr;
    void *pBufferAddr;

    if (NULL == pEthMsg)
    {
        return;
    }

    pBufferAddr = (void*)&pEthMsg[-hdrOffset];

    pAllocHdr = _DRV_PIC32MZW_MemHdr(pBufferAddr);

    if (NULL == pAllocHdr)
    {
        return;
    }

    if (NULL == pAllocHdr->pAllocPtr)
    {
        ptrPacket = NULL;

        if (NULL != pic32mzwMACDescriptor.pktAllocF)
        {
            ptrPacket = pic32mzwMACDescriptor.pktAllocF(sizeof(TCPIP_MAC_PACKET), 0, 0);
        }

        if (NULL == ptrPacket)
        {
            DRV_PIC32MZW_MemFree(pBufferAddr);
            return;
        }
    }
    else
    {
        ptrPacket = pAllocHdr->pAllocPtr;
    }

    ptrPacket->ackFunc = _DRV_PIC32MZW_AllocPktCallback;
    ptrPacket->ackParam = pAllocHdr;

    pAllocHdr->pAllocPtr = ptrPacket;

    ptrPacket->pDSeg->segLen = lengthEthMsg;
    ptrPacket->pDSeg->segSize = lengthEthMsg;
    ptrPacket->pDSeg->segLoad = (void*)pEthMsg;
    ptrPacket->pDSeg->segLoadOffset = hdrOffset + sizeof(void*);
    ptrPacket->pktFlags |= TCPIP_MAC_PKT_FLAG_QUEUED;
    ptrPacket->tStamp = SYS_TMR_TickCountGet();
    ptrPacket->pMacLayer = ptrPacket->pDSeg->segLoad;

    ptrPacket->pNetLayer = ptrPacket->pMacLayer + ETHERNET_HDR_LEN;

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

void* DRV_PIC32MZW_MemAlloc(uint16_t size)
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

    OSAL_MUTEX_Unlock(&pic32mzwMemMutex);

    return pAllocHdr->memory;
}

//*******************************************************************************
/*
  Function:
    int8_t DRV_PIC32MZW_MemFree(void *buffer_addr)

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

int8_t DRV_PIC32MZW_MemFree(void *pBufferAddr)
{
    DRV_PIC32MZW_MEM_ALLOC_HDR *pAllocHdr;

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

        if (MEM_PRI_HPRX == pAllocHdr->priLevel)
        {
            TCPIP_Helper_ProtectedSingleListTailAdd(&pic32mzwHighPriRxQueue, (SGL_LIST_NODE*)pAllocHdr);

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

int8_t DRV_PIC32MZW_MemAddUsers(void *pBufferAddr, int count)
{
    DRV_PIC32MZW_MEM_ALLOC_HDR *pAllocHdr;

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

void* DRV_PIC32MZW_PacketMemAlloc(uint16_t size, MEM_PRIORITY_LEVEL_T priLevel)
{
    void *pBufferAddr;
    DRV_PIC32MZW_MEM_ALLOC_HDR *pAllocHdr;

    if (priLevel >= NUM_MEM_PRI_LEVELS)
    {
        WDRV_DBG_ERROR_PRINT("PktMemAlloc: Invalid priority level %d\r\n", priLevel);
        return NULL;
    }

    if (MEM_PRI_HPRX == priLevel)
    {
        WDRV_PIC32MZW_PKT_LIST_NODE *pNode;

        if (size > SHARED_PKT_MEM_BUFFER_SIZE)
        {
            return NULL;
        }

        pNode = (WDRV_PIC32MZW_PKT_LIST_NODE*)TCPIP_Helper_ProtectedSingleListHeadRemove(&pic32mzwHighPriRxQueue);

        if (NULL == pNode)
        {
#ifdef WDRV_PIC32MZW_STATS_ENABLE
            if (OSAL_RESULT_TRUE == OSAL_MUTEX_Lock(&pic32mzwMemStatsMutex, OSAL_WAIT_FOREVER))
            {
                uint32_t hprx;

                hprx = ++pic32mzMemStatistics.err.hprx;
                OSAL_MUTEX_Unlock(&pic32mzwMemStatsMutex);

                /* Rate limit error output */
                if (hprx > 10)
                {
                    return NULL;
                }
            }
#endif
            WDRV_DBG_ERROR_PRINT("PktMemAlloc: HPRX NULL\r\n");
            return NULL;
        }

        pBufferAddr = pNode->hdr.memory;

        pNode->hdr.pNext     = NULL;
        pNode->hdr.size      = size;
        pNode->hdr.users     = 1;
        pNode->hdr.pAllocPtr = NULL;
    }
    else
    {
        pBufferAddr = DRV_PIC32MZW_MemAlloc(size);

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

void DRV_PIC32MZW_PacketMemFree(void *pPktBuff)
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

        DRV_PIC32MZW_MemFree(pPktBuff);
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

    pAllocHdr = _DRV_PIC32MZW_MemHdr(pPktBuff);

    if (NULL == pAllocHdr)
    {
        return;
    }

    TCPIP_Helper_ProtectedSingleListTailAdd(&pic32mzwWIDTxQueue, (SGL_LIST_NODE*)pAllocHdr);
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
#ifdef WDRV_PIC32MZW_WPA3_SUPPORT
            ||  (WDRV_PIC32MZW_AUTH_TYPE_WPA2WPA3_PERSONAL == authType)
#endif
        )
        {
            dot11iInfo |= DRV_PIC32MZW_11I_BIPCMAC128 | DRV_PIC32MZW_11I_MFP_REQUIRED;
        }
    }

    if (authMod & WDRV_PIC32MZW_AUTH_MOD_MFP_OFF)
    {
        if (!(dot11iInfo & DRV_PIC32MZW_11I_MFP_REQUIRED))
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

    return dot11iInfo;
}
