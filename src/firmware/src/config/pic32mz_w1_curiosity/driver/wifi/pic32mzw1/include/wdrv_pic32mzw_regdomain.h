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
/*******************************************************************************
Copyright (C) 2020 released Microchip Technology Inc. All rights reserved.

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
    regulatory domain, major and minor values of RF PHY version.

  Remarks:
    None.
*/

typedef struct
{
    uint8_t regDomainLen;
    uint8_t regDomain[WDRV_PIC32MZW_REGDOMAIN_MAX_NAME_LEN];
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
    pRegDomain       - Pointer to a string name of the regulatory domain.
    pfRegDomCallback - Pointer to callback function to receive confirmation.

  Returns:
    WDRV_PIC32MZW_STATUS_OK              - The request has been accepted.
    WDRV_PIC32MZW_STATUS_NOT_OPEN        - The driver instance is not open.
    WDRV_PIC32MZW_STATUS_INVALID_ARG     - The parameters were incorrect.
    WDRV_PIC32MZW_STATUS_REQUEST_ERROR   - The request to the PIC32MZW was rejected.

  Remarks:
    None.

*/

WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_RegDomainSet
(
    DRV_HANDLE handle,
    const char *pRegDomain,
    const WDRV_PIC32MZW_REGDOMAIN_CALLBACK pfRegDomCallback
);

#endif /* _WDRV_PIC32MZW_REGDOMAIN_H */
