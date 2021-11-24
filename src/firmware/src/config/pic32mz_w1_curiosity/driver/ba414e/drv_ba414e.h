/*******************************************************************************
  BA414E Crypto Driver Interface Header File

  Company:
    Microchip Technology Inc.

  File Name:
    drv_ba414e.h

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

#ifndef _DRV_BA414E_H_
#define _DRV_BA414E_H_

// *****************************************************************************
// *****************************************************************************
// Section: File includes
// *****************************************************************************
// *****************************************************************************

#include <stddef.h>
#include <stdbool.h>
#include "system/system.h"
#include "driver/driver.h"
#include "drv_ba414e_definitions.h"

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

typedef enum
{
    DRV_BA414E_OP_SUCCESS = 0,
    DRV_BA414E_OP_PENDING = 1,
    DRV_BA414E_OP_POINT_NOT_ON_CURVE = 2,
    DRV_BA414E_OP_POINT_AT_INFINITY = 3,           
    DRV_BA414E_OP_ERROR = -1,        
    DRV_BA414E_OP_BUSY = -2,  
    DRV_BA414E_OP_SIGN_VERIFY_FAIL = -3,
    DRV_BA414E_OP_ERROR_POINT_AT_INFINITY = -4,           
            
} DRV_BA414E_OP_RESULT;

// *****************************************************************************
/* BA414E Driver Callback Function Type

   Summary
    Pointer to a BA414E Callback function

   Description
    This data type defines the required function signature for the BA414E 
 callback function.  This callback function can be used while the driver is in
 non-blocking mode.  This callback function will not be used when the driver is
 in blocking mode.

    The parameters and return values and are described here and
    a partial example implementation is provided.

  Parameters:
    result           - The result of the function that this callback was given to

    context         - The context passed to the non-blocking function.

  Returns:
    None.

*/

typedef void ( *DRV_BA414E_CALLBACK )
(
    DRV_BA414E_OP_RESULT result,
    uintptr_t context
);

// PIC32MZW1 hardware module operand size.
// Domains with smaller operands must be rounded up to the specified bit sizes
typedef enum
{
    DRV_BA414E_OPSZ_128 = 2,
    DRV_BA414E_OPSZ_192 = 3,
    DRV_BA414E_OPSZ_256 = 4,
    DRV_BA414E_OPSZ_320 = 5,
    DRV_BA414E_OPSZ_384 = 6,
    DRV_BA414E_OPSZ_448 = 7,
    DRV_BA414E_OPSZ_512 = 8,
} DRV_BA414E_OPERAND_SIZE;


// Structure describing a set of elliptic curve domain parameters
typedef struct
{
    uint16_t keySize;                               // Key Size
    DRV_BA414E_OPERAND_SIZE opSize;    // Hardware operand size
    const uint8_t * primeField;                     // Prime field
    const uint8_t * order;                          // Order
    const uint8_t * generatorX;                     // Generator point x coordinate
    const uint8_t * generatorY;                     // Generator point y coordinate
    const uint8_t * a;                              // A value
    const uint8_t * b;                              // B value
    int cofactor;                                   // Cofactor
} DRV_BA414E_ECC_DOMAIN;


// *****************************************************************************
// *****************************************************************************
// Section: BA414E Driver Module Interface Routines
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Function:
     SYS_MODULE_OBJ DRV_BA414E_Initialize
    (
        const SYS_MODULE_INDEX index,
        const SYS_MODULE_INIT * const init
    )

  Summary:
    Initializes the crypto hardware instance for the specified driver index.
    <p><b>Implementation:</b> Static</p>

  Description:
    This routine initializes the BA414E crypto driver instance 
    for the specified driver index, making it ready for clients to open and use 
    it. The initialization data is specified by the init parameter. The 
    initialization may fail if the number of driver objects allocated are 
    insufficient or if the specified driver instance is already initialized. 
    The driver instance index is independent of the crypto module ID. For 
    example, driver instance 1 can be assigned to Crypto module 1.  If the 
    driver is built statically, then some of the initialization parameters are 
    overridden by configuration macros. Refer to the description of the 
    DRV_BA414E_INIT data structure for more details on
    which members on this data structure are overridden.

  Precondition:
    None.

  Parameters:
    index  - Identifier for the instance to be initialized

    init   - Pointer to a data structure containing any data necessary to
             initialize the driver.

  Returns:
    If successful, returns a valid handle to a driver instance object.
    Otherwise, returns SYS_MODULE_OBJ_INVALID.

  Example:
    <code>
    // The following code snippet shows an example driver initialization.
    // The current driver implementation does not require any initialization 
    // parameters.

    DRV_BA414E_INIT    cryptoInit;
    SYS_MODULE_OBJ                  objectHandle;

    objectHandle = DRV_BA414E_Initialize(
        DRV_BA414E_MODULE_IDX, 
        (SYS_MODULE_INIT*)&cryptoInit);
 
    if (SYS_MODULE_OBJ_INVALID == objectHandle)
    {
        // Handle error
    }
    </code>

  Remarks:
    This routine must be called before any other BA414E routine is 
    called.

    This routine should only be called once during system initialization
    unless DRV_BA414E_Deinitialize is called to deinitialize the 
    driver instance. This routine will NEVER block for hardware access.
*/

SYS_MODULE_OBJ DRV_BA414E_Initialize
(
    const SYS_MODULE_INDEX index,
    const SYS_MODULE_INIT * const init
);

// *****************************************************************************
/* Function:
    void DRV_BA414E_Deinitialize( SYS_MODULE_OBJ object)

  Summary:
    Deinitializes the specified instance of the BA414E asymmetric crypto 
    driver module.
    <p><b>Implementation:</b> Static</p>

  Description:
    Deinitializes the specified instance of the BA414E asymmetric crypto 
    driver module, disabling its operation (and any hardware).  Invalidates all 
    the internal data.

  Precondition:
    Function DRV_BA414E_Initialize should have been called before 
    calling this function.

  Parameters:
    object          - Driver object handle, returned from the
                      DRV_BA414E_Initialize routine

  Returns:
    None.

  Example:
    <code>
    SYS_MODULE_OBJ      object;     //  Returned from DRV_CRYPTO_P32MZW1_ASYM_Initialize
    SYS_STATUS          status;

    DRV_BA414E_Deinitialize(object);

    status = DRV_BA414E_Status(object);
    if (SYS_MODULE_DEINITIALIZED != status)
    {
        // Check again later if you need to know
        // when the driver is deinitialized.
    }
    </code>

  Remarks:
    Once the Initialize operation has been called, the Deinitialize operation
    must be called before the Initialize operation can be called again. This
    routine will NEVER block waiting for hardware.
*/

void DRV_BA414E_Deinitialize( SYS_MODULE_OBJ object);

// *****************************************************************************
/* Function:
    SYS_STATUS DRV_BA414E_Status( SYS_MODULE_OBJ object )

  Summary:
    Gets the current status of the BA414E crypto driver module.
    <p><b>Implementation:</b> Static</p>

  Description:
    This routine provides the current status of the BA414E crypto 
    driver module.

  Precondition:
    Function DRV_BA414E_Initialize should have been called before 
    calling this function.

  Parameters:
    object          - Driver object handle, returned from the
                      DRV_BA414E_Initialize routine

  Returns:
    SYS_STATUS_READY          - Indicates that the driver is busy with a
                                previous system level operation and cannot start
                                another

    SYS_STATUS_DEINITIALIZED  - Indicates that the driver has been
                                deinitialized

  Example:
    <code>
    SYS_MODULE_OBJ      object;     // Returned from DRV_CRYPTO_P32MZW1_ASYM_Initialize
    SYS_STATUS          status;

    status = DRV_BA414E_Status(object);
    if (SYS_STATUS_READY == status)
    {
        // This means the driver can be opened using the
        // DRV_BA414E_Open() function.
    }
    </code>

  Remarks:
    A driver can opened only when its status is SYS_STATUS_READY.
*/
SYS_STATUS DRV_BA414E_Status( SYS_MODULE_OBJ object);

// *****************************************************************************
/* MPLAB BA414E Driver Tasks

  Summary:
    MPLAB Harmony tasks function used for BA414E tasks.
	<p><b>Implementation:</b> Dynamic</p>
    
  Description:
    This function is called by the main loop.  

  Preconditions:
    The layer must be successfully initialized with DRV_BA414E_Initialize.

  Parameters:
    object	- The valid object passed back to DRV_BA414E_Initialize

  Returns:
    None.
	  
    */

void DRV_BA414E_Tasks(SYS_MODULE_OBJ obj);

// *****************************************************************************
// *****************************************************************************
// Section: BA414E Driver Client Routines
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Function:
    DRV_HANDLE DRV_BA414E_Open
    (
        const SYS_MODULE_INDEX index,
        const DRV_IO_INTENT ioIntent
    )

  Summary:
    Opens the specified BA414E crypto driver instance and returns 
    a handle to it.
    <p><b>Implementation:</b> Static</p>

  Description:
    This routine opens the specified BA414E crypto driver 
    instance and provides a handle that must be provided to all other 
    client-level operations to identify the caller and the instance of the 
    driver. The ioIntent parameter defines how the client interacts with this 
    driver instance.

    The DRV_IO_INTENT_BLOCKING and DRV_IO_INTENT_NONBLOCKING ioIntent options
    additionally affect the behavior of the cryptographic operation functions. 
    If the ioIntent is DRV_IO_INTENT_NONBLOCKING, then these functions will not 
    block during cryptographic primitive execution.
    If the ioIntent is DRV_IO_INTENT_BLOCKING, these functions will block until 
    the the cryptographic primitive execution has completed.  Blocking mode is 
    only available in projects with an RTOS enabled

    Specifying a DRV_IO_INTENT_EXCLUSIVE will cause the driver to provide
    exclusive access to this client. The driver cannot be opened by any
    other client.

  Precondition:
    Function DRV_CRYPTO_P32MZW1_ASYM_Initialize must have been called before 
    calling this function.

  Parameters:
    index   - Identifier for the object instance to be opened

    intent  - Zero or more of the values from the enumeration
              DRV_IO_INTENT "ORed" together to indicate the intended use
              of the driver. See function description for details.

  Returns:
    If successful, the routine returns a valid open-instance handle (a number
    identifying both the caller and the module instance).

    If an error occurs, the return value is DRV_HANDLE_INVALID. Error can occur
    - if the number of client objects allocated via DRV_USART_CLIENTS_NUMBER is
      insufficient.
    - if the client is trying to open the driver but driver has been opened
      exclusively by another client.
    - if the driver hardware instance being opened is not initialized or is
      invalid.
    - if the client is trying to open the driver exclusively, but has already
      been opened in a non exclusive mode by another client.
    - if the driver is not ready to be opened, typically when the initialize
      routine has not completed execution.
    - if the driver is to be opened in blocking mode, in projects without a RTOS

  Example:
    <code>
    DRV_HANDLE handle;

    handle = DRV_BA414E_Open(DRV_BA414E_MODULE_IDX, 
        DRV_IO_INTENT_EXCLUSIVE);
    if (DRV_HANDLE_INVALID == handle)
    {
        // Unable to open the driver
        // May be the driver is not initialized or the initialization
        // is not complete.
    }
    </code>

  Remarks:
    The handle returned is valid until the DRV_BA414E_Close routine 
    is called.
    This routine will NEVER block waiting for hardware. If the requested intent
    flags are not supported, the routine will return DRV_HANDLE_INVALID.  This
    function is thread safe in a RTOS application.
*/

DRV_HANDLE DRV_BA414E_Open( const SYS_MODULE_INDEX index, 
        const DRV_IO_INTENT ioIntent);

// *****************************************************************************
/* Function:
    void DRV_BA414E_Close( DRV_Handle handle )

  Summary:
    Closes an opened-instance of the BA414E crypto driver.
    <p><b>Implementation:</b> Static</p>

  Description:
    This routine closes an opened-instance of the BA414E crypto 
    driver, invalidating the handle. After calling this routine, the handle 
    passed in "handle" must not be used with any of the remaining driver 
    routines. A new handle must be obtained by calling 
    DRV_BA414E_Open before the caller may use the driver again.

  Precondition:
    The DRV_BA414E_Initialize routine must have been called for 
    the specified BA414E crypto driver instance.

    DRV_BA414E_Open must have been called to obtain a valid opened 
    device handle.

  Parameters:
    handle       - A valid open-instance handle, returned from the driver's
                   open routine

  Returns:
    None.

  Example:
    <code>
    DRV_HANDLE handle;  // Returned from DRV_BA414E_Open

    DRV_BA414E_Close(handle);

    </code>
*/
void DRV_BA414E_Close( const DRV_HANDLE handle);

// *****************************************************************************
/* Function:
    DRV_BA414E_OP_RESULT DRV_BA414E_ECDSA_Sign(     const DRV_HANDLE handle,
    const DRV_BA414E_ECC_DOMAIN * domain,
    uint8_t * R,
    uint8_t * S,
    const uint8_t * privateKey,
    const uint8_t * k,
    const uint8_t * msgHash,
    int msgHashSz,
    DRV_BA414E_CALLBACK callback,
    uintptr_t context )

  Summary:
 Conducts an EcDSA signature operation over the provided data.
    <p><b>Implementation:</b> Static</p>

  Description:
    This routine takes the provided parameters and returns a EcDSA signature 
   (R & S).

  Precondition:
    The DRV_BA414E_Initialize routine must have been called for 
    the specified BA414E crypto driver instance.

    DRV_BA414E_Open must have been called to obtain a valid opened 
    device handle.

  Parameters:
    handle       - A valid open-instance handle, returned from the driver's
                   open routine
    domain       - The structure describing the domain of the ECC curve that
                   is being used
    R            - The R component of the generated signature
    S            - The S component of the generated signature
    privateKey   - The private key to be used to generate the signature
    k            - The random array of bytes used as part of the signature
                   this must be of the same size as the key.
    msgHash      - The hash of the message to be signed
    msgHashSz    - The size of the message to be signed
    callback     - The callback that gets called when the operation is complete
                   (Not used with blocking operations)
    context      - The context that gets sent to the callback when the operation
                   is complete. (Not used with blocking operations)

  Returns:
    DRV_BA414E_OP_SUCCESS - If the operation completed successfully
    DRV_BA414E_OP_PENDING - With non-blocking operations, this return signals 
                            that the operation is pending
    DRV_BA414E_OP_ERROR   - There was an error with the operation
    DRV_BA414E_OP_BUSY    - There is currently an operation pending, the caller
                            should wait and try again later
    DRV_BA414E_OP_SIGN_VERIFY_FAIL - Signature generated by the hardware is
                                     invalid

  Example:
    <code>
    DRV_HANDLE handle;  // Returned from DRV_BA414E_Open
    DRV_BA414E_ECC_DOMAIN eccDomain;
    eccDomain.keySize = 32;
    eccDomain.opSize = DRV_BA414E_OPSZ_256;
    eccDomain.primeField = (uint8_t*)&(mpPrime);
    eccDomain.order = (uint8_t*)&(mpOrder);
    eccDomain.generatorX = (uint8_t*)&(mpGx);
    eccDomain.generatorY = (uint8_t*)&(mpGy);
    eccDomain.a = (uint8_t*)&(mpA);
    eccDomain.b = (uint8_t*)&(mpB);

    DRV_BA414E_OP_RESULT ret = DRV_BA414E_ECDSA_Sign(handle, 
            &domain, 
            (uint8_t*)&(r),
            (uint8_t*)&(s),
            (uint8_t*)&(key),
            k,
            hash,
            hashSz,
            0, 0            
            );

    </code>
*/
DRV_BA414E_OP_RESULT DRV_BA414E_ECDSA_Sign(
    const DRV_HANDLE handle,
    const DRV_BA414E_ECC_DOMAIN * domain,
    uint8_t * R,
    uint8_t * S,
    const uint8_t * privateKey,
    const uint8_t * k,
    const uint8_t * msgHash,
    int msgHashSz,
    DRV_BA414E_CALLBACK callback,
    uintptr_t context  
);

// *****************************************************************************
/* Function:
    DRV_BA414E_OP_RESULT DRV_BA414E_ECDSA_Verify(const DRV_HANDLE handle,
    const DRV_BA414E_ECC_DOMAIN * domain,
    uint8_t * R,
    uint8_t * S,
    const uint8_t * privateKey,
    const uint8_t * k,
    const uint8_t * msgHash,
    int msgHashSz,
    DRV_BA414E_CALLBACK callback,
    uintptr_t context )

  Summary:
    Conducts an EcDSA signature verification operation over the provided data.
    <p><b>Implementation:</b> Static</p>

  Description:
    This routine takes the provided parameters and returns a success or signature 
 verification failure based on the provided signature

  Precondition:
    The DRV_BA414E_Initialize routine must have been called for 
    the specified BA414E crypto driver instance.

    DRV_BA414E_Open must have been called to obtain a valid opened 
    device handle.

  Parameters:
    handle       - A valid open-instance handle, returned from the driver's
                   open routine
    domain       - The structure describing the domain of the ECC curve that
                   is being used
    publicKeyX   - The X coordinate of the public key
    publicKeyY   - The Y coordinate of the public key
    R            - The R component of the generated signature
    S            - The S component of the generated signature
    msgHash      - The hash of the message to be signed
    msgHashSz    - The size of the message to be signed
    callback     - The callback that gets called when the operation is complete
                   (Not used with blocking operations)
    context      - The context that gets sent to the callback when the operation
                   is complete. (Not used with blocking operations)

  Returns:
    DRV_BA414E_OP_SUCCESS - If the operation completed successfully and the 
                            signature is valid
    DRV_BA414E_OP_PENDING - With non-blocking operations, this return signals 
                            that the operation is pending
    DRV_BA414E_OP_ERROR   - There was an error with the operation
    DRV_BA414E_OP_BUSY    - There is currently an operation pending, the caller
                            should wait and try again later
    DRV_BA414E_OP_SIGN_VERIFY_FAIL - The operation completed successfully, but
                                     the provided signature is not valid.

  Example:
    <code>
    DRV_HANDLE handle;  // Returned from DRV_BA414E_Open
    DRV_BA414E_ECC_DOMAIN eccDomain;
    eccDomain.keySize = 32;
    eccDomain.opSize = DRV_BA414E_OPSZ_256;
    eccDomain.primeField = (uint8_t*)&(mpPrime);
    eccDomain.order = (uint8_t*)&(mpOrder);
    eccDomain.generatorX = (uint8_t*)&(mpGx);
    eccDomain.generatorY = (uint8_t*)&(mpGy);
    eccDomain.a = (uint8_t*)&(mpA);
    eccDomain.b = (uint8_t*)&(mpB);

    DRV_BA414E_OP_RESULT ret = DRV_BA414E_ECDSA_Verify(
            handle, &domain, 
            pubkey.x,
            pubkey.y,
            (uint8_t*)&(r),
            (uint8_t*)&(s),
            hash,
            hashSz,
            0, 0            
            );


    </code>
*/
DRV_BA414E_OP_RESULT DRV_BA414E_ECDSA_Verify(
    const DRV_HANDLE handle,
    const DRV_BA414E_ECC_DOMAIN * domain,
    const uint8_t * publicKeyX,
    const uint8_t * publicKeyY,
    uint8_t * R,
    uint8_t * S,
    const uint8_t * msgHash,
    int msgHashSz,
    DRV_BA414E_CALLBACK callback,
    uintptr_t context  
);

// *****************************************************************************
/* Function:
    DRV_BA414E_OP_RESULT DRV_BA414E_PRIM_EccPointDouble(const DRV_HANDLE handle,
    const DRV_BA414E_ECC_DOMAIN * domain,
    uint8_t * outX,
    uint8_t * outY,
    const uint8_t * p1X,
    const uint8_t * p1Y,
    DRV_BA414E_CALLBACK callback,
    uintptr_t context )

  Summary:
    Conducts an ECC Point doubling operation based on the parameters.
    <p><b>Implementation:</b> Static</p>

  Description:
 This routine takes the provided point (p1) and doubles it on the provided
 curve (domain).

  Precondition:
    The DRV_BA414E_Initialize routine must have been called for 
    the specified BA414E crypto driver instance.

    DRV_BA414E_Open must have been called to obtain a valid opened 
    device handle.

  Parameters:
    handle       - A valid open-instance handle, returned from the driver's
                   open routine
    domain       - The structure describing the domain of the ECC curve that
                   is being used
    outX         - The X of the doubled point
    outY         - The Y of the doubled point
    p1X          - The X of the point to be doubled
    p1Y          - The Y of the point to be doubled
    callback     - The callback that gets called when the operation is complete
                   (Not used with blocking operations)
    context      - The context that gets sent to the callback when the operation
                   is complete. (Not used with blocking operations)

  Returns:
    DRV_BA414E_OP_SUCCESS - If the operation completed successfully
    DRV_BA414E_OP_PENDING - With non-blocking operations, this return signals 
                            that the operation is pending
    DRV_BA414E_OP_ERROR   - There was an error with the operation
    DRV_BA414E_OP_BUSY    - There is currently an operation pending, the caller
                            should wait and try again later
    
  Example:
    <code>
    DRV_HANDLE handle;  // Returned from DRV_BA414E_Open
    DRV_BA414E_ECC_DOMAIN eccDomain;
    eccDomain.keySize = 32;
    eccDomain.opSize = DRV_BA414E_OPSZ_256;
    eccDomain.primeField = (uint8_t*)&(mpPrime);
    eccDomain.order = (uint8_t*)&(mpOrder);
    eccDomain.generatorX = (uint8_t*)&(mpGx);
    eccDomain.generatorY = (uint8_t*)&(mpGy);
    eccDomain.a = (uint8_t*)&(mpA);
    eccDomain.b = (uint8_t*)&(mpB);

    ret = DRV_BA414E_PRIM_EccPointDouble(ba414Handle, &eccDomain, 
                                        (uint8_t*)&(outX), 
                                        (uint8_t*)&(outY), 
                                        (uint8_t*)&(inX), 
                                        (uint8_t*)&(inY), 
                                        0, 0);

    </code>
*/

DRV_BA414E_OP_RESULT DRV_BA414E_PRIM_EccPointDouble(
    const DRV_HANDLE handle,
    const DRV_BA414E_ECC_DOMAIN * domain,
    uint8_t * outX,
    uint8_t * outY,
    const uint8_t * p1X,
    const uint8_t * p1Y,
    DRV_BA414E_CALLBACK callback,
    uintptr_t context          
);

// *****************************************************************************
/* Function:
    DRV_BA414E_OP_RESULT DRV_BA414E_PRIM_EccPointAddition(const DRV_HANDLE handle,
    const DRV_BA414E_ECC_DOMAIN * domain,
    uint8_t * outX,
    uint8_t * outY,
    const uint8_t * p1X,
    const uint8_t * p1Y,
    const uint8_t * p2X,
    const uint8_t * p2Y,
    DRV_BA414E_CALLBACK callback,
    uintptr_t context )

  Summary:
    Conducts an ECC Point Addition operation based on the parameters.
    <p><b>Implementation:</b> Static</p>

  Description:
 This routine takes the provided points (p1 & P2) and adds them together on 
 the provided curve (domain).  The operation is C = A + B

  Precondition:
    The DRV_BA414E_Initialize routine must have been called for 
    the specified BA414E crypto driver instance.

    DRV_BA414E_Open must have been called to obtain a valid opened 
    device handle.

  Parameters:
    handle       - A valid open-instance handle, returned from the driver's
                   open routine
    domain       - The structure describing the domain of the ECC curve that
                   is being used
    outX         - The X of the output (C)
    outY         - The Y of the output (C)
    p1X          - The X of the first point (A)
    p1Y          - The Y of the first point (A)
    p2X          - The X of the first point (B)
    p2Y          - The Y of the first point (B)
    callback     - The callback that gets called when the operation is complete
                   (Not used with blocking operations)
    context      - The context that gets sent to the callback when the operation
                   is complete. (Not used with blocking operations)

  Returns:
    DRV_BA414E_OP_SUCCESS - If the operation completed successfully
    DRV_BA414E_OP_PENDING - With non-blocking operations, this return signals 
                            that the operation is pending
    DRV_BA414E_OP_ERROR   - There was an error with the operation
    DRV_BA414E_OP_BUSY    - There is currently an operation pending, the caller
                            should wait and try again later
    DRV_BA414E_OP_ERROR_POINT_AT_INFINITY - The operation resulted in a point 
                                            that is positioned at infinity
    
  Example:
    <code>
    DRV_HANDLE handle;  // Returned from DRV_BA414E_Open
    DRV_BA414E_ECC_DOMAIN eccDomain;
    eccDomain.keySize = 32;
    eccDomain.opSize = DRV_BA414E_OPSZ_256;
    eccDomain.primeField = (uint8_t*)&(mpPrime);
    eccDomain.order = (uint8_t*)&(mpOrder);
    eccDomain.generatorX = (uint8_t*)&(mpGx);
    eccDomain.generatorY = (uint8_t*)&(mpGy);
    eccDomain.a = (uint8_t*)&(mpA);
    eccDomain.b = (uint8_t*)&(mpB);

    ret = DRV_BA414E_PRIM_EccPointAddition(ba414Handle, &eccDomain, 
                                    (uint8_t*)&(outX), 
                                    (uint8_t*)&(outY), 
                                    (uint8_t*)&(p1X), 
                                    (uint8_t*)&(p1Y), 
                                    (uint8_t*)&(p2X), 
                                    (uint8_t*)&(p2Y), 
                                    0, 0);

    </code>
*/

DRV_BA414E_OP_RESULT DRV_BA414E_PRIM_EccPointAddition(
    const DRV_HANDLE handle,
    const DRV_BA414E_ECC_DOMAIN * domain,
    uint8_t * outX,
    uint8_t * outY,
    const uint8_t * p1X,
    const uint8_t * p1Y,
    const uint8_t * p2X,
    const uint8_t * p2Y,
    DRV_BA414E_CALLBACK callback,
    uintptr_t context          
);


// *****************************************************************************
/* Function:
    DRV_BA414E_OP_RESULT DRV_BA414E_PRIM_EccPointMultiplication(const DRV_HANDLE handle,
    const DRV_BA414E_ECC_DOMAIN * domain,
    uint8_t * outX,
    uint8_t * outY,
    const uint8_t * p1X,
    const uint8_t * p1Y,
    const uint8_t * k,
    DRV_BA414E_CALLBACK callback,
    uintptr_t context )

  Summary:
    Conducts an ECC Point Multiplication operation based on the parameters.
    <p><b>Implementation:</b> Static</p>

  Description:
 This routine takes the provided point (p1) and multiplies it with a scalar (k) 
 on the provided curve (domain).  The operation is C = k * A

  Precondition:
    The DRV_BA414E_Initialize routine must have been called for 
    the specified BA414E crypto driver instance.

    DRV_BA414E_Open must have been called to obtain a valid opened 
    device handle.

  Parameters:
    handle       - A valid open-instance handle, returned from the driver's
                   open routine
    domain       - The structure describing the domain of the ECC curve that
                   is being used
    outX         - The X of the output (C)
    outY         - The Y of the output (C)
    p1X          - The X of the first point (A)
    p1Y          - The Y of the first point (A)
    k            - The scalar to multiply p1 with
    callback     - The callback that gets called when the operation is complete
                   (Not used with blocking operations)
    context      - The context that gets sent to the callback when the operation
                   is complete. (Not used with blocking operations)

  Returns:
    DRV_BA414E_OP_SUCCESS - If the operation completed successfully 
    DRV_BA414E_OP_PENDING - With non-blocking operations, this return signals 
                            that the operation is pending
    DRV_BA414E_OP_ERROR   - There was an error with the operation
    DRV_BA414E_OP_BUSY    - There is currently an operation pending, the caller
                            should wait and try again later
    
  Example:
    <code>
    DRV_HANDLE handle;  // Returned from DRV_BA414E_Open
    DRV_BA414E_ECC_DOMAIN eccDomain;
    eccDomain.keySize = 32;
    eccDomain.opSize = DRV_BA414E_OPSZ_256;
    eccDomain.primeField = (uint8_t*)&(mpPrime);
    eccDomain.order = (uint8_t*)&(mpOrder);
    eccDomain.generatorX = (uint8_t*)&(mpGx);
    eccDomain.generatorY = (uint8_t*)&(mpGy);
    eccDomain.a = (uint8_t*)&(mpA);
    eccDomain.b = (uint8_t*)&(mpB);

    ret = DRV_BA414E_PRIM_EccPointAddition(ba414Handle, &eccDomain, 
                                    (uint8_t*)&(outX), 
                                    (uint8_t*)&(outY), 
                                    (uint8_t*)&(p1X), 
                                    (uint8_t*)&(p1Y), 
                                    (uint8_t*)&(k), 
                                    0, 0);

    </code>
*/
DRV_BA414E_OP_RESULT DRV_BA414E_PRIM_EccPointMultiplication(
    const DRV_HANDLE handle,
    const DRV_BA414E_ECC_DOMAIN * domain,
    uint8_t * outX,
    uint8_t * outY,
    const uint8_t * p1X,
    const uint8_t * p1Y,
    const uint8_t * k,
    DRV_BA414E_CALLBACK callback,
    uintptr_t context          
);

// *****************************************************************************
/* Function:
    DRV_BA414E_OP_RESULT DRV_BA414E_PRIM_DRV_BA414E_PRIM_EccCheckPointOnCurve(
    const DRV_HANDLE handle,
    const DRV_BA414E_ECC_DOMAIN * domain,
    const uint8_t * p1X,
    const uint8_t * p1Y,
    DRV_BA414E_CALLBACK callback,
    uintptr_t context          
)

  Summary:
    Conducts an ECC Check Point on Curve operation based on the parameters.
    <p><b>Implementation:</b> Static</p>

  Description:
 This routine takes the provided point (p1) and checks to see if it is on the
 provided curve (domain)

  Precondition:
    The DRV_BA414E_Initialize routine must have been called for 
    the specified BA414E crypto driver instance.

    DRV_BA414E_Open must have been called to obtain a valid opened 
    device handle.

  Parameters:
    handle       - A valid open-instance handle, returned from the driver's
                   open routine
    domain       - The structure describing the domain of the ECC curve that
                   is being used
    p1X          - The X of the first point (A)
    p1Y          - The Y of the first point (A)
    callback     - The callback that gets called when the operation is complete
                   (Not used with blocking operations)
    context      - The context that gets sent to the callback when the operation
                   is complete. (Not used with blocking operations)

  Returns:
    DRV_BA414E_OP_SUCCESS - If the operation completed successfully and the 
                            point is on the curve
    DRV_BA414E_OP_PENDING - With non-blocking operations, this return signals 
                            that the operation is pending
    DRV_BA414E_OP_ERROR   - There was an error with the operation
    DRV_BA414E_OP_BUSY    - There is currently an operation pending, the caller
                            should wait and try again later
    DRV_BA414E_OP_POINT_NOT_ON_CURVE - the point is not on the curve.
    
  Example:
    <code>
    DRV_HANDLE handle;  // Returned from DRV_BA414E_Open
    DRV_BA414E_ECC_DOMAIN eccDomain;
    eccDomain.keySize = 32;
    eccDomain.opSize = DRV_BA414E_OPSZ_256;
    eccDomain.primeField = (uint8_t*)&(mpPrime);
    eccDomain.order = (uint8_t*)&(mpOrder);
    eccDomain.generatorX = (uint8_t*)&(mpGx);
    eccDomain.generatorY = (uint8_t*)&(mpGy);
    eccDomain.a = (uint8_t*)&(mpA);
    eccDomain.b = (uint8_t*)&(mpB);

    ret = DRV_BA414E_PRIM_DRV_BA414E_PRIM_EccCheckPointOnCurve(ba414Handle, 
                                   &eccDomain, 
                                    (uint8_t*)&(p1X), 
                                    (uint8_t*)&(p1Y), 
                                    0, 0);

    </code>
*/

DRV_BA414E_OP_RESULT DRV_BA414E_PRIM_EccCheckPointOnCurve(
    const DRV_HANDLE handle,
    const DRV_BA414E_ECC_DOMAIN * domain,
    const uint8_t * p1X,
    const uint8_t * p1Y,
    DRV_BA414E_CALLBACK callback,
    uintptr_t context          
);

// *****************************************************************************
/* Function:
    DRV_BA414E_OP_RESULT DRV_BA414E_PRIM_ModAddition(
    const DRV_HANDLE handle,
    DRV_BA414E_OPERAND_SIZE opSize,    // Hardware operand size
    uint8_t * c,
    const uint8_t * p,
    const uint8_t * a,
    const uint8_t * b,
    DRV_BA414E_CALLBACK callback,
    uintptr_t context          
)

  Summary:
 Conducts a modular addition with the provided parameters
    <p><b>Implementation:</b> Static</p>

  Description:
 TThis routine takes the inputs and conducts a modular addition operation. 
 (C = (a + b) mod P

  Precondition:
    The DRV_BA414E_Initialize routine must have been called for 
    the specified BA414E crypto driver instance.

    DRV_BA414E_Open must have been called to obtain a valid opened 
    device handle.

  Parameters:
    handle       - A valid open-instance handle, returned from the driver's
                   open routine
    opSize       - The size of the operands
    c            - The result of the operation
    p            - The number that serves as the modulus
    a            - The first number to take part in the addition operation
    b            - The second number to take part in the addition operation
    callback     - The callback that gets called when the operation is complete
                   (Not used with blocking operations)
    context      - The context that gets sent to the callback when the operation
                   is complete. (Not used with blocking operations)

  Returns:
    DRV_BA414E_OP_SUCCESS - If the operation completed successfully and the 
                            point is on the curve
    DRV_BA414E_OP_PENDING - With non-blocking operations, this return signals 
                            that the operation is pending
    DRV_BA414E_OP_ERROR   - There was an error with the operation
    DRV_BA414E_OP_BUSY    - There is currently an operation pending, the caller
                            should wait and try again later
    
  Example:
    <code>
    DRV_HANDLE handle;  // Returned from DRV_BA414E_Open
    DRV_BA414E_OP_RESULT ret = DRV_BA414E_PRIM_ModAddition(ba414Handle, 
                                            DRV_BA414E_OPSZ_256, 
                                            (uint8_t*)&(mpC), 
                                            (uint8_t*)&(mpPrime), 
                                            (uint8_t*)&(mpA), 
                                            (uint8_t*)&(mpB), 
                                            0, 0);

    </code>
*/

DRV_BA414E_OP_RESULT DRV_BA414E_PRIM_ModAddition(
    const DRV_HANDLE handle,
    DRV_BA414E_OPERAND_SIZE opSize,    // Hardware operand size
    uint8_t * c,
    const uint8_t * p,
    const uint8_t * a,
    const uint8_t * b,
    DRV_BA414E_CALLBACK callback,
    uintptr_t context          
);
  
// *****************************************************************************
/* Function:
    DRV_BA414E_OP_RESULT DRV_BA414E_PRIM_ModSubtraction(
    const DRV_HANDLE handle,
    DRV_BA414E_OPERAND_SIZE opSize,    // Hardware operand size
    uint8_t * c,
    const uint8_t * p,
    const uint8_t * a,
    const uint8_t * b,
    DRV_BA414E_CALLBACK callback,
    uintptr_t context          
)

  Summary:
 Conducts a modular subtraction with the provided parameters
    <p><b>Implementation:</b> Static</p>

  Description:
 TThis routine takes the inputs and conducts a modular addition operation. 
 (C = (a - b) mod P

  Precondition:
    The DRV_BA414E_Initialize routine must have been called for 
    the specified BA414E crypto driver instance.

    DRV_BA414E_Open must have been called to obtain a valid opened 
    device handle.

  Parameters:
    handle       - A valid open-instance handle, returned from the driver's
                   open routine
    opSize       - The size of the operands
    c            - The result of the operation
    p            - The number that serves as the modulus
    a            - The first number to take part in the subtraction operation
    b            - The second number to take part in the subtraction operation
    callback     - The callback that gets called when the operation is complete
                   (Not used with blocking operations)
    context      - The context that gets sent to the callback when the operation
                   is complete. (Not used with blocking operations)

  Returns:
    DRV_BA414E_OP_SUCCESS - If the operation completed successfully and the 
                            point is on the curve
    DRV_BA414E_OP_PENDING - With non-blocking operations, this return signals 
                            that the operation is pending
    DRV_BA414E_OP_ERROR   - There was an error with the operation
    DRV_BA414E_OP_BUSY    - There is currently an operation pending, the caller
                            should wait and try again later
    
  Example:
    <code>
    DRV_HANDLE handle;  // Returned from DRV_BA414E_Open
    DRV_BA414E_OP_RESULT ret = DRV_BA414E_PRIM_ModSubtraction(ba414Handle, 
                                            DRV_BA414E_OPSZ_256, 
                                            (uint8_t*)&(mpC), 
                                            (uint8_t*)&(mpPrime), 
                                            (uint8_t*)&(mpA), 
                                            (uint8_t*)&(mpB), 
                                            0, 0);

    </code>
*/
DRV_BA414E_OP_RESULT DRV_BA414E_PRIM_ModSubtraction(
    const DRV_HANDLE handle,
    DRV_BA414E_OPERAND_SIZE opSize,    // Hardware operand size
    uint8_t * c,
    const uint8_t * p,
    const uint8_t * a,
    const uint8_t * b,
    DRV_BA414E_CALLBACK callback,
    uintptr_t context          
);

// *****************************************************************************
/* Function:
    DRV_BA414E_OP_RESULT DRV_BA414E_PRIM_ModMultiplication(
    const DRV_HANDLE handle,
    DRV_BA414E_OPERAND_SIZE opSize,    // Hardware operand size
    uint8_t * c,
    const uint8_t * p,
    const uint8_t * a,
    const uint8_t * b,
    DRV_BA414E_CALLBACK callback,
    uintptr_t context          
)

  Summary:
 Conducts a modular multiplication with the provided parameters
    <p><b>Implementation:</b> Static</p>

  Description:
 TThis routine takes the inputs and conducts a modular multiplication operation. 
 (C = (a * b) mod P

  Precondition:
    The DRV_BA414E_Initialize routine must have been called for 
    the specified BA414E crypto driver instance.

    DRV_BA414E_Open must have been called to obtain a valid opened 
    device handle.

  Parameters:
    handle       - A valid open-instance handle, returned from the driver's
                   open routine
    opSize       - The size of the operands
    c            - The result of the operation
    p            - The number that serves as the modulus
    a            - The first number to take part in the multiplication operation
    b            - The second number to take part in the multiplication operation
    callback     - The callback that gets called when the operation is complete
                   (Not used with blocking operations)
    context      - The context that gets sent to the callback when the operation
                   is complete. (Not used with blocking operations)

  Returns:
    DRV_BA414E_OP_SUCCESS - If the operation completed successfully and the 
                            point is on the curve
    DRV_BA414E_OP_PENDING - With non-blocking operations, this return signals 
                            that the operation is pending
    DRV_BA414E_OP_ERROR   - There was an error with the operation
    DRV_BA414E_OP_BUSY    - There is currently an operation pending, the caller
                            should wait and try again later
    
  Example:
    <code>
    DRV_HANDLE handle;  // Returned from DRV_BA414E_Open
    DRV_BA414E_OP_RESULT ret = DRV_BA414E_PRIM_ModMultiplication(ba414Handle, 
                                            DRV_BA414E_OPSZ_256, 
                                            (uint8_t*)&(mpC), 
                                            (uint8_t*)&(mpPrime), 
                                            (uint8_t*)&(mpA), 
                                            (uint8_t*)&(mpB), 
                                            0, 0);

    </code>
*/
DRV_BA414E_OP_RESULT DRV_BA414E_PRIM_ModMultiplication(
    const DRV_HANDLE handle,
    DRV_BA414E_OPERAND_SIZE opSize,    // Hardware operand size
    uint8_t * c,
    const uint8_t * p,
    const uint8_t * a,
    const uint8_t * b,
    DRV_BA414E_CALLBACK callback,
    uintptr_t context          
);


// *****************************************************************************
/* Function:
    DRV_BA414E_OP_RESULT DRV_BA414E_PRIM_ModExponentiation(
    const DRV_HANDLE handle,
    DRV_BA414E_OPERAND_SIZE opSize,    // Hardware operand size
    uint8_t * C,
    const uint8_t * n,
    const uint8_t * M,
    const uint8_t * e,
    DRV_BA414E_CALLBACK callback,
    uintptr_t context          
)

  Summary:
 Conducts a modular exponentiation with the provided parameters
    <p><b>Implementation:</b> Static</p>

  Description:
 TThis routine takes the inputs and conducts a modular exponentiation operation. 
 (C = (M^e) mod n

  Precondition:
    The DRV_BA414E_Initialize routine must have been called for 
    the specified BA414E crypto driver instance.

    DRV_BA414E_Open must have been called to obtain a valid opened 
    device handle.

  Parameters:
    handle       - A valid open-instance handle, returned from the driver's
                   open routine
    opSize       - The size of the operands
    c            - The result of the operation
    n            - The number that serves as the modulus
    M            - The operation that acts as the message
    e            - The operation that acts as the exponent
    callback     - The callback that gets called when the operation is complete
                   (Not used with blocking operations)
    context      - The context that gets sent to the callback when the operation
                   is complete. (Not used with blocking operations)

  Returns:
    DRV_BA414E_OP_SUCCESS - If the operation completed successfully and the 
                            point is on the curve
    DRV_BA414E_OP_PENDING - With non-blocking operations, this return signals 
                            that the operation is pending
    DRV_BA414E_OP_ERROR   - There was an error with the operation
    DRV_BA414E_OP_BUSY    - There is currently an operation pending, the caller
                            should wait and try again later
    
  Example:
    <code>
    DRV_HANDLE handle;  // Returned from DRV_BA414E_Open
    DRV_BA414E_OP_RESULT ret = DRV_BA414E_PRIM_ModExponentiation(ba414Handle, 
                                            DRV_BA414E_OPSZ_256, 
                                            (uint8_t*)&(mpC), 
                                            (uint8_t*)&(mpN), 
                                            (uint8_t*)&(mpM), 
                                            (uint8_t*)&(mpE), 
                                            0, 0);

    </code>
*/
DRV_BA414E_OP_RESULT DRV_BA414E_PRIM_ModExponentiation(
    const DRV_HANDLE handle,
    DRV_BA414E_OPERAND_SIZE opSize,    // Hardware operand size
    uint8_t * C,
    const uint8_t * n,
    const uint8_t * M,
    const uint8_t * e,
    DRV_BA414E_CALLBACK callback,
    uintptr_t context          
);


// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

    }

#endif
// DOM-IGNORE-END



#endif //_DRV_BA414E_H_
