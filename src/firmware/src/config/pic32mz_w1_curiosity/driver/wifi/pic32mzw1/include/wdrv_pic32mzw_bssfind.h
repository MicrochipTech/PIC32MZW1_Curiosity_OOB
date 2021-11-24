/*******************************************************************************
  PIC32MZW Driver BSS Find Header File

  Company:
    Microchip Technology Inc.

  File Name:
    wdrv_pic32mzw_bssfind.h

  Summary:
    PIC32MZW wireless driver BSS scanning header file.

  Description:
    This interface manages the operation of searching for a BSS.

    Searching operates on the find-first/find-next principal. When a find-first
      operation is requested the PIC32MZW will scan for available BSSs on the
      channels specified. The results will be provided via callback or polling
      and iterated over using a find-next operation.

    The callback function supplied to WDRV_PIC32MZW_BSSFindFirst may return true
      if the user wishes the next scan results to be automatically requested. By
      returning true the user can avoid needing to call WDRV_PIC32MZW_BSSFindNext,
      the callback will be called for each BSS found.
 *******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
Copyright (C) 2020-21 released Microchip Technology Inc. All rights reserved.

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
// DOM-IGNORE-END

#ifndef _WDRV_PIC32MZW_BSSFIND_H
#define _WDRV_PIC32MZW_BSSFIND_H

// *****************************************************************************
// *****************************************************************************
// Section: File includes
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>

#include "wdrv_pic32mzw_common.h"
#include "wdrv_pic32mzw_authctx.h"
#include "wdrv_pic32mzw_bssctx.h"

// *****************************************************************************
// *****************************************************************************
// Section: PIC32MZW Driver BSS Find Data Types
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/*  Channel Mask (2.4GHz).

  Summary:
    List of possible channel mask elements for 2.4GHz.

  Description:
    The channel mask consists of a single bit for each discrete channel. Channels
      maybe combined to form groups of channels, some of which are defined in
      this list.

  Remarks:
    None.
*/

typedef enum
{
    /* 2.4GHz (2412 MHz) channel 1. */
    WDRV_PIC32MZW_CM_2_4G_CH1 = 0x0001,

    /* 2.4GHz (2417 MHz) channel 2. */
    WDRV_PIC32MZW_CM_2_4G_CH2 = 0x0002,

    /* 2.4GHz (2422 MHz) channel 3. */
    WDRV_PIC32MZW_CM_2_4G_CH3 = 0x0004,

    /* 2.4GHz (2427 MHz) channel 4. */
    WDRV_PIC32MZW_CM_2_4G_CH4 = 0x0008,

    /* 2.4GHz (2432 MHz) channel 5. */
    WDRV_PIC32MZW_CM_2_4G_CH5 = 0x0010,

    /* 2.4GHz (2437 MHz) channel 6. */
    WDRV_PIC32MZW_CM_2_4G_CH6 = 0x0020,

    /* 2.4GHz (2442 MHz) channel 7. */
    WDRV_PIC32MZW_CM_2_4G_CH7 = 0x0040,

    /* 2.4GHz (2447 MHz) channel 8. */
    WDRV_PIC32MZW_CM_2_4G_CH8 = 0x0080,

    /* 2.4GHz (2452 MHz) channel 9. */
    WDRV_PIC32MZW_CM_2_4G_CH9 = 0x0100,

    /* 2.4GHz (2457 MHz) channel 10. */
    WDRV_PIC32MZW_CM_2_4G_CH10 = 0x0200,

    /* 2.4GHz (2462 MHz) channel 11. */
    WDRV_PIC32MZW_CM_2_4G_CH11 = 0x0400,

    /* 2.4GHz (2467 MHz) channel 12. */
    WDRV_PIC32MZW_CM_2_4G_CH12 = 0x0800,

    /* 2.4GHz (2472 MHz) channel 13. */
    WDRV_PIC32MZW_CM_2_4G_CH13 = 0x1000,

    /* 2.4GHz (2484 MHz) channel 14. */
    WDRV_PIC32MZW_CM_2_4G_CH14 = 0x2000,

    /* 2.4GHz channels 1 through 14 */
    WDRV_PIC32MZW_CM_2_4G_ALL = 0x3fff,

    /* 2.4GHz channels 1 through 11 */
    WDRV_PIC32MZW_CM_2_4G_DEFAULT = 0x07ff,

    /* 2.4GHz channels 1 through 11 */
    WDRV_PIC32MZW_CM_2_4G_NORTH_AMERICA = 0x07ff,

    /* 2.4GHz channels 1 through 13 */
    WDRV_PIC32MZW_CM_2_4G_EUROPE = 0x1fff,

    /* 2.4GHz channels 1 through 14 */
    WDRV_PIC32MZW_CM_2_4G_ASIA = 0x3fff
} WDRV_PIC32MZW_CHANNEL24_MASK;

// *****************************************************************************
/*  Scan Matching Mode.

  Summary:
    List of possible scan matching modes.

  Description:
    The scan matching mode can be to stop on first match or match all.

  Remarks:
    None.
*/

typedef enum
{
    /* Stop scan on first match. */
    WDRV_PIC32MZW_SCAN_MATCH_MODE_STOP_ON_FIRST,

    /* Scan for all matches. */
    WDRV_PIC32MZW_SCAN_MATCH_MODE_FIND_ALL
} WDRV_PIC32MZW_SCAN_MATCH_MODE;

// *****************************************************************************
/* Security capabilities

  Summary:
    List of possible security capabilities.

  Description:
    The security capabilities that may be offered by an AP.

  Remarks:
    None.
*/

typedef enum
{
    /* AP supports WEP-40 or WEP-104, as standalone or as GTK with 11i TSN. */
    WDRV_PIC32MZW_SEC_BIT_WEP           = 0x01,

    /* AP supports TKIP as GTK (if not WEP) and PTK. */
    WDRV_PIC32MZW_SEC_BIT_WPA           = 0x02,

    /* AP includes RSNE and supports CCMP-128 as GTK (if not TKIP or WEP) and PTK. */
    WDRV_PIC32MZW_SEC_BIT_WPA2OR3       = 0x04,

    /* AP supports BIP-CMAC-128 as IGTK. */
    WDRV_PIC32MZW_SEC_BIT_MFP_CAPABLE   = 0x08,

    /* AP requires management frame protection. */
    WDRV_PIC32MZW_SEC_BIT_MFP_REQUIRED  = 0x10,

    /* AP supports 802.1x authentication. */
    WDRV_PIC32MZW_SEC_BIT_ENTERPRISE    = 0x20,

    /* AP supports PSK authentication. */
    WDRV_PIC32MZW_SEC_BIT_PSK           = 0x40,

    /* AP supports SAE authentication. */
    WDRV_PIC32MZW_SEC_BIT_SAE           = 0x80
} WDRV_PIC32MZW_SEC_MASK;

// *****************************************************************************
/*  BSS Information

  Summary:
    Structure containing information about a BSS.

  Description:
    This structure contains the BSSID and SSID of the BSS as well as the
      signal strength RSSI. The authentication type used by the BSS and the
      channel it is operating on are also provided.

  Remarks:
    None.
*/

typedef struct
{
    /* BSS context information (BSSID, SSID, channel etc). */
    WDRV_PIC32MZW_BSS_CONTEXT ctx;

    /* Signal strength RSSI of BSS. */
    int8_t rssi;

    /* Security capabilities of BSS. */
    WDRV_PIC32MZW_SEC_MASK secCapabilities;

    /* Recommended authentication type for connecting to BSS. */
    WDRV_PIC32MZW_AUTH_TYPE authTypeRecommended;
} WDRV_PIC32MZW_BSS_INFO;

// *****************************************************************************
/*  BSS Discovery Notification Callback

  Summary:
    Callback to signal discovery of a BSS.

  Description:
    When the information about a discovered BSS is requested the driver will
      use this callback to provide the BSS information to the user.

  Parameters:
    handle      - Client handle obtained by a call to WDRV_PIC32MZW_Open.
    index       - Index of BSS find results.
    ofTotal     - Total number of BSS find results.
    pBSSInfo    - Pointer to BSS information structure.

  Returns:
    If true is returned the PIC32MZW driver will automatically fetch the next BSS
    find results which will cause a later call to this callback. If false is
    returned the PIC32MZW driver will not fetch the next BSS find results, the user
    must call WDRV_PIC32MZW_BSSFindNext to retrieve them.

  Remarks:
    The callback function must return true or false. true indicates that the user
      wishes the next BSS information structure be provided. false indicates that
      the user will call WDRV_PIC32MZW_BSSFindNext to obtain the next BBS
      information structure.
*/

typedef bool (*WDRV_PIC32MZW_BSSFIND_NOTIFY_CALLBACK)
(
    DRV_HANDLE handle,
    uint8_t index,
    uint8_t ofTotal,
    WDRV_PIC32MZW_BSS_INFO *pBSSInfo
);

// *****************************************************************************
// *****************************************************************************
// Section: PIC32MZW Driver BSS Find Routines
// *****************************************************************************
// *****************************************************************************

//*******************************************************************************
/*
  Function:
    WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_BSSFindFirst
    (
        DRV_HANDLE handle,
        WDRV_PIC32MZW_CHANNEL_ID channel,
        bool active,
        const WDRV_PIC32MZW_SSID_LIST *const pSSIDList,
        const WDRV_PIC32MZW_BSSFIND_NOTIFY_CALLBACK pfNotifyCallback
    )

  Summary:
    Request a BSS scan is performed by the PIC32MZW.

  Description:
    A scan is requested on the specified channels. An optional callback can
      be provided to receive notification of the first BSS discovered.

  Precondition:
    WDRV_PIC32MZW_Initialize must have been called.
    WDRV_PIC32MZW_Open must have been called to obtain a valid handle.

  Parameters:
    handle           - Client handle obtained by a call to WDRV_PIC32MZW_Open.
    channel          - Channel to scan, maybe WDRV_PIC32MZW_CID_ANY in
                         which case all enabled channels are scanned.
    active           - Use active vs passive scanning.
    pSSIDList        - Pointer to list of SSIDs to match on.
    pfNotifyCallback - Callback to receive notification of first BSS found.

  Returns:
    WDRV_PIC32MZW_STATUS_OK               - A scan was initiated.
    WDRV_PIC32MZW_STATUS_NOT_OPEN         - The driver instance is not open.
    WDRV_PIC32MZW_STATUS_REQUEST_ERROR    - The request to the PIC32MZW was rejected.
    WDRV_PIC32MZW_STATUS_INVALID_ARG      - The parameters were incorrect.
    WDRV_PIC32MZW_STATUS_SCAN_IN_PROGRESS - A scan is already in progress.

  Remarks:
    If channel is WDRV_PIC32MZW_CID_ANY then all enabled channels are
      scanned. The enabled channels can be configured using
      WDRV_PIC32MZW_BSSFindSetEnabledChannels. How the scan is performed can
      be configured using WDRV_PIC32MZW_BSSFindSetScanParameters.

*/

WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_BSSFindFirst
(
    DRV_HANDLE handle,
    WDRV_PIC32MZW_CHANNEL_ID channel,
    bool active,
    const WDRV_PIC32MZW_SSID_LIST *const pSSIDList,
    const WDRV_PIC32MZW_BSSFIND_NOTIFY_CALLBACK pfNotifyCallback
);

//*******************************************************************************
/*
  Function:
    WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_BSSFindNext
    (
        DRV_HANDLE handle,
        WDRV_PIC32MZW_BSSFIND_NOTIFY_CALLBACK pfNotifyCallback
    )

  Summary:
    Request the next scan results be provided.

  Description:
    The information structure of the next BSS is requested from the PIC32MZW.

  Precondition:
    WDRV_PIC32MZW_Initialize must have been called.
    WDRV_PIC32MZW_Open must have been called to obtain a valid handle.
    WDRV_PIC32MZW_BSSFindFirst must have been called.

  Parameters:
    handle           - Client handle obtained by a call to WDRV_PIC32MZW_Open.
    pfNotifyCallback - Callback to receive notification of next BSS found.

  Returns:
    WDRV_PIC32MZW_STATUS_OK               - The request was accepted.
    WDRV_PIC32MZW_STATUS_NOT_OPEN         - The driver instance is not open.
    WDRV_PIC32MZW_STATUS_INVALID_ARG      - The parameters were incorrect.
    WDRV_PIC32MZW_STATUS_SCAN_IN_PROGRESS - A scan is already in progress.
    WDRV_PIC32MZW_STATUS_BSS_FIND_END     - No more results are available.

  Remarks:
    None.

*/

WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_BSSFindNext
(
    DRV_HANDLE handle,
    WDRV_PIC32MZW_BSSFIND_NOTIFY_CALLBACK pfNotifyCallback
);

//*******************************************************************************
/*
  Function:
    WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_BSSFindReset
    (
        DRV_HANDLE handle,
        WDRV_PIC32MZW_BSSFIND_NOTIFY_CALLBACK pfNotifyCallback
    )

  Summary:
    Request the first scan results again

  Description:
    The information structure of the first BSS is requested from the PIC32MZW.

  Precondition:
    WDRV_PIC32MZW_Initialize must have been called.
    WDRV_PIC32MZW_Open must have been called to obtain a valid handle.
    WDRV_PIC32MZW_BSSFindFirst must have been called.

  Parameters:
    handle           - Client handle obtained by a call to WDRV_PIC32MZW_Open.
    pfNotifyCallback - Callback to receive notification of next BSS found.

  Returns:
    WDRV_PIC32MZW_STATUS_OK               - The request was accepted.
    WDRV_PIC32MZW_STATUS_NOT_OPEN         - The driver instance is not open.
    WDRV_PIC32MZW_STATUS_INVALID_ARG      - The parameters were incorrect.
    WDRV_PIC32MZW_STATUS_SCAN_IN_PROGRESS - A scan is already in progress.
    WDRV_PIC32MZW_STATUS_BSS_FIND_END     - No more results are available.

  Remarks:
    None.

*/

WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_BSSFindReset
(
    DRV_HANDLE handle,
    WDRV_PIC32MZW_BSSFIND_NOTIFY_CALLBACK pfNotifyCallback
);

//*******************************************************************************
/*
  Function:
    WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_BSSFindGetInfo
    (
        DRV_HANDLE handle,
        WDRV_PIC32MZW_BSS_INFO *const pBSSInfo
    )

  Summary:
    Requests the information structure of the current BSS scan result.

  Description:
    After each call to either WDRV_PIC32MZW_BSSFindFirst or WDRV_PIC32MZW_BSSFindNext
      the driver receives a single BSS information structure which it stores.
      This function retrieves that structure.

  Precondition:
    WDRV_PIC32MZW_Initialize must have been called.
    WDRV_PIC32MZW_Open must have been called to obtain a valid handle.
    WDRV_PIC32MZW_BSSFindFirst must have been called.

  Parameters:
    handle   - Client handle obtained by a call to WDRV_PIC32MZW_Open.
    pBSSInfo - Pointer to structure to populate with the current BSS information.

  Returns:
    WDRV_PIC32MZW_STATUS_OK          - The request was accepted.
    WDRV_PIC32MZW_STATUS_NOT_OPEN    - The driver instance is not open.
    WDRV_PIC32MZW_STATUS_INVALID_ARG - The parameters were incorrect.
    WDRV_PIC32MZW_STATUS_NO_BSS_INFO - There is no current BBS information available.

  Remarks:
    This function may be polled after calling WDRV_PIC32MZW_BSSFindFirst or
      WDRV_PIC32MZW_BSSFindNext until it returns WDRV_PIC32MZW_STATUS_OK.

*/

WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_BSSFindGetInfo
(
    DRV_HANDLE handle,
    WDRV_PIC32MZW_BSS_INFO *const pBSSInfo
);

//*******************************************************************************
/*
  Function:
    WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_BSSFindSetScanParameters
    (
        DRV_HANDLE handle,
        uint8_t numSlots,
        uint16_t activeSlotTime,
        uint16_t passiveSlotTime,
        uint8_t numProbes
    )

  Summary:
    Configures the scan operation.

  Description:
    Configures the time periods of active and passive scanning operations.

  Precondition:
    WDRV_PIC32MZW_Initialize must have been called.
    WDRV_PIC32MZW_Open must have been called to obtain a valid handle.

  Parameters:
    handle            - Client handle obtained by a call to WDRV_PIC32MZW_Open.
    numSlots          - Number of slots (minimum is 2).
    activeSlotTime    - Time spent on each active channel probing for BSS's.
    passiveSlotTime   - Time spent on each passive channel listening for beacons.
    numProbes         - Number of probes per slot.

  Returns:
    WDRV_PIC32MZW_STATUS_OK            - The request was accepted.
    WDRV_PIC32MZW_STATUS_NOT_OPEN      - The driver instance is not open.
    WDRV_PIC32MZW_STATUS_INVALID_ARG   - The parameters were incorrect.

  Remarks:
    If any parameter is zero then the configured value is unchanged.

*/

WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_BSSFindSetScanParameters
(
    DRV_HANDLE handle,
    uint8_t numSlots,
    uint16_t activeSlotTime,
    uint16_t passiveSlotTime,
    uint8_t numProbes
);

//*******************************************************************************
/*
  Function:
    WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_BSSFindSetEnabledChannels24
    (
        DRV_HANDLE handle,
        WDRV_PIC32MZW_CHANNEL24_MASK channelMask24
    )

  Summary:
    Set the enabled channels list for 2.4GHz.

  Description:
    To comply with regulatory domains certain channels must not be scanned.
      This function configures which channels are enabled to be used.

  Precondition:
    WDRV_PIC32MZW_Initialize must have been called.
    WDRV_PIC32MZW_Open must have been called to obtain a valid handle.

  Parameters:
    handle        - Client handle obtained by a call to WDRV_PIC32MZW_Open.
    channelMask24 - A 2.4GHz channel mask detailing all the enabled channels.

  Returns:
    WDRV_PIC32MZW_STATUS_OK             - The request was accepted.
    WDRV_PIC32MZW_STATUS_NOT_OPEN       - The driver instance is not open.
    WDRV_PIC32MZW_STATUS_INVALID_ARG    - The parameters were incorrect.
    WDRV_PIC32MZW_STATUS_REQUEST_ERROR  - The PIC32MZW was unable to accept this request.

  Remarks:
    None.

*/

WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_BSSFindSetEnabledChannels24
(
    DRV_HANDLE handle,
    WDRV_PIC32MZW_CHANNEL24_MASK channelMask24
);

//*******************************************************************************
/*
  Function:
    WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_BSSFindSetScanMatchMode
    (
        DRV_HANDLE handle,
        WDRV_PIC32MZW_SCAN_MATCH_MODE matchMode
    )

  Summary:
    Configures the scan matching mode.

  Description:
    This function configures the matching mode, either stop on first or
      match all, used when scanning for SSIDs.

  Precondition:
    WDRV_PIC32MZW_Initialize must have been called.
    WDRV_PIC32MZW_Open must have been called to obtain a valid handle.

  Parameters:
    handle    - Client handle obtained by a call to WDRV_PIC32MZW_Open.
    matchMode - Required scan matching mode.

  Returns:
    WDRV_PIC32MZW_STATUS_OK            - The request was accepted.
    WDRV_PIC32MZW_STATUS_NOT_OPEN      - The driver instance is not open.
    WDRV_PIC32MZW_STATUS_INVALID_ARG   - The parameters were incorrect.
    WDRV_PIC32MZW_STATUS_REQUEST_ERROR - The PIC32MZW was unable to accept this request.

  Remarks:
    None.

*/

WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_BSSFindSetScanMatchMode
(
    DRV_HANDLE handle,
    WDRV_PIC32MZW_SCAN_MATCH_MODE matchMode
);

//*******************************************************************************
/*
  Function:
    uint8_t WDRV_PIC32MZW_BSSFindGetNumBSSResults(DRV_HANDLE handle)

  Summary:
    Returns the number of BSS scan results found.

  Description:
    Returns the number of BSS scan results found.

  Precondition:
    WDRV_PIC32MZW_Initialize must have been called.
    WDRV_PIC32MZW_Open must have been called to obtain a valid handle.
    WDRV_PIC32MZW_BSSFindFirst must have been called to start a scan.

  Parameters:
    handle - Client handle obtained by a call to WDRV_PIC32MZW_Open.

  Returns:
    Number of BSS scan results available. Zero indicates no results or an
      error occurred.

  Remarks:
    None.

*/

uint8_t WDRV_PIC32MZW_BSSFindGetNumBSSResults(DRV_HANDLE handle);

//*******************************************************************************
/*
  Function:
    bool WDRV_PIC32MZW_BSSFindInProgress(DRV_HANDLE handle)

  Summary:
    Indicates if a BSS scan is in progress.

  Description:
    Returns a flag indicating if a BSS scan operation is currently running.

  Precondition:
    WDRV_PIC32MZW_Initialize must have been called.
    WDRV_PIC32MZW_Open must have been called to obtain a valid handle.

  Parameters:
    handle - Client handle obtained by a call to WDRV_PIC32MZW_Open.

  Returns:
    Flag indicating if a scan is in progress. If an error occurs the result
      is false.

  Remarks:
    None.

*/

bool WDRV_PIC32MZW_BSSFindInProgress(DRV_HANDLE handle);

#endif /* _WDRV_PIC32MZW_BSSFIND_H */
