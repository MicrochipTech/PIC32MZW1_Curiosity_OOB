/*******************************************************************************
  Wi-Fi Provisioning system service JSON parser

  File Name
    sys_wifiprov_json.h

  Summary
    Wi-Fi Provisioning system service JSON parser

  Description
    Wi-Fi Provisioning system service JSON parser

 *******************************************************************************/

//DOM-IGNORE-BEGIN
/*******************************************************************************
Copyright (C) 2020 released Microchip Technology Inc.  All rights reserved.

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



#ifndef JSON_H_INCLUDED
#define JSON_H_INCLUDED


// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C"
{
#endif

/** Max token size of JSON element. Token means that pair of key and value. */
#define JSON_MAX_TOKEN_SIZE 64
/** Max size of token name. */
#define JSON_MAX_NAME_SIZE 16

/**
 * \brief JSON type.
 */
enum json_type
{
	JSON_TYPE_NULL = 0,
	JSON_TYPE_OBJECT,
	JSON_TYPE_STRING,
	JSON_TYPE_BOOLEAN,
	JSON_TYPE_INTEGER,
	JSON_TYPE_REAL,
	JSON_TYPE_ARRAY,
	JSON_TYPE_MAX
};

/** \brief JSON data structure. */
struct json_obj
{
	/** Type of this data. */
	enum json_type type;
	/** Name of this data. */
	char name[JSON_MAX_NAME_SIZE];
	/** End pointer of JSON buffer. */
	char *end_ptr;

	/** Value of this JSON token. */
	union
	{
		/* String data. */
		char s[JSON_MAX_TOKEN_SIZE - JSON_MAX_NAME_SIZE];
		/* Boolean data. */
		int b;
		/* Fixed number data. */
		int i;
		/* Real number data. */
		double d;
		/* Object or Array data. */
		char *o; /* Start point of object. */
	} value;
};

/**
 * \brief Create the JSON data from the string buffer.
 *
 * \param[out] obj             Pointer of JSON token which will be stored json informations.
 * \param[in]  data            JSON data represented as a string.
 * \param[in]  data_len        JSON data length.
 *
 * \return     0               Success.
 * \return     otherwise       Failed to create.
 */
int json_create(struct json_obj *obj, const char *data, int data_len);

/**
 * \brief Get child count in the JSON object.
 *
 * The input should be an object or array.
 *
 * \param[in]  obj             Pointer of the parent JSON data.
 *
 * \return     0               Success.
 * \return     otherwise       Failed to create.
 */
int json_get_child_count(struct json_obj *obj);

/**
 * \brief Get child data in the JSON object.
 *
 * The input should be an object or array.
 *
 * \param[in]  obj             Pointer of the parent JSON data.
 * \param[in]  index           Index which is located in the parent object.
 * \param[in]  out             Pointer of JSON token which will be stored child JSON informations.
 *
 * \return     0               Success.
 * \return     otherwise       Failed to create.
 */
int json_get_child(struct json_obj *obj, int index, struct json_obj *out);

/**
 * \brief Find data from the JSON object
 *
 * The input should be an object or array.
 * This function supported colon separated search.
 *
 * If JSON data is as follows
 * {"obj1":{"data1":"value1","data2":"value2","obj2":{"data3":"value3"}}}
 * You can be found value of data3 using the following name variable.
 * "obj1:obj2:data3"
 *
 * \param[in]  obj             Pointer of the parent JSON data.
 * \param[in]  name            The name of the item you are looking for.
 * \param[in]  out             Pointer of JSON token which will be stored child JSON informations.
 *
 * \return     0               Success.
 * \return     otherwise       Failed to create.
 */
int json_find(struct json_obj *obj, const char *name, struct json_obj *out);


#ifdef __cplusplus
}
#endif

#endif /* JSON_H_INCLUDED */

