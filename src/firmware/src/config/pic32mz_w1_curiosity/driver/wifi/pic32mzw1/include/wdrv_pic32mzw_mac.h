/*******************************************************************************
  PIC32MZW MAC Driver TCP/IP Interface Header File

  Company:
    Microchip Technology Inc.

  File Name:
    wdrv_pic32mzw_mac.h

  Summary:
    PIC32MZW wireless driver MAC interface for TCP/IP stack.

  Description:
    Provides a control API for the MAC interface driver.
 *******************************************************************************/

// DOM-IGNORE-BEGIN
/*
Copyright (C) 2020-2023, Microchip Technology Inc., and its subsidiaries. All rights reserved.

The software and documentation is provided by microchip and its contributors
"as is" and any express, implied or statutory warranties, including, but not
limited to, the implied warranties of merchantability, fitness for a particular
purpose and non-infringement of third party intellectual property rights are
disclaimed to the fullest extent permitted by law. In no event shall microchip
or its contributors be liable for any direct, indirect, incidental, special,
exemplary, or consequential damages (including, but not limited to, procurement
of substitute goods or services; loss of use, data, or profits; or business
interruption) however caused and on any theory of liability, whether in contract,
strict liability, or tort (including negligence or otherwise) arising in any way
out of the use of the software and documentation, even if advised of the
possibility of such damage.

Except as expressly permitted hereunder and subject to the applicable license terms
for any third-party software incorporated in the software and any applicable open
source software license terms, no license or other rights, whether express or
implied, are granted under any patent or other intellectual property rights of
Microchip or any third party.
*/
// DOM-IGNORE-END

#ifndef _WDRV_PIC32MZW_MAC_H
#define _WDRV_PIC32MZW_MAC_H

// *****************************************************************************
// *****************************************************************************
// Section: File includes
// *****************************************************************************
// *****************************************************************************

#include "wdrv_pic32mzw.h"

// *****************************************************************************
// *****************************************************************************
// Section: PIC32MZW MAC Driver Routines
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

  Precondition:
    WDRV_PIC32MZW_Initialize should have been called.
    WDRV_PIC32MZW_Open should have been called to obtain a valid handle.

  Parameters:
    handle  - Client handle obtained by a call to WDRV_PIC32MZW_Open.

  Returns:
    Flag indicating network active state (true/false).

  Remarks:
    None.

*/

bool WDRV_PIC32MZW_MACLinkCheck(DRV_HANDLE handle);

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

  Precondition:
    WDRV_PIC32MZW_Initialize should have been called.
    WDRV_PIC32MZW_Open should have been called to obtain a valid handle.

  Parameters:
    handle        - Client handle obtained by a call to WDRV_PIC32MZW_Open.
    DestMACAddr - Pointer to new MAC address.

  Returns:
    TCPIP_MAC_RES_OK     - Operation performed.
    TCPIP_MAC_RES_OP_ERR - An error occurred.

  Remarks:
    None.

*/

TCPIP_MAC_RES WDRV_PIC32MZW_MACRxFilterHashTableEntrySet
(
    DRV_HANDLE handle,
    const TCPIP_MAC_ADDR* DestMACAddr
);

//*******************************************************************************
/*
  Function:
    bool WDRV_PIC32MZW_MACPowerMode(DRV_HANDLE handle, TCPIP_MAC_POWER_MODE pwrMode)

  Summary:
    Change the power mode.

  Description:
    Not currently supported.

  Precondition:
    WDRV_PIC32MZW_Initialize should have been called.
    WDRV_PIC32MZW_Open should have been called to obtain a valid handle.

  Parameters:
    handle    - Client handle obtained by a call to WDRV_PIC32MZW_Open.
    pwrMode - New power mode.

  Returns:
    false - Not supported.

  Remarks:
    None.

*/

bool WDRV_PIC32MZW_MACPowerMode(DRV_HANDLE handle, TCPIP_MAC_POWER_MODE pwrMode);

//*******************************************************************************
/*
  Function:
    TCPIP_MAC_RES WDRV_PIC32MZW_MACPacketTx(DRV_HANDLE handle, TCPIP_MAC_PACKET* ptrPacket)

  Summary:
    Send an Ethernet frame via the PIC32MZW.

  Description:
    Takes an Ethernet frame from the TCP/IP stack and schedules it with the
      PIC32MZW.

  Precondition:
    WDRV_PIC32MZW_Initialize should have been called.
    WDRV_PIC32MZW_Open should have been called to obtain a valid handle.

  Parameters:
    handle      - Client handle obtained by a call to WDRV_PIC32MZW_Open.
    ptrPacket - Pointer to Ethernet frame to send.

  Returns:
    None.

  Remarks:
    None.

*/

TCPIP_MAC_RES WDRV_PIC32MZW_MACPacketTx(DRV_HANDLE handle, TCPIP_MAC_PACKET* ptrPacket);

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

  Precondition:
    WDRV_PIC32MZW_Initialize should have been called.
    WDRV_PIC32MZW_Open should have been called to obtain a valid handle.

  Parameters:
    handle    - Client handle obtained by a call to WDRV_PIC32MZW_Open.
    pRes      -
    pPktStat  -

  Returns:
    Pointer to next received Ethernet frame.

  Remarks:
    None.

*/

TCPIP_MAC_PACKET* WDRV_PIC32MZW_MACPacketRx
(
    DRV_HANDLE handle,
    TCPIP_MAC_RES* pRes,
    TCPIP_MAC_PACKET_RX_STAT* pPktStat
);

//*******************************************************************************
/*
  Function:
    TCPIP_MAC_RES WDRV_PIC32MZW_MACProcess(DRV_HANDLE handle)

  Summary:
    Regular update to MAC state machine.

  Description:
    Called by the TCP/IP to update the internal state machine.

  Precondition:
    WDRV_PIC32MZW_Initialize should have been called.
    WDRV_PIC32MZW_Open should have been called to obtain a valid handle.

  Parameters:
    handle - Client handle obtained by a call to WDRV_PIC32MZW_Open.

  Returns:
    None.

  Remarks:
    No currently used.

*/

TCPIP_MAC_RES WDRV_PIC32MZW_MACProcess(DRV_HANDLE handle);

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

  Precondition:
    WDRV_PIC32MZW_Initialize should have been called.
    WDRV_PIC32MZW_Open should have been called to obtain a valid handle.

  Parameters:
    handle          - Client handle obtained by a call to WDRV_PIC32MZW_Open.
    pRxStatistics - Pointer to receive statistics structure.
    pTxStatistics - Pointer to transmit statistics structure.

  Returns:
    TCPIP_MAC_RES_OP_ERR - Not supported.

  Remarks:
    None.

*/

TCPIP_MAC_RES WDRV_PIC32MZW_MACStatisticsGet
(
    DRV_HANDLE handle,
    TCPIP_MAC_RX_STATISTICS* pRxStatistics,
    TCPIP_MAC_TX_STATISTICS* pTxStatistics
);

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

  Precondition:
    WDRV_PIC32MZW_Initialize should have been called.
    WDRV_PIC32MZW_Open should have been called to obtain a valid handle.

  Parameters:
    handle       - Client handle obtained by a call to WDRV_PIC32MZW_Open.
    pMacParams - Pointer to structure to fill.

  Returns:
    TCPIP_MAC_RES_IS_BUSY - Data is unavailable.
    TCPIP_MAC_RES_OK      - Data is available.

  Remarks:
    None.

*/

TCPIP_MAC_RES WDRV_PIC32MZW_MACParametersGet
(
    DRV_HANDLE handle,
    TCPIP_MAC_PARAMETERS* pMacParams
);

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

  Precondition:
    WDRV_PIC32MZW_Initialize should have been called.
    WDRV_PIC32MZW_Open should have been called to obtain a valid handle.

  Parameters:
    handle        - Client handle obtained by a call to WDRV_PIC32MZW_Open.
    pRegEntries -
    nEntries    -
    pHwEntries  -

  Returns:
    None.

  Remarks:
    None.

*/

TCPIP_MAC_RES WDRV_PIC32MZW_MACRegisterStatisticsGet
(
    DRV_HANDLE handle,
    TCPIP_MAC_STATISTICS_REG_ENTRY* pRegEntries,
    int nEntries,
    int* pHwEntries
);

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

  Precondition:
    None.

  Parameters:
    modId       -
    configBuff  -
    buffSize    -
    pConfigSize -

  Returns:
    0

  Remarks:
    None.

*/

size_t WDRV_PIC32MZW_MACConfigGet
(
    DRV_HANDLE handle,
    void* configBuff,
    size_t buffSize,
    size_t* pConfigSize
);

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

  Precondition:
    WDRV_PIC32MZW_Initialize should have been called.
    WDRV_PIC32MZW_Open should have been called to obtain a valid handle.

  Parameters:
    handle      - Client handle obtained by a call to WDRV_PIC32MZW_Open.
    macEvents - Mask of events to be changed.
    enable    - Flag indicating if the events are added or removed.

  Returns:
    Flag indicating success or failure.

  Remarks:
    None.

*/

bool WDRV_PIC32MZW_MACEventMaskSet
(
    DRV_HANDLE handle,
    TCPIP_MAC_EVENT macEvents,
    bool enable
);

//*******************************************************************************
/*
  Function:
    bool WDRV_PIC32MZW_MACEventAcknowledge(DRV_HANDLE handle, TCPIP_MAC_EVENT macEvents)

  Summary:
    Acknowledge an event.

  Description:
    Indicates that certain events are to be acknowledged and cleared.

  Precondition:
    WDRV_PIC32MZW_Initialize should have been called.
    WDRV_PIC32MZW_Open should have been called to obtain a valid handle.

  Parameters:
    handle      - Client handle obtained by a call to WDRV_PIC32MZW_Open.
    macEvents - Mask of events to be changed.

  Returns:
    Flag indicating success or failure.

  Remarks:
    None.

*/

bool WDRV_PIC32MZW_MACEventAcknowledge(DRV_HANDLE handle, TCPIP_MAC_EVENT macEvents);

//*******************************************************************************
/*
  Function:
    TCPIP_MAC_EVENT WDRV_PIC32MZW_MACEventPendingGet(DRV_HANDLE handle)

  Summary:
    Retrieve the current events.

  Description:
    Returns the current event state.

  Precondition:
    WDRV_PIC32MZW_Initialize should have been called.
    WDRV_PIC32MZW_Open should have been called to obtain a valid handle.

  Parameters:
    handle - Client handle obtained by a call to WDRV_PIC32MZW_Open.

  Returns:
    None.

  Remarks:
    None.

*/

TCPIP_MAC_EVENT WDRV_PIC32MZW_MACEventPendingGet(DRV_HANDLE handle);

#endif /* _WDRV_PIC32MZW_MAC_H */
