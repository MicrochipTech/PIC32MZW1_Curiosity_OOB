/*******************************************************************************
  PIC32MZW Wireless Driver

  File Name:
    wdrv_pic32mzw_int.c

  Summary:
    PIC32MZW wireless driver interrupt control.

  Description:
    PIC32MZW wireless driver interrupt control.
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

#include "wdrv_pic32mzw_common.h"

// *****************************************************************************
// *****************************************************************************
// Section: PIC32MZW Driver Interrupt Control Implementation
// *****************************************************************************
// *****************************************************************************

/****************************************************************************************
 * Function:        void DRV_PIC32MZW_MACTimer0Enable(void)
 *
 * Overview:        This function enables the timer 0 interrupt
****************************************************************************************/
void DRV_PIC32MZW_MACTimer0Enable(void)
{
    SYS_INT_SourceEnable(INT_SOURCE_RFTM0);
}

/****************************************************************************************
 * Function:        void DRV_PIC32MZW_MACTimer0Disable(void)
 *
 * Overview:        This function disables the timer 0 interrupt
****************************************************************************************/
void DRV_PIC32MZW_MACTimer0Disable(void)
{
    SYS_INT_SourceDisable(INT_SOURCE_RFTM0);
}

/****************************************************************************************
 * Function:        void DRV_PIC32MZW_MACTimer1Enable(void)
 *
 * Overview:        This function enables the timer 1 interrupt
****************************************************************************************/
void DRV_PIC32MZW_MACTimer1Enable(void)
{
    SYS_INT_SourceEnable(INT_SOURCE_RFTM1);
}

/****************************************************************************************
 * Function:        void DRV_PIC32MZW_MACTimer1Disable(void)
 *
 * Overview:        This function disables the timer 1 interrupt
****************************************************************************************/
void DRV_PIC32MZW_MACTimer1Disable(void)
{
    SYS_INT_SourceDisable(INT_SOURCE_RFTM1);
}
