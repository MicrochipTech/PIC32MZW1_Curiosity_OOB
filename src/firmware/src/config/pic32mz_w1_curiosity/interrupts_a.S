/*******************************************************************************
  System Interrupts File

  Company:
    Microchip Technology Inc.

  File Name:
    interrupt_a.S

  Summary:
    Raw ISR definitions.

  Description:
    This file maps all the interrupt vectors to their corresponding
    implementations. If a particular module interrupt is used, then its ISR
    definition can be found in corresponding PLIB source file. If a module
    interrupt is not used, then its ISR implementation is mapped to dummy
    handler.
 *******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
* Copyright (C) 2019 Microchip Technology Inc. and its subsidiaries.
*
* Subject to your compliance with these terms, you may use Microchip software
* and any derivatives exclusively with Microchip products. It is your
* responsibility to comply with third party license terms applicable to your
* use of third party software (including open source software) that may
* accompany Microchip software.
*
* THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
* EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
* WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
* PARTICULAR PURPOSE.
*
* IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
* INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
* WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
* BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
* FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
* ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
* THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *******************************************************************************/
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <xc.h>
#include "ISR_Support.h"

    .extern  CORE_TIMER_Handler

    .section   .vector_0,code, keep
    .equ     __vector_dispatch_0, IntVectorCORE_TIMER_Handler
    .global  __vector_dispatch_0
    .set     nomicromips
    .set     noreorder
    .set     nomips16
    .set     noat
    .ent  IntVectorCORE_TIMER_Handler

IntVectorCORE_TIMER_Handler:
    portSAVE_CONTEXT
    la    s6,  CORE_TIMER_Handler
    jalr  s6
    nop
    portRESTORE_CONTEXT
    .end   IntVectorCORE_TIMER_Handler
    .extern  TIMER_2_Handler

    .section   .vector_9,code, keep
    .equ     __vector_dispatch_9, IntVectorTIMER_2_Handler
    .global  __vector_dispatch_9
    .set     nomicromips
    .set     noreorder
    .set     nomips16
    .set     noat
    .ent  IntVectorTIMER_2_Handler

IntVectorTIMER_2_Handler:
    portSAVE_CONTEXT
    la    s6,  TIMER_2_Handler
    jalr  s6
    nop
    portRESTORE_CONTEXT
    .end   IntVectorTIMER_2_Handler
    .extern  TIMER_3_Handler

    .section   .vector_14,code, keep
    .equ     __vector_dispatch_14, IntVectorTIMER_3_Handler
    .global  __vector_dispatch_14
    .set     nomicromips
    .set     noreorder
    .set     nomips16
    .set     noat
    .ent  IntVectorTIMER_3_Handler

IntVectorTIMER_3_Handler:
    portSAVE_CONTEXT
    la    s6,  TIMER_3_Handler
    jalr  s6
    nop
    portRESTORE_CONTEXT
    .end   IntVectorTIMER_3_Handler
    .extern  RTCC_Handler

    .section   .vector_33,code, keep
    .equ     __vector_dispatch_33, IntVectorRTCC_Handler
    .global  __vector_dispatch_33
    .set     nomicromips
    .set     noreorder
    .set     nomips16
    .set     noat
    .ent  IntVectorRTCC_Handler

IntVectorRTCC_Handler:
    portSAVE_CONTEXT
    la    s6,  RTCC_Handler
    jalr  s6
    nop
    portRESTORE_CONTEXT
    .end   IntVectorRTCC_Handler
    .extern  USB_Handler

    .section   .vector_34,code, keep
    .equ     __vector_dispatch_34, IntVectorUSB_Handler
    .global  __vector_dispatch_34
    .set     nomicromips
    .set     noreorder
    .set     nomips16
    .set     noat
    .ent  IntVectorUSB_Handler

IntVectorUSB_Handler:
    portSAVE_CONTEXT
    la    s6,  USB_Handler
    jalr  s6
    nop
    portRESTORE_CONTEXT
    .end   IntVectorUSB_Handler
    .extern  SPI1_RX_Handler

    .section   .vector_36,code, keep
    .equ     __vector_dispatch_36, IntVectorSPI1_RX_Handler
    .global  __vector_dispatch_36
    .set     nomicromips
    .set     noreorder
    .set     nomips16
    .set     noat
    .ent  IntVectorSPI1_RX_Handler

IntVectorSPI1_RX_Handler:
    portSAVE_CONTEXT
    la    s6,  SPI1_RX_Handler
    jalr  s6
    nop
    portRESTORE_CONTEXT
    .end   IntVectorSPI1_RX_Handler
    .extern  SPI1_TX_Handler

    .section   .vector_37,code, keep
    .equ     __vector_dispatch_37, IntVectorSPI1_TX_Handler
    .global  __vector_dispatch_37
    .set     nomicromips
    .set     noreorder
    .set     nomips16
    .set     noat
    .ent  IntVectorSPI1_TX_Handler

IntVectorSPI1_TX_Handler:
    portSAVE_CONTEXT
    la    s6,  SPI1_TX_Handler
    jalr  s6
    nop
    portRESTORE_CONTEXT
    .end   IntVectorSPI1_TX_Handler
    .extern  UART1_FAULT_Handler

    .section   .vector_38,code, keep
    .equ     __vector_dispatch_38, IntVectorUART1_FAULT_Handler
    .global  __vector_dispatch_38
    .set     nomicromips
    .set     noreorder
    .set     nomips16
    .set     noat
    .ent  IntVectorUART1_FAULT_Handler

IntVectorUART1_FAULT_Handler:
    portSAVE_CONTEXT
    la    s6,  UART1_FAULT_Handler
    jalr  s6
    nop
    portRESTORE_CONTEXT
    .end   IntVectorUART1_FAULT_Handler
    .extern  UART1_RX_Handler

    .section   .vector_39,code, keep
    .equ     __vector_dispatch_39, IntVectorUART1_RX_Handler
    .global  __vector_dispatch_39
    .set     nomicromips
    .set     noreorder
    .set     nomips16
    .set     noat
    .ent  IntVectorUART1_RX_Handler

IntVectorUART1_RX_Handler:
    portSAVE_CONTEXT
    la    s6,  UART1_RX_Handler
    jalr  s6
    nop
    portRESTORE_CONTEXT
    .end   IntVectorUART1_RX_Handler
    .extern  UART1_TX_Handler

    .section   .vector_40,code, keep
    .equ     __vector_dispatch_40, IntVectorUART1_TX_Handler
    .global  __vector_dispatch_40
    .set     nomicromips
    .set     noreorder
    .set     nomips16
    .set     noat
    .ent  IntVectorUART1_TX_Handler

IntVectorUART1_TX_Handler:
    portSAVE_CONTEXT
    la    s6,  UART1_TX_Handler
    jalr  s6
    nop
    portRESTORE_CONTEXT
    .end   IntVectorUART1_TX_Handler
    .extern  I2C2_BUS_Handler

    .section   .vector_59,code, keep
    .equ     __vector_dispatch_59, IntVectorI2C2_BUS_Handler
    .global  __vector_dispatch_59
    .set     nomicromips
    .set     noreorder
    .set     nomips16
    .set     noat
    .ent  IntVectorI2C2_BUS_Handler

IntVectorI2C2_BUS_Handler:
    portSAVE_CONTEXT
    la    s6,  I2C2_BUS_Handler
    jalr  s6
    nop
    portRESTORE_CONTEXT
    .end   IntVectorI2C2_BUS_Handler
    .extern  I2C2_MASTER_Handler

    .section   .vector_61,code, keep
    .equ     __vector_dispatch_61, IntVectorI2C2_MASTER_Handler
    .global  __vector_dispatch_61
    .set     nomicromips
    .set     noreorder
    .set     nomips16
    .set     noat
    .ent  IntVectorI2C2_MASTER_Handler

IntVectorI2C2_MASTER_Handler:
    portSAVE_CONTEXT
    la    s6,  I2C2_MASTER_Handler
    jalr  s6
    nop
    portRESTORE_CONTEXT
    .end   IntVectorI2C2_MASTER_Handler
    .extern  RFSMC_Handler

    .section   .vector_83,code, keep
    .equ     __vector_dispatch_83, IntVectorRFSMC_Handler
    .global  __vector_dispatch_83
    .set     nomicromips
    .set     noreorder
    .set     nomips16
    .set     noat
    .ent  IntVectorRFSMC_Handler

IntVectorRFSMC_Handler:
    portSAVE_CONTEXT
    la    s6,  RFSMC_Handler
    jalr  s6
    nop
    portRESTORE_CONTEXT
    .end   IntVectorRFSMC_Handler
    .extern  RFMAC_Handler

    .section   .vector_84,code, keep
    .equ     __vector_dispatch_84, IntVectorRFMAC_Handler
    .global  __vector_dispatch_84
    .set     nomicromips
    .set     noreorder
    .set     nomips16
    .set     noat
    .ent  IntVectorRFMAC_Handler

IntVectorRFMAC_Handler:
    portSAVE_CONTEXT
    la    s6,  RFMAC_Handler
    jalr  s6
    nop
    portRESTORE_CONTEXT
    .end   IntVectorRFMAC_Handler
    .extern  RFTM0_Handler

    .section   .vector_86,code, keep
    .equ     __vector_dispatch_86, IntVectorRFTM0_Handler
    .global  __vector_dispatch_86
    .set     nomicromips
    .set     noreorder
    .set     nomips16
    .set     noat
    .ent  IntVectorRFTM0_Handler

IntVectorRFTM0_Handler:
    portSAVE_CONTEXT
    la    s6,  RFTM0_Handler
    jalr  s6
    nop
    portRESTORE_CONTEXT
    .end   IntVectorRFTM0_Handler
    .extern  ADC_DATA15_Handler

    .section   .vector_121,code, keep
    .equ     __vector_dispatch_121, IntVectorADC_DATA15_Handler
    .global  __vector_dispatch_121
    .set     nomicromips
    .set     noreorder
    .set     nomips16
    .set     noat
    .ent  IntVectorADC_DATA15_Handler

IntVectorADC_DATA15_Handler:
    portSAVE_CONTEXT
    la    s6,  ADC_DATA15_Handler
    jalr  s6
    nop
    portRESTORE_CONTEXT
    .end   IntVectorADC_DATA15_Handler
    .extern  CRYPTO1_Handler

    .section   .vector_166,code, keep
    .equ     __vector_dispatch_166, IntVectorCRYPTO1_Handler
    .global  __vector_dispatch_166
    .set     nomicromips
    .set     noreorder
    .set     nomips16
    .set     noat
    .ent  IntVectorCRYPTO1_Handler

IntVectorCRYPTO1_Handler:
    portSAVE_CONTEXT
    la    s6,  CRYPTO1_Handler
    jalr  s6
    nop
    portRESTORE_CONTEXT
    .end   IntVectorCRYPTO1_Handler
    .extern  CRYPTO1_FAULT_Handler

    .section   .vector_167,code, keep
    .equ     __vector_dispatch_167, IntVectorCRYPTO1_FAULT_Handler
    .global  __vector_dispatch_167
    .set     nomicromips
    .set     noreorder
    .set     nomips16
    .set     noat
    .ent  IntVectorCRYPTO1_FAULT_Handler

IntVectorCRYPTO1_FAULT_Handler:
    portSAVE_CONTEXT
    la    s6,  CRYPTO1_FAULT_Handler
    jalr  s6
    nop
    portRESTORE_CONTEXT
    .end   IntVectorCRYPTO1_FAULT_Handler

