/*******************************************************************************
  ADCHS Peripheral Library Interface Header File

  Company
    Microchip Technology Inc.

  File Name
    plib_adchs_common.h

  Summary
    Commonly needed stuff for the ADCHS peripheral libraries interfaces.

  Description
    This file defines several items commonly needed by the interfaces to
    the ADCHS peripheral libraries.

*******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
* Copyright (C) 2018 Microchip Technology Inc. and its subsidiaries.
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

#ifndef PLIB_ADCHS_COMMON_H    // Guards against multiple inclusion
#define PLIB_ADCHS_COMMON_H


// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

/*  This section lists the other files that are included in this file.
*/

#include <stddef.h>
#include <stdbool.h>


// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

extern "C" {

#endif

// DOM-IGNORE-END





// *****************************************************************************
// *****************************************************************************
// Section: Data Types
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************

typedef enum
{
    ADCHS_MODULE0_MASK = (1U << 0U),
    ADCHS_MODULE7_MASK = (1U << 7U)
}ADCHS_MODULE_MASK;



  #define  ADCHS_CH0  (0U)
  #define  ADCHS_CH1  (1U)
  #define  ADCHS_CH2  (2U)
  #define  ADCHS_CH3  (3U)
  #define  ADCHS_CH4  (4U)
  #define  ADCHS_CH5  (5U)
  #define  ADCHS_CH6  (6U)
  #define  ADCHS_CH7  (7U)
  #define  ADCHS_CH8  (8U)
  #define  ADCHS_CH9  (9U)
  #define  ADCHS_CH10  (10U)
  #define  ADCHS_CH11  (11U)
  #define  ADCHS_CH12  (12U)
  #define  ADCHS_CH13  (13U)
  #define  ADCHS_CH14  (14U)
  #define  ADCHS_CH15  (15U)
  #define  ADCHS_CH16  (16U)
  #define  ADCHS_CH17  (17U)
  #define  ADCHS_CH18  (18U)
  #define  ADCHS_CH19  (19U)
  #define  ADCHS_CH20  (20U)
  #define  ADCHS_CH21  (21U)
  #define  ADCHS_CH22  (22U)
  #define  ADCHS_CH23  (23U)
typedef uint32_t ADCHS_CHANNEL_NUM;



// *****************************************************************************

typedef void (*ADCHS_CALLBACK)(ADCHS_CHANNEL_NUM channel, uintptr_t context);

typedef void (*ADCHS_EOS_CALLBACK)(uintptr_t context);




// *****************************************************************************

typedef struct
{
    ADCHS_CALLBACK callback_fn;
    uintptr_t context;
}ADCHS_CALLBACK_OBJECT;

typedef struct
{
    ADCHS_EOS_CALLBACK callback_fn;
    uintptr_t context;
}ADCHS_EOS_CALLBACK_OBJECT;




// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

}

#endif
// DOM-IGNORE-END

#endif //PLIB_ADCHS_COMMMON_H

/**
 End of File
*/
