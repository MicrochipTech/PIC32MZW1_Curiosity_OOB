/*******************************************************************************
  PIC32MZW Driver Client API Header File

  Company:
    Microchip Technology Inc.

  File Name:
    wdrv_pic32mzw_client_api.h

  Summary:
    PIC32MZW wireless driver client API header file.

  Description:
    This file pulls together the elements which make up the client API
      assoc       - Current association.
      bssfind     - BSS scan functionality.
      softap      - Soft-AP mode.
      sta         - Infrastructure stations mode.

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

#ifndef _WDRV_PIC32MZW_CLIENT_API_H
#define _WDRV_PIC32MZW_CLIENT_API_H

// *****************************************************************************
// *****************************************************************************
// Section: File includes
// *****************************************************************************
// *****************************************************************************

#include "configuration.h"

#include "wdrv_pic32mzw.h"
#include "wdrv_pic32mzw_bssfind.h"
#include "wdrv_pic32mzw_assoc.h"
#include "wdrv_pic32mzw_softap.h"
#include "wdrv_pic32mzw_sta.h"
#include "wdrv_pic32mzw_regdomain.h"
#include "wdrv_pic32mzw_common.h"

#endif /* _WDRV_PIC32MZW_CLIENT_API_H */
