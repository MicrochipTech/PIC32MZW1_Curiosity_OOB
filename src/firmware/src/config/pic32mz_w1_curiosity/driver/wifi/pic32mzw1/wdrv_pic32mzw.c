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
#include "tcpip/src/tcpip_packet.h"
#include "tcpip/src/link_list.h"
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

#define MEM_CHUNK_INFO_SIZE                 4
#define SHARED_PKT_MEM_BUFFER_SIZE          (1600 - MEM_CHUNK_INFO_SIZE)

#define TCPIP_MAC_FRAME_OFFSET              (34 + 4)

#define MAX_MAC_HDR_LEN                     26
#define SUB_MSDU_HEADER_LENGTH              14
#define SNAP_HDR_LEN                        8
#define ETHERNET_HDR_LEN                    14

#define ETH_ETHERNET_HDR_OFFSET             (MAX_MAC_HDR_LEN + SUB_MSDU_HEADER_LENGTH + \
                                             SNAP_HDR_LEN - ETHERNET_HDR_LEN)

#define TCPIP_SEG_MAC_HDR_OFFSET_SIZE       (sizeof(TCPIP_MAC_ETHERNET_HEADER) + TCPIP_MAC_FRAME_OFFSET)

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

/* This is a structure for holding a queued packet. */
typedef struct _TAG_HPR_LIST_NODE
{
    struct _TAG_HPR_LIST_NODE*  next;       // next node in list
    struct _TAG_HPR_LIST_NODE*  prev;       // previous node in list
    union
    {
        uint8_t             memory[1600];
        TCPIP_MAC_PACKET    macPacket;
    } pkt;
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
static DOUBLE_LIST pic32mzwHighPriRxQueue;

/* This is the high priority receive packet storage. */
static WDRV_PIC32MZW_PKT_LIST_NODE pic32mzwHighPriPktList[NUM_HIGH_PRI_PKTS];

/* This is the firmware to driver receive WID queue. */
static PROTECTED_SINGLE_LIST pic32mzwWIDRxQueue;

/* This is the driver to firmware transmit WID queue. */
static PROTECTED_SINGLE_LIST pic32mzwWIDTxQueue;

#ifdef WDRV_PIC32MZW_STATS_ENABLE
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
    TCPIP_MAC_PACKET* _DRV_PIC32MZW_BufToPacket(void *pPktBuff)

  Summary:
    Translate packet buffer payload pointer into packet structure.

  Description:
    Determines the TCP MAC packet structure address from the packet buffer.

  Precondition:
    None.

  Parameters:
    pPktBuff - Pointer to packet memory buffer.

  Returns:
    Pointer to TCP MAC packet structure.

  Remarks:
    The pointer to the TCP MAC packet is store in the four bytes preceeding the
    packet buffer memory.
*/

static TCPIP_MAC_PACKET* _DRV_PIC32MZW_BufToPacket(void *pPktBuff)
{
    return (TCPIP_MAC_PACKET*)*(uint32_t*)((uint8_t *)pPktBuff - sizeof(uint32_t*));
}

//*******************************************************************************
/*
  Function:
    void* _DRV_PIC32MZW_PacketToBuf(void *pPacket)

  Summary:
    Translate packet structure into packet buffer payload pointer.

  Description:
    Determines the packet buffer address from the TCP MAC packet structure.

  Precondition:
    None.

  Parameters:
    pPacket - Pointer to TCP MAC packet structure.

  Returns:
    Pointer to packet buffer.

  Remarks:
    None.
*/

static void* _DRV_PIC32MZW_PacketToBuf(TCPIP_MAC_PACKET *pPacket)
{
    return pPacket->pDSeg->segLoad;
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

        SYS_INT_SourceEnable(INT_SOURCE_RFMAC);
        SYS_INT_SourceEnable(INT_SOURCE_RFTM0);

        TCPIP_Helper_ProtectedSingleListInitialize(&pic32mzwWIDRxQueue);
        TCPIP_Helper_ProtectedSingleListInitialize(&pic32mzwWIDTxQueue);

        DRV_PIC32MZW1_Crypto_Random_Init(pInitData->pCryptRngCtx);
    }
    else if (TCPIP_MODULE_MAC_PIC32MZW1 == index)
    {
        int i;
        const TCPIP_MAC_MODULE_CTRL* const stackData = ((TCPIP_MAC_INIT*)init)->macControl;
        //const TCPIP_MODULE_MAC_PIC32MZW1_CONFIG* initData = (const TCPIP_MODULE_MAC_PIC32MZW1_CONFIG*)((TCPIP_MAC_INIT*)init)->moduleData;

        if (NULL == stackData)
        {
            return SYS_MODULE_OBJ_INVALID;
        }

        pDcpt = &pic32mzwDescriptor[1];

        if (true == pDcpt->isInit)
        {
            return (SYS_MODULE_OBJ)pDcpt;
        }

        if (TCPIP_MODULE_MAC_PIC32MZW1 != stackData->moduleId)
        {
            return SYS_MODULE_OBJ_INVALID;
        }

        TCPIP_Helper_ProtectedSingleListInitialize(&pic32mzwMACDescriptor.ethRxPktList);

        TCPIP_Helper_DoubleListInitialize(&pic32mzwHighPriRxQueue);

        for (i=0; i<NUM_HIGH_PRI_PKTS; i++)
        {
            TCPIP_Helper_DoubleListTailAdd(&pic32mzwHighPriRxQueue, (DBL_LIST_NODE*)&pic32mzwHighPriPktList[i]);
        }

        pic32mzwMACDescriptor.handle       = DRV_HANDLE_INVALID;

        pic32mzwMACDescriptor.eventF       = stackData->eventF;
        pic32mzwMACDescriptor.eventParam   = stackData->eventParam;
        pic32mzwMACDescriptor.eventMask    = 0;
        pic32mzwMACDescriptor.events       = 0;
        OSAL_SEM_Create(&pic32mzwMACDescriptor.eventSemaphore, OSAL_SEM_TYPE_BINARY, 1, 1);
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

#ifdef WDRV_PIC32MZW_STATS_ENABLE
    memset(&pic32mzMemStatistics, 0, sizeof(pic32mzMemStatistics));
#endif

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

    if (pDcpt == &pic32mzwDescriptor[0])
    {
        SYS_INT_SourceDisable(INT_SOURCE_RFMAC);
        SYS_INT_SourceDisable(INT_SOURCE_RFTM0);

        OSAL_SEM_Delete(&pic32mzwCtrlDescriptor.drvAccessSemaphore);
    }
    else if (pDcpt == &pic32mzwDescriptor[1])
    {
        OSAL_SEM_Delete(&pic32mzwMACDescriptor.eventSemaphore);
    }

    /* Clear internal state. */
    pDcpt->isInit  = false;
    pDcpt->isOpen  = false;
    pDcpt->sysStat = SYS_STATUS_UNINITIALIZED;
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
            break;
        }

        case SYS_STATUS_BUSY:
        {
                if (OSAL_RESULT_TRUE == OSAL_SEM_Pend(&pic32mzwCtrlDescriptor.drvAccessSemaphore, 0))
                {
                    wdrv_pic32mzw_user_main();
                    OSAL_SEM_Post(&pic32mzwCtrlDescriptor.drvAccessSemaphore);
                pDcpt->sysStat = SYS_STATUS_READY;
            }

            break;
        }

        /* Running steady state. */
        case SYS_STATUS_READY:
        {
            SGL_LIST_NODE *pNode;

            if (OSAL_RESULT_TRUE == OSAL_SEM_Pend(&pic32mzwCtrlDescriptor.drvAccessSemaphore, 0))
            {
                if (TCPIP_Helper_ProtectedSingleListCount(&pic32mzwWIDTxQueue) > 0)
                {
                    pNode = TCPIP_Helper_ProtectedSingleListHeadRemove(&pic32mzwWIDTxQueue);

                    if (NULL != pNode)
                    {
                        wdrv_pic32mzw_process_cfg_message(_DRV_PIC32MZW_PacketToBuf((TCPIP_MAC_PACKET*)pNode));
                    }
                }

                wdrv_pic32mzw_mac_controller_task();

                OSAL_SEM_Post(&pic32mzwCtrlDescriptor.drvAccessSemaphore);
            }

            if (TCPIP_Helper_ProtectedSingleListCount(&pic32mzwWIDRxQueue) > 0)
            {
                pNode = TCPIP_Helper_ProtectedSingleListHeadRemove(&pic32mzwWIDRxQueue);

                if (NULL != pNode)
                {
                    DRV_PIC32MZW_ProcessHostRsp(_DRV_PIC32MZW_PacketToBuf((TCPIP_MAC_PACKET*)pNode));
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

    memcpy(pStats, &pic32mzMemStatistics, sizeof(WDRV_PIC32MZW_MAC_MEM_STATISTICS));

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
    uint8_t *payLoadPtr, *pktbuf;
    int pktLen = 0;
    uint8_t pkt_tos;
    IPV4_HEADER* pHdr;

    uint8_t pktoffset = 0;

    if (NULL == ptrPacket->pDSeg->next)
    {
        uint32_t* p = (uint32_t*)(ptrPacket->pDSeg->segLoad - ptrPacket->pDSeg->segLoadOffset);

        *p++ = (uint32_t)ptrPacket;  //Save the packet pointer

        payLoadPtr = (uint8_t*)p;

        if (ptrPacket->pMacLayer != ptrPacket->pDSeg->segLoad)
        {
            int moveOffset = (TCPIP_MAC_FRAME_OFFSET - ptrPacket->pDSeg->segLoadOffset) - (ptrPacket->pMacLayer - ptrPacket->pDSeg->segLoad);

            memmove(&ptrPacket->pMacLayer[moveOffset], ptrPacket->pMacLayer, ptrPacket->pDSeg->segLen);

            ptrPacket->pMacLayer        += moveOffset;
            ptrPacket->pNetLayer        += moveOffset;
            ptrPacket->pTransportLayer  += moveOffset;

            ptrPacket->pDSeg->segLoad += TCPIP_MAC_FRAME_OFFSET - ptrPacket->pDSeg->segLoadOffset;
            ptrPacket->pDSeg->segLoadOffset = TCPIP_MAC_FRAME_OFFSET;
        }

        pktLen = ptrPacket->pDSeg->segLen;
    }
    else
    {
        TCPIP_MAC_DATA_SEGMENT* pDseg;
        bool first = false;

        pktbuf = payLoadPtr = DRV_PIC32MZW_PacketMemAlloc(SHARED_PKT_MEM_BUFFER_SIZE, MEM_PRI_HPTX);

        if (NULL == payLoadPtr)
        {
            _TCPIP_PKT_ACK_FNC(ptrPacket, TCPIP_MAC_PKT_ACK_TX_OK, TCPIP_THIS_MODULE_ID);

            return TCPIP_MAC_RES_OP_ERR;
        }

        pDseg = ptrPacket->pDSeg;

        pktbuf += (ETH_ETHERNET_HDR_OFFSET + 2);

        while (NULL != pDseg)
        {
            if ((pktLen + pDseg->segLen) < SHARED_PKT_MEM_BUFFER_SIZE)
            {
                if ((NULL != pDseg->next) && (false == first))
                {
                    pktoffset = 2;
                }

                memcpy(pktbuf, pDseg->segLoad, pDseg->segLen);

                first = true;
            }
            else
            {
                _TCPIP_PKT_ACK_FNC(ptrPacket, TCPIP_MAC_PKT_ACK_TX_OK, TCPIP_THIS_MODULE_ID);
                DRV_PIC32MZW_PacketMemFree(payLoadPtr);

                return TCPIP_MAC_RES_OP_ERR;
            }

            pktbuf += pDseg->segLen;
            pktLen += pDseg->segLen;

            pDseg = pDseg->next;
        };

        _TCPIP_PKT_ACK_FNC(ptrPacket, TCPIP_MAC_PKT_ACK_TX_OK, TCPIP_THIS_MODULE_ID);

        ptrPacket = _DRV_PIC32MZW_BufToPacket(payLoadPtr);
        ptrPacket->pktFlags |= TCPIP_MAC_PKT_FLAG_TX;
    }

    // Signal list manager that this packet cannot be reused until the
    // ACK function is called.

    ptrPacket->pktFlags |= TCPIP_MAC_PKT_FLAG_QUEUED;

    pHdr = (IPV4_HEADER*)ptrPacket->pNetLayer;

    // Map the received TOS with AC's
    switch (pHdr->TypeOfService.val)
    {
        case 0x04:  // Video
        {
            pkt_tos = 160;
            break;
        }

        case 0x08:  // Voice
        {
            pkt_tos = 192;
            break;
        }

        case 0x10:  // BK
        {
            pkt_tos = 32;
            break;
        }

        default:    // BE
        {
            pkt_tos = 0;
            break;
        }
    }

#ifdef WDRV_PIC32MZW_MAC_TX_PKT_INSPECT_HOOK
    WDRV_PIC32MZW_MAC_TX_PKT_INSPECT_HOOK(ptrPacket);
#endif

    OSAL_SEM_Pend(&pic32mzwCtrlDescriptor.drvAccessSemaphore, OSAL_WAIT_FOREVER);
#ifdef WDRV_PIC32MZW_STATS_ENABLE
    pic32mzMemStatistics.pkt.tx++;
#endif
    wdrv_pic32mzw_wlan_send_packet(payLoadPtr, pktLen, pkt_tos, pktoffset);
    OSAL_SEM_Post(&pic32mzwCtrlDescriptor.drvAccessSemaphore);

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
        pic32mzMemStatistics.pkt.rx++;
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
    return TCPIP_MAC_RES_OP_ERR;
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
            pMacParams->ifPhyAddress.v[0] = g_macaddress[0];
            pMacParams->ifPhyAddress.v[1] = g_macaddress[1];
            pMacParams->ifPhyAddress.v[2] = g_macaddress[2];
            pMacParams->ifPhyAddress.v[3] = g_macaddress[3];
            pMacParams->ifPhyAddress.v[4] = g_macaddress[4];
            pMacParams->ifPhyAddress.v[5] = g_macaddress[5];

            pMacParams->processFlags = (TCPIP_MAC_PROCESS_FLAG_RX | TCPIP_MAC_PROCESS_FLAG_TX);
            pMacParams->macType = TCPIP_MAC_TYPE_WLAN;
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
    OSAL_SEM_Pend(&pic32mzwMACDescriptor.eventSemaphore, OSAL_WAIT_FOREVER);

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
    OSAL_SEM_Pend(&pic32mzwMACDescriptor.eventSemaphore, OSAL_WAIT_FOREVER);
    pic32mzwMACDescriptor.events &= ~macEvents;
    OSAL_SEM_Post(&pic32mzwMACDescriptor.eventSemaphore);

    return true;
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

    OSAL_SEM_Pend(&pic32mzwMACDescriptor.eventSemaphore, OSAL_WAIT_FOREVER);
    events = pic32mzwMACDescriptor.events;
    OSAL_SEM_Post(&pic32mzwMACDescriptor.eventSemaphore);

    return events;
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

    memcpy(pMACAddress, g_macaddress, 6);

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
            if (NULL != pCtrl->pfRegDomCB)
            {
                char regName[WDRV_PIC32MZW_REGDOMAIN_MAX_NAME_LEN+1];
                bool current;

                if (1 == pData[2])
                {
                    current = true;
                }
                else
                {
                    current = false;
                }

                memcpy(regName, &pData[3], WDRV_PIC32MZW_REGDOMAIN_MAX_NAME_LEN);
                regName[WDRV_PIC32MZW_REGDOMAIN_MAX_NAME_LEN] = '\0';

                if ((0 == pData[0]) || (0 == pData[1]))
                {
                    pCtrl->pfRegDomCB((DRV_HANDLE)pDcpt, 0, 0, false, regName);
                }
                else
                {
                    pCtrl->pfRegDomCB((DRV_HANDLE)pDcpt, pData[0], pData[1], current, regName);
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
    wdrv_pic32mzw_timer_tick_isr(0);
    IFS2bits.RFTM0IF = 0;
}

// *****************************************************************************
// *****************************************************************************
// Section: PIC32MZW MAC Driver Internal Callback Implementation
// *****************************************************************************
// *****************************************************************************

//*******************************************************************************
/*
  Function:
    static bool DRV_PIC32MZW_AllocPktCallback
    (
        TCPIP_MAC_PACKET* pktHandle,
        const void* ackParam
    )

  Summary:
    Allocated packet completion callback.

  Description:
    Packets allocated as general memory are freed when this callback is invoked.

  Precondition:
    A packet previously allocated by _DRV_PIC32MZW_AllocPkt.

  Parameters:
    pktHandle - Pointer to packet structure.
    ackParam  - Pointer to priority tracking structure.

  Returns:
    true or false indicating if operation was successful.

  Remarks:
    None.
*/

static bool DRV_PIC32MZW_AllocPktCallback
(
    TCPIP_MAC_PACKET* pktHandle,
    const void* ackParam
)
{
    if (NULL == pktHandle)
    {
        return false;
    }

#ifdef WDRV_PIC32MZW_STATS_ENABLE
    if (ackParam == (const void*)&pic32mzMemStatistics.mem)
    {
        pic32mzMemStatistics.mem.free++;
        pic32mzMemStatistics.mem.freeSize += pktHandle->pDSeg->segSize;
        ackParam = (const void*)NULL;
    }
#endif

    if (NULL != ackParam)
    {
        pktmem_priority_t *pktmem_pri;

        pktmem_pri = (pktmem_priority_t*)ackParam;

        pktmem_pri->num_allocd--;

#ifdef WDRV_PIC32MZW_STATS_ENABLE
        {
            uint32_t idx = ((uint32_t)pktmem_pri - (uint32_t)g_pktmem_pri)/sizeof(pktmem_priority_t);
            uint16_t size = (idx == MEM_PRI_CONFIG)?
                                pktHandle->pDSeg->segSize :
                                SHARED_PKT_MEM_BUFFER_SIZE;
            pic32mzMemStatistics.pri[idx].free++;
            pic32mzMemStatistics.pri[idx].freeSize += size;
        }
#endif
    }

    pktHandle->ackParam = NULL;

    _TCPIP_PKT_PacketFree(pktHandle);

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
    static TCPIP_MAC_PACKET* _DRV_PIC32MZW_AllocPkt
    (
        uint16_t pktLen,
        TCPIP_MAC_PACKET_FLAGS flags
    )

  Summary:
    Allocate packet memory.

  Description:
    Allocates memory from packet memory for general use.

  Precondition:
    TCP/IP stack must have been initialized.

  Parameters:
    pktLen - Size of memory to allocate.
    flags  - Additional packet flags.

  Returns:
    Pointer to TCPIP_MAC_PACKET packet.

  Remarks:
    A packet is normally allocated with space for an extra Ethernet frame structure
    and MAC frame offset so the actual requested packet segment size is made
    smaller to compensate. The allocated segment is then adjusted to remove
    the MAC frame offset.

    In place of the Ethernet frame structure and offset is a single pointer to
    the TCPIP_MAC_PACKET structure so we can later find the structure using the
    pointer stored just before the data segment.
*/

static TCPIP_MAC_PACKET* _DRV_PIC32MZW_AllocPkt
(
    uint16_t pktLen,
    TCPIP_MAC_PACKET_FLAGS flags
)
{
    TCPIP_MAC_PACKET *p_packet;

    p_packet = _TCPIP_PKT_ALLOC_FNC(sizeof(TCPIP_MAC_PACKET),
           (pktLen - TCPIP_SEG_MAC_HDR_OFFSET_SIZE + sizeof(uint8_t*)), flags);

    if (NULL == p_packet)
    {
        return NULL;
    }

    /* Set the ACK function to clean up after we've finished. */
    p_packet->ackFunc = DRV_PIC32MZW_AllocPktCallback;

    /* seqLoad points to the Ethernet header structure, move it back to the
       start of the segment. */
    p_packet->pDSeg->segLoad -= (TCPIP_MAC_FRAME_OFFSET);

    /* Store the packet pointer in the first four bytes of the segment. */
    *(uint32_t *)p_packet->pDSeg->segLoad = (uint32_t)p_packet;

    /* Update segment data pointer to be after stored pointer. */
    p_packet->pDSeg->segLoad += sizeof(uint8_t*);

    /* Segment load offset is now four bytes into segment. */
    p_packet->pDSeg->segLoadOffset = sizeof(uint8_t*);

    /* Segment size increases by reclaimed Ethernet MAC structure and offset
       minus the stored pointer at the start of the segment. */
    p_packet->pDSeg->segSize += (TCPIP_SEG_MAC_HDR_OFFSET_SIZE - sizeof(uint8_t*));

    /* The segment data is allocated with the packet header, so it doesn't
     need to be deallocated separately at the end. */
    p_packet->pDSeg->segFlags |= TCPIP_MAC_SEG_FLAG_STATIC;

    return p_packet;
}

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
    TCPIP_MAC_PACKET *p_packet;
    TCPIP_MAC_EVENT events;

    if (NULL == pEthMsg)
    {
        return;
    }

    p_packet = (TCPIP_MAC_PACKET *)*(uint32_t*)(pEthMsg - (hdrOffset + sizeof(uint8_t*)));

    p_packet->next = NULL;
    p_packet->pDSeg->segLen = lengthEthMsg - ETHERNET_HDR_LEN;
    p_packet->pktFlags |= TCPIP_MAC_PKT_FLAG_QUEUED;
    p_packet->tStamp = SYS_TMR_TickCountGet();

    /* If the received Packet is AMSDU packet, then the first MSDU packet is at
         offset 52 and second MSDU is at offset 24. Normal packets have offset 22. */
    if((52 == hdrOffset) || (24 == hdrOffset))
    {
        memcpy(p_packet->pDSeg->segLoad+2, pEthMsg, lengthEthMsg);
        p_packet->pMacLayer = p_packet->pDSeg->segLoad + 2;
    }
    else
    {
        p_packet->pMacLayer = p_packet->pDSeg->segLoad + hdrOffset;
    }

    p_packet->pNetLayer = p_packet->pMacLayer + ETHERNET_HDR_LEN;

    /* Store packet in FIFO and signal stack that packet ready to process. */
    TCPIP_Helper_ProtectedSingleListTailAdd(&pic32mzwMACDescriptor.ethRxPktList, (SGL_LIST_NODE*)p_packet);

    /* Notify stack of received packet. */
    OSAL_SEM_Pend(&pic32mzwMACDescriptor.eventSemaphore, OSAL_WAIT_FOREVER);
    events = pic32mzwMACDescriptor.events | ~pic32mzwMACDescriptor.eventMask;
    pic32mzwMACDescriptor.events |= TCPIP_EV_RX_DONE;
    OSAL_SEM_Post(&pic32mzwMACDescriptor.eventSemaphore);

    if (0 == (events & TCPIP_EV_RX_DONE))
    {
        pic32mzwMACDescriptor.eventF(TCPIP_EV_RX_DONE, pic32mzwMACDescriptor.eventParam);
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
    TCPIP_MAC_PACKET *p_packet;

    if(size < TCPIP_SEG_MAC_HDR_OFFSET_SIZE)
    {
        size = TCPIP_SEG_MAC_HDR_OFFSET_SIZE;
    }

    p_packet = _DRV_PIC32MZW_AllocPkt(size, TCPIP_MAC_PKT_FLAG_USER);

    if (NULL == p_packet)
    {
        return NULL;
    }
#ifdef WDRV_PIC32MZW_STATS_ENABLE
    p_packet->ackParam = (void*)&pic32mzMemStatistics.mem;
    pic32mzMemStatistics.mem.alloc++;
    pic32mzMemStatistics.mem.allocSize += p_packet->pDSeg->segSize;
#endif
    return _DRV_PIC32MZW_PacketToBuf(p_packet);
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
    0

  Remarks:
    None.
*/

int8_t DRV_PIC32MZW_MemFree(void *pBufferAddr)
{
    DRV_PIC32MZW_PacketMemFree(pBufferAddr);
    return 0;
}

//*******************************************************************************
/*
  Function:
    void *DRV_PIC32MZW_PacketMemAlloc(uint16_t size, uint32_t priLevel)

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

void *DRV_PIC32MZW_PacketMemAlloc(uint16_t size, MEM_PRIORITY_LEVEL_T priLevel)
{
    TCPIP_MAC_PACKET *p_packet;

    if (MEM_PRI_HPRX == priLevel)
    {
        DBL_LIST_NODE *node;

        node = TCPIP_Helper_DoubleListHeadRemove(&pic32mzwHighPriRxQueue);

        if (NULL == node)
        {
#ifdef WDRV_PIC32MZW_STATS_ENABLE
            // Print the error mesage few times....
            if (++pic32mzMemStatistics.err.hprx > 10)
            {
                return NULL;
            }
#endif
            WDRV_DBG_ERROR_PRINT("PktMemAlloc: HPRX NULL\r\n");
            return NULL;
        }

#ifdef WDRV_PIC32MZW_STATS_ENABLE
        pic32mzMemStatistics.pri[MEM_PRI_HPRX].alloc++;
        pic32mzMemStatistics.pri[MEM_PRI_HPRX].allocSize += SHARED_PKT_MEM_BUFFER_SIZE;
 #endif

        return &((WDRV_PIC32MZW_PKT_LIST_NODE*)node)->pkt;
    }

    p_packet = _DRV_PIC32MZW_AllocPkt(size, 0);

    if (NULL == p_packet)
    {
#ifdef WDRV_PIC32MZW_STATS_ENABLE
        // Print the error mesage few times....
        if (++pic32mzMemStatistics.err.gen > 10)
        {
            return NULL;
        }
#endif

        WDRV_DBG_ERROR_PRINT("PktMemAlloc: Alloc NULL\r\n");
        return NULL;
    }

    g_pktmem_pri[priLevel].num_allocd++;

    p_packet->ackParam = &g_pktmem_pri[priLevel];

#ifdef WDRV_PIC32MZW_STATS_ENABLE
    pic32mzMemStatistics.pri[priLevel].alloc++;
    pic32mzMemStatistics.pri[priLevel].allocSize +=
            (priLevel == MEM_PRI_CONFIG)? p_packet->pDSeg->segSize:
                                          SHARED_PKT_MEM_BUFFER_SIZE;
#endif

    return _DRV_PIC32MZW_PacketToBuf(p_packet);
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
    void *pk0PktBuff = pPktBuff;

    if (NULL == pPktBuff)
    {
        return;
    }

    if (IS_KVA1(pPktBuff))
    {
        pk0PktBuff = KVA1_TO_KVA0(pPktBuff);
    }

    if ((pk0PktBuff >= (void*)&pic32mzwHighPriPktList[0]) && (pk0PktBuff < (void*)&pic32mzwHighPriPktList[NUM_HIGH_PRI_PKTS]))
    {
        int i;

        for (i=0; i<NUM_HIGH_PRI_PKTS; i++)
        {
            if (pk0PktBuff == &pic32mzwHighPriPktList[i].pkt)
            {
                TCPIP_Helper_DoubleListHeadAdd(&pic32mzwHighPriRxQueue, (DBL_LIST_NODE*)&pic32mzwHighPriPktList[i]);
#ifdef WDRV_PIC32MZW_STATS_ENABLE
                pic32mzMemStatistics.pri[MEM_PRI_HPRX].free++;
                pic32mzMemStatistics.pri[MEM_PRI_HPRX].freeSize += SHARED_PKT_MEM_BUFFER_SIZE;
#endif
                return;
            }
        }
    }
    else
    {
        TCPIP_MAC_PACKET *p_packet = _DRV_PIC32MZW_BufToPacket(pPktBuff);

        if (NULL != p_packet)
        {
            if(((p_packet->pktFlags & (TCPIP_MAC_PKT_FLAG_TX|TCPIP_MAC_PKT_FLAG_USER)) != 0) ||
                (p_packet->pktFlags == TCPIP_MAC_PKT_FLAG_CAST_DISABLED))
            {
                _TCPIP_PKT_ACK_FNC(p_packet, TCPIP_MAC_PKT_ACK_TX_OK, TCPIP_THIS_MODULE_ID);
            }
            else
            {
                if((p_packet->pktFlags & TCPIP_MAC_PKT_FLAG_QUEUED) != TCPIP_MAC_PKT_FLAG_QUEUED)
                {
                    _TCPIP_PKT_ACK_FNC(p_packet, 0, TCPIP_THIS_MODULE_ID);
                }
            }
        }
        else
        {
            WDRV_DBG_ERROR_PRINT("PktMemFree: invalid packet buffer\r\n");
        }
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
    if (NULL == pPktBuff)
    {
        return;
    }

    TCPIP_Helper_ProtectedSingleListTailAdd(&pic32mzwWIDRxQueue, (SGL_LIST_NODE*)_DRV_PIC32MZW_BufToPacket(pPktBuff));
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
    if (NULL == pPktBuff)
    {
        return;
    }

    TCPIP_Helper_ProtectedSingleListTailAdd(&pic32mzwWIDTxQueue, (SGL_LIST_NODE*)_DRV_PIC32MZW_BufToPacket(pPktBuff));
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
