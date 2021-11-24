/*******************************************************************************
  PIC32MZW Driver Regulatory Domain Implementation

  File Name:
    wdrv_pic32mzw_regdomain.c

  Summary:
    PIC32MZW wireless driver regulatory domain implementation.

  Description:
    This interface provides an method to query and control the current
    regulatory domain.
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
#include <string.h>

#include "wdrv_pic32mzw.h"
#include "wdrv_pic32mzw_regdomain.h"
#include "wdrv_pic32mzw_cfg.h"

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

  Remarks:
    See wdrv_pic32mzw_regdomain.h for usage information.

*/

WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_RegDomainGet
(
    DRV_HANDLE handle,
    const WDRV_PIC32MZW_REGDOMAIN_SELECT selection,
    const WDRV_PIC32MZW_REGDOMAIN_CALLBACK pfRegDomCallback
)
{
    WDRV_PIC32MZW_DCPT *const pDcpt = (WDRV_PIC32MZW_DCPT *const)handle;
    DRV_PIC32MZW_WIDCTX wids;
    uint8_t widSelVal;
    OSAL_CRITSECT_DATA_TYPE critSect;

    /* Ensure the driver handle is valid. */
    if ((DRV_HANDLE_INVALID == handle) || (NULL == pDcpt) || (NULL == pDcpt->pCtrl))
    {
        return WDRV_PIC32MZW_STATUS_INVALID_ARG;
    }

    /* Ensure the driver instance has been opened for use. */
    if ((false == pDcpt->isOpen) || (DRV_HANDLE_INVALID == pDcpt->pCtrl->handle))
    {
        return WDRV_PIC32MZW_STATUS_NOT_OPEN;
    }

    if (WDRV_PIC32MZW_REGDOMAIN_SELECT_CURRENT == selection)
    {
        widSelVal = 1;
    }
    else if (WDRV_PIC32MZW_REGDOMAIN_SELECT_ALL == selection)
    {
        widSelVal = 0;
    }
    else
    {
        return WDRV_PIC32MZW_STATUS_INVALID_ARG;
    }

    /* Allocate memory for the WIDs. */
    DRV_PIC32MZW_MultiWIDInit(&wids, 32);

    DRV_PIC32MZW_MultiWIDAddData(&wids, DRV_WIFI_WID_REG_DOMAIN_INFO, &widSelVal, 1);

    critSect = OSAL_CRIT_Enter(OSAL_CRIT_TYPE_LOW);

    /* Write the wids. */
    if (false == DRV_PIC32MZW_MultiWid_Write(&wids))
    {
        OSAL_CRIT_Leave(OSAL_CRIT_TYPE_LOW, critSect);
        return WDRV_PIC32MZW_STATUS_REQUEST_ERROR;
    }

    pDcpt->pCtrl->pfRegDomCB = pfRegDomCallback;

    OSAL_CRIT_Leave(OSAL_CRIT_TYPE_LOW, critSect);

    return WDRV_PIC32MZW_STATUS_OK;
}

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

  Remarks:
    See wdrv_pic32mzw_regdomain.h for usage information.

*/

WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_RegDomainSet
(
    DRV_HANDLE handle,
    const char *pRegDomain,
    const WDRV_PIC32MZW_REGDOMAIN_CALLBACK pfRegDomCallback
)
{
    WDRV_PIC32MZW_DCPT *const pDcpt = (WDRV_PIC32MZW_DCPT *const)handle;
    DRV_PIC32MZW_WIDCTX wids;
    char widRegName[WDRV_PIC32MZW_REGDOMAIN_MAX_NAME_LEN+1];
    OSAL_CRITSECT_DATA_TYPE critSect;

    /* Ensure the driver handle and regulatory string pointer are valid. */
    if ((DRV_HANDLE_INVALID == handle) || (NULL == pDcpt) || (NULL == pDcpt->pCtrl) || (NULL == pRegDomain))
    {
        return WDRV_PIC32MZW_STATUS_INVALID_ARG;
    }

    /* Ensure the new name is within bounds for length. */
    if (strlen(pRegDomain) > WDRV_PIC32MZW_REGDOMAIN_MAX_NAME_LEN)
    {
        return WDRV_PIC32MZW_STATUS_INVALID_ARG;
    }

    /* Ensure the driver instance has been opened for use. */
    if ((false == pDcpt->isOpen) || (DRV_HANDLE_INVALID == pDcpt->pCtrl->handle))
    {
        return WDRV_PIC32MZW_STATUS_NOT_OPEN;
    }

    /* Copy region domain string to buffer, zero pad to full length */
    strncpy(widRegName, pRegDomain, sizeof(widRegName));

    /* Allocate memory for the WIDs. */
    DRV_PIC32MZW_MultiWIDInit(&wids, 32);

    DRV_PIC32MZW_MultiWIDAddData(&wids, DRV_WIFI_WID_REG_DOMAIN, (uint8_t*)widRegName, WDRV_PIC32MZW_REGDOMAIN_MAX_NAME_LEN);

    critSect = OSAL_CRIT_Enter(OSAL_CRIT_TYPE_LOW);

    /* Write the wids. */
    if (false == DRV_PIC32MZW_MultiWid_Write(&wids))
    {
        OSAL_CRIT_Leave(OSAL_CRIT_TYPE_LOW, critSect);
        return WDRV_PIC32MZW_STATUS_REQUEST_ERROR;
    }

    pDcpt->pCtrl->pfRegDomCB = pfRegDomCallback;

    OSAL_CRIT_Leave(OSAL_CRIT_TYPE_LOW, critSect);

    return WDRV_PIC32MZW_STATUS_OK;
}
