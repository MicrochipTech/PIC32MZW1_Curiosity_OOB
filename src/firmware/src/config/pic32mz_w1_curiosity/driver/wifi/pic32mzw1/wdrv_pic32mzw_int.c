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
