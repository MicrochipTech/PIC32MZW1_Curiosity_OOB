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
    if (false == DRV_PIC32MZW_MultiWIDWrite(&wids))
    {
        OSAL_CRIT_Leave(OSAL_CRIT_TYPE_LOW, critSect);

        DRV_PIC32MZW_MultiWIDDestroy(&wids);

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
    if (false == DRV_PIC32MZW_MultiWIDWrite(&wids))
    {
        OSAL_CRIT_Leave(OSAL_CRIT_TYPE_LOW, critSect);

        DRV_PIC32MZW_MultiWIDDestroy(&wids);

        return WDRV_PIC32MZW_STATUS_REQUEST_ERROR;
    }

    pDcpt->pCtrl->pfRegDomCB = pfRegDomCallback;

    OSAL_CRIT_Leave(OSAL_CRIT_TYPE_LOW, critSect);

    return WDRV_PIC32MZW_STATUS_OK;
}
