/*******************************************************************************
  ADCHS Peripheral Library Interface Source File

  Company
    Microchip Technology Inc.

  File Name
    plib_adchs.c

  Summary
    ADCHS peripheral library source.

  Description
    This file implements the ADCHS peripheral library.

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
#include "device.h"
#include "plib_adchs.h"
#include "interrupts.h"

#define ADCHS_CHANNEL_32  (32U)

// *****************************************************************************
// *****************************************************************************
// Section: ADCHS Implementation
// *****************************************************************************
// *****************************************************************************

/* Object to hold callback function and context */
volatile static ADCHS_CALLBACK_OBJECT ADCHS_CallbackObj[23];



void ADCHS_Initialize(void)
{
    ADCCON1bits.ON = 0;


    ADCCON1 = 0x660000U;
    ADCCON2 = 0x20001U;
    ADCCON3 = 0x1000000U;

    ADCTRGMODE = 0x0U;

    ADCTRG1 = 0x0U; 
    ADCTRG2 = 0x0U; 
    
    
    
    
    

    ADCTRGSNS = 0x0U;

    ADCIMCON1 = 0x0U;
    ADCIMCON2 = 0x0U; 
    
    

    /* Input scan */
    ADCCSS1 = 0x8000U;
    




/* Result interrupt enable */
ADCGIRQEN1 = 0x8000U;
/* Interrupt Enable */
IEC3SET = 0x2000000U;
IEC4SET = 0x0U;



    /* Turn ON ADC */
    ADCCON1bits.ON = 1;
    while(ADCCON2bits.BGVRRDY == 0U) // Wait until the reference voltage is ready
    {
        /* Nothing to do */
    }
    while(ADCCON2bits.REFFLT != 0U) // Wait if there is a fault with the reference voltage
    {
        /* Nothing to do */
    }

    /* ADC 7 */
    ADCANCONbits.ANEN7 = 1;      // Enable the clock to analog bias
    while(ADCANCONbits.WKRDY7 == 0U) // Wait until ADC is ready
    {
        /* Nothing to do */
    }
    ADCCON3bits.DIGEN7 = 1;      // Enable ADC


}


/* Enable ADCHS channels */
void ADCHS_ModulesEnable (ADCHS_MODULE_MASK moduleMask)
{
    ADCCON3 |= ((uint32_t)moduleMask << 16);
}

/* Disable ADCHS channels */
void ADCHS_ModulesDisable (ADCHS_MODULE_MASK moduleMask)
{
    ADCCON3 &= ~(((uint32_t)moduleMask << 16));
}


void ADCHS_ChannelResultInterruptEnable (ADCHS_CHANNEL_NUM channel)
{
    if (channel < ADCHS_CHANNEL_32)
    {
        ADCGIRQEN1 |= 0x01UL << channel;
    }
}

void ADCHS_ChannelResultInterruptDisable (ADCHS_CHANNEL_NUM channel)
{
    if (channel < ADCHS_CHANNEL_32)
    {
        ADCGIRQEN1 &= ~(0x01UL << channel);
    }
}


void ADCHS_GlobalEdgeConversionStart(void)
{
    ADCCON3bits.GSWTRG = 1;
}

void ADCHS_GlobalLevelConversionStart(void)
{
    ADCCON3bits.GLSWTRG = 1;
}

void ADCHS_GlobalLevelConversionStop(void)
{
    ADCCON3bits.GLSWTRG = 0;
}

void ADCHS_ChannelConversionStart(ADCHS_CHANNEL_NUM channel)
{
    ADCCON3bits.ADINSEL = (uint8_t)channel;
    ADCCON3bits.RQCNVRT = 1;
}


/*Check if conversion result is available */
bool ADCHS_ChannelResultIsReady(ADCHS_CHANNEL_NUM channel)
{
    bool status = false;
    if (channel < ADCHS_CHANNEL_32)
    {
        status = (((ADCDSTAT1 >> channel) & 0x01U) != 0U);
    }
    return status;
}

/* Read the conversion result */
uint16_t ADCHS_ChannelResultGet(ADCHS_CHANNEL_NUM channel)
{
    return (uint16_t)(*((&ADCDATA0) + (channel << 2)));

}

void ADCHS_CallbackRegister(ADCHS_CHANNEL_NUM channel, ADCHS_CALLBACK callback, uintptr_t context)
{
    ADCHS_CallbackObj[channel].callback_fn = callback;
    ADCHS_CallbackObj[channel].context = context;
}




bool ADCHS_EOSStatusGet(void)
{
    return (bool)(ADCCON2bits.EOSRDY);
}

void __attribute__((used)) ADC_DATA15_InterruptHandler(void)
{
    if (ADCHS_CallbackObj[15].callback_fn != NULL)
    {
        uintptr_t context = ADCHS_CallbackObj[15].context;
        ADCHS_CallbackObj[15].callback_fn(ADCHS_CH15, context);
    }

    IFS3CLR = _IFS3_ADCD15IF_MASK;

}

