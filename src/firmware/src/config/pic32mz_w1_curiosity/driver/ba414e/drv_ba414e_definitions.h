/*******************************************************************************
  BA414E Crypto Driver Interface Header File

  Company:
    Microchip Technology Inc.

  File Name:
    drv_ba414e_definitions.h

  Summary:
    BA414E Driver Interface Header File

  Description:
    This device driver provides a simple interface to manage the asymmetric 
    cryptography module on Microchip's PIC32MZ1025W104132 family of 
    microcontrollers.  This file provides the interface definition for this 
    driver.
*******************************************************************************/

//DOM-IGNORE-BEGIN
/*******************************************************************************
Copyright (c) 2012-2018 released Microchip Technology Inc.  All rights reserved.

Microchip licenses to you the right to use, modify, copy and distribute
Software only when embedded on a Microchip microcontroller or digital signal
controller that is integrated into your product or third party product
(pursuant to the sublicense terms in the accompanying license agreement).

You should refer to the license agreement accompanying this Software for
additional information regarding your rights and obligations.

SOFTWARE AND DOCUMENTATION ARE PROVIDED AS IS  WITHOUT  WARRANTY  OF  ANY  KIND,
EITHER EXPRESS  OR  IMPLIED,  INCLUDING  WITHOUT  LIMITATION,  ANY  WARRANTY  OF
MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A  PARTICULAR  PURPOSE.
IN NO EVENT SHALL MICROCHIP OR  ITS  LICENSORS  BE  LIABLE  OR  OBLIGATED  UNDER
CONTRACT, NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION,  BREACH  OF  WARRANTY,  OR
OTHER LEGAL  EQUITABLE  THEORY  ANY  DIRECT  OR  INDIRECT  DAMAGES  OR  EXPENSES
INCLUDING BUT NOT LIMITED TO ANY  INCIDENTAL,  SPECIAL,  INDIRECT,  PUNITIVE  OR
CONSEQUENTIAL DAMAGES, LOST  PROFITS  OR  LOST  DATA,  COST  OF  PROCUREMENT  OF
SUBSTITUTE  GOODS,  TECHNOLOGY,  SERVICES,  OR  ANY  CLAIMS  BY  THIRD   PARTIES
(INCLUDING BUT NOT LIMITED TO ANY DEFENSE  THEREOF),  OR  OTHER  SIMILAR  COSTS.
*******************************************************************************/
//DOM-IGNORE-END

#ifndef _DRV_BA414E_DEFINITIONS_H_
#define _DRV_BA414E_DEFINITIONS_H_

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
/* BA414E Driver Module Index

  Summary:
    BA414E crypto driver index definitions

  Description:
    This constant provide BA414E crypto driver index definitions.

  Remarks:
    This constant should be used in place of a hard-coded numeric literal.
    This value should be passed into the DRV_BA414E_Initialize and
    DRV_BA414E_Open routines to identify the driver instance in use.
*/

#define DRV_BA414E_INDEX_0            0

// *****************************************************************************
/* BA414E Crypto Driver Module Count

  Summary:
    Number of valid BA414E crypto drivers

  Description:
    This constant identifies the maximum number of BA414E Crypto 
    Driver instances that should be defined in the system. Defining more 
    instances than this constant will waste RAM memory space.

    This constant can also be used by the system and application to identify the
    number of BA414E crypto instances on this microcontroller.

  Remarks:
    This value is part-specific.
*/

#define DRV_BA414E_COUNT  /*DOM-IGNORE-BEGIN*/ 1/*DOM-IGNORE-END*/

#define DRV_BA414E_MAX_KEY_SIZE                64
        
// *****************************************************************************
/* BA414E Crypto Driver Result

   Summary
    Identifies the possible result of asymmetric crypto operations.

   Description
    This enumeration identifies the possible result of the buffer processing.

*/

typedef enum
{
    /* Operation succeeded */
    DRV_BA414E_RESULT_OK,

    /* Operation is executing operation (non-blocking) */
    DRV_BA414E_RESULT_WORKING,

    /* Module handle is not valid*/
    DRV_BA414E_RESULT_HANDLE_INVALID,

    /* Operation failed */
    DRV_BA414E_RESULT_FAILED,

    /* Argument invalid */
    DRV_BA414E_ARG_INVALID,

    /* Module is currently in use */            
    DRV_BA414E_RESULT_COULD_NOT_GET_MUTEX,

    /* The result of the operation returned the point at infinity */            
    DRV_BA414E_RESULT_POINT_AT_INFINITY,

    /* A parameter validation operation determined that the specified 
     * parameters were not valid 
     Possible operations:
        Signature validation
        Check ECC a and b parameters
        Check ECC order
        Check ECC point coordinates
        Check ECC point-on-curve 
        ECC k value leads to r == 0 or s == 0*/
    DRV_BA414E_RESULT_CHECK_PARAMS_INVALID,
} DRV_BA414E_RESULT;

// *****************************************************************************
/* BA414E Crypto Driver Buffer Events

   Summary
    Identifies the possible events that can result from a crypto operation.

   Description
    This enumeration identifies the possible events that can result from a
    crypto operation initiated by the client.

   Remarks:
    One of these values is passed in the "event" parameter of the event
    handling callback function that the client registered with the driver by
    calling the DRV_BA414E_EventHandlerSet function when a crypto
    operation is completed.

*/

typedef enum
{
    /* The crypto operation was completed successfully. */
    DRV_BA414E_EVENT_COMPLETE,

    /* There was an error while performing the crypto operation. */
    DRV_BA414E_EVENT_ERROR
            
} DRV_BA414E_EVENT;


// *****************************************************************************
/* BA414E Crypto Driver Buffer Handler Function Pointer

   Summary
    Pointer to a BA414E Crypto Driver Event handler function

   Description
    This data type defines the required function signature for the BA414E 
    Crypto driver event handling callback function. A client must 
    register a pointer to an event handling function whose function signature 
    (parameter and return value types) match the types specified by this 
    function pointer in order to receive event calls back from the driver.

    The parameters and return values and are described here and
    a partial example implementation is provided.

  Parameters:
    event           - Identifies the type of event

    context         - Value identifying the context of the application that registered
                      the event handling function.

  Returns:
    None.

  Example:
    <code>
    void APP_MyEventHandler( DRV_BA414E_EVENT event,
                                uintptr_t context )
    {
        MY_APP_DATA_STRUCT pAppData = (MY_APP_DATA_STRUCT) context;

        switch(event)
        {
            case DRV_BA414E_EVENT_COMPLETE:

                // Retrieve the crypto operation result
                break;

            case DRV_BA414E_EVENT_ERROR:
            default:

                // Handle error.
                break;
        }
    }
    </code>

  Remarks:
    If the event is DRV_BA414E_EVENT_COMPLETE, it means that the 
    operation completed successfully.

    If the event is DRV_BA414E_EVENT_ERROR, it means that the 
    operation did not complete successfully.

    The context parameter contains the a handle to the client context,
    provided at the time the event handling function was registered using the
    DRV_BA414E_EventHandlerSet function.  This context handle value 
    is passed back to the client as the "context" parameter.  It can be any 
    value necessary to identify the client context or instance (such as a 
    pointer to the client's data) instance of the client that made the buffer 
    add request.

    The event handler function executes in the driver peripheral's interrupt
    context when the driver is configured for interrupt mode operation. It is
    recommended of the application to not perform process intensive or blocking
    operations with in this function.
*/

typedef void ( *DRV_BA414E_EVENT_HANDLER )
(
    DRV_BA414E_EVENT event,
    uintptr_t context
);

// *****************************************************************************
/* BA414E Crypto Driver Initialization Data

  Summary:
    Defines the data required to initialize or reinitialize the BA414E 
    Crypto driver

  Description:
    This data type defines the data required to initialize or reinitialize the
    BA414E Crypto driver. If the driver is built statically, the 
    members of this data structure are statically over-ridden by static override 
    definitions in the system_config.h file.

  Remarks:
    None.
*/

typedef struct
{

} DRV_BA414E_INIT_DATA;

//DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

#endif //_DRV_BA414E_DEFINITIONS_H_
