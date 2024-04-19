/*******************************************************************************
  PIC32MZW Driver Regulatory Domain Header File

  Company:
    Microchip Technology Inc.

  File Name:
    wdrv_pic32mzw_regdomain.h

  Summary:
    PIC32MZW wireless driver regulatory domain header file.

  Description:
    This interface provides an method to query and control the current
    regulatory domain.
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

#ifndef _WDRV_PIC32MZW_REGDOMAIN_H
#define _WDRV_PIC32MZW_REGDOMAIN_H

// *****************************************************************************
// *****************************************************************************
// Section: File includes
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>

#include "wdrv_pic32mzw_common.h"

// *****************************************************************************
// *****************************************************************************
// Section: PIC32MZW Driver Regulatory Domain Data Types
// *****************************************************************************
// *****************************************************************************

/* Maximum size of regulatory strings. */
#define WDRV_PIC32MZW_REGDOMAIN_MAX_NAME_LEN    DRV_PIC32MZW_REGDOMAIN_MAX_NAME_LEN

// *****************************************************************************
/*  Regulatory Domain Information

  Summary:
    Defines a regulatory domain's name and version.

  Description:
    Specifies the country code length, country code associated with the
    regulatory domain, supported channels, major and minor values of
    RF PHY version.

  Remarks:
    None.
*/

typedef struct
{
    uint8_t regDomainLen;
    uint8_t regDomain[WDRV_PIC32MZW_REGDOMAIN_MAX_NAME_LEN];
    WDRV_PIC32MZW_CHANNEL24_MASK channelMask;
    struct
    {
        uint16_t major;
        uint16_t minor;
    } version;
} WDRV_PIC32MZW_REGDOMAIN_INFO;

// *****************************************************************************
/* Regulatory Domain Callback Function Pointer

  Summary:
    Pointer to a regulatory domain callback.

  Description:
    This defines a function pointer to a regulatory domain callback which will
      receive information about a single domain.

  Parameters:
    handle      - Client handle obtained by a call to WDRV_PIC32MZW_Open.
    index       - Index with a grouping of regulatory domains.
    ofTotal     - Total number of domain within a grouping.
    isCurrent   - Is this domain the currently active one.
    pRegDomInfo - Pointer to WDRV_PIC32MZW_REGDOMAIN_INFO of regulatory domain.

  Returns:
    None.

  Remarks:
    When called in response to WDRV_PIC32MZW_RegDomainGet this callback
      will receive a grouping of 'ofTotal' domain records. An empty grouping is
      indicated by pRegDomain being blank (empty or NULL) and ofTotal being zero.

    When called in response to WDRV_PIC32MZW_RegDomainSet this callback
      will receive either a single group of the request domain as confirmation of
      application or ofTotal will be zero.
*/

typedef void (*WDRV_PIC32MZW_REGDOMAIN_CALLBACK)
(
    DRV_HANDLE handle,
    uint8_t index,
    uint8_t ofTotal,
    bool isCurrent,
    const WDRV_PIC32MZW_REGDOMAIN_INFO *const pRegDomInfo
);

// *****************************************************************************
/*  Regulatory Domain Selection

  Summary:
    Defines possible selections of regulatory domains.

  Description:
    Specifies a grouping of regulatory domains.

  Remarks:
    None.
*/

typedef enum
{
    WDRV_PIC32MZW_REGDOMAIN_SELECT_NONE,
    WDRV_PIC32MZW_REGDOMAIN_SELECT_CURRENT,
    WDRV_PIC32MZW_REGDOMAIN_SELECT_ALL
} WDRV_PIC32MZW_REGDOMAIN_SELECT;

// *****************************************************************************
// *****************************************************************************
// Section: PIC32MZW Driver Regulatory Domain Routines
// *****************************************************************************
// *****************************************************************************

//*******************************************************************************
/*
  Function:
    WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_RegDomainGet
    (
        DRV_HANDLE handle,
        const WDRV_PIC32MZW_REGDOMAIN_SELECT selection,
        const WDRV_PIC32MZW_REGDOMAIN_CALLBACK pfRegDomCallback
    )

  Summary:
    Requests information about regulatory domains.

  Description:
    Requests either the currently active regulatory domain or all possible
      regulatory domains.

  Precondition:
    WDRV_PIC32MZW_Initialize should have been called.
    WDRV_PIC32MZW_Open should have been called to obtain a valid handle.

  Parameters:
    handle           - Client handle obtained by a call to WDRV_PIC32MZW_Open.
    selection        - Type of regulatory domain information to retrieve.
    pfRegDomCallback - Pointer to callback function to receive the information.

  Returns:
    WDRV_PIC32MZW_STATUS_OK              - The request has been accepted.
    WDRV_PIC32MZW_STATUS_NOT_OPEN        - The driver instance is not open.
    WDRV_PIC32MZW_STATUS_INVALID_ARG     - The parameters were incorrect.
    WDRV_PIC32MZW_STATUS_REQUEST_ERROR   - The request to the PIC32MZW was rejected.

  Remarks:
    None.

*/

WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_RegDomainGet
(
    DRV_HANDLE handle,
    const WDRV_PIC32MZW_REGDOMAIN_SELECT selection,
    const WDRV_PIC32MZW_REGDOMAIN_CALLBACK pfRegDomCallback
);

//*******************************************************************************
/*
  Function:
    WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_RegDomainSet
    (
        DRV_HANDLE handle,
        const char *pRegDomain,
        const WDRV_PIC32MZW_REGDOMAIN_CALLBACK pfRegDomCallback
    )

  Summary:
    Attempts to set the current regulatory domain.

  Description:
    Requests that the current regulatory domain is changed to the one specified.

  Precondition:
    WDRV_PIC32MZW_Initialize should have been called.
    WDRV_PIC32MZW_Open should have been called to obtain a valid handle.

  Parameters:
    handle           - Client handle obtained by a call to WDRV_PIC32MZW_Open.
    pRegDomain       - Pointer to a null-terminated string which should match
                       the name of one of the domains defined in device flash
                       memory.
                       The length of the string can be maximum 6 characters.
    pfRegDomCallback - Pointer to callback function to receive confirmation.

  Returns:
    WDRV_PIC32MZW_STATUS_OK              - The request has been accepted.
    WDRV_PIC32MZW_STATUS_NOT_OPEN        - The driver instance is not open.
    WDRV_PIC32MZW_STATUS_INVALID_ARG     - The parameters were incorrect.
    WDRV_PIC32MZW_STATUS_REQUEST_ERROR   - The request to the PIC32MZW was rejected.

  Remarks:
    If the requested regulatory domain is not found in the device flash, then
    the domain remains unchanged (even though the return value is STATUS_OK).
    See also WDRV_PIC32MZW_RegDomainGet.

*/

WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_RegDomainSet
(
    DRV_HANDLE handle,
    const char *pRegDomain,
    const WDRV_PIC32MZW_REGDOMAIN_CALLBACK pfRegDomCallback
);

#endif /* _WDRV_PIC32MZW_REGDOMAIN_H */
