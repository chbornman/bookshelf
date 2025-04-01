/*
  Copyright (c) 2009-2017 Dave Gamble and cJSON contributors

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

#ifndef cJSON__h
#define cJSON__h

#ifdef __cplusplus
extern "C"
{
#endif

#include <stddef.h>

/* cJSON Types: */
#define cJSON_Invalid (0)
#define cJSON_False  (1 << 0)
#define cJSON_True   (1 << 1)
#define cJSON_NULL   (1 << 2)
#define cJSON_Number (1 << 3)
#define cJSON_String (1 << 4)
#define cJSON_Array  (1 << 5)
#define cJSON_Object (1 << 6)
#define cJSON_Raw    (1 << 7)

/* The cJSON structure: */
typedef struct cJSON
{
    /* next/prev allow you to walk array/object chains. Alternatively, use GetArraySize/GetArrayItem/GetObjectItem */
    struct cJSON *next;
    struct cJSON *prev;
    /* An array or object item will have a child pointer pointing to a chain of the items in the array/object. */
    struct cJSON *child;

    /* The type of the item, as above. */
    int type;

    /* The item's string, if type==cJSON_String */
    char *valuestring;
    /* The item's number, if type==cJSON_Number */
    double valuedouble;
    /* The item's number, if type==cJSON_Number */
    int valueint;

    /* The item's name string, if this item is the child of, or is in the list of subitems of an object. */
    char *string;
} cJSON;

/* Parse a string and return a C object representing it */
extern cJSON *cJSON_Parse(const char *value);
/* Delete a cJSON entity and all sub-entities */
extern void cJSON_Delete(cJSON *c);
/* Get the array size of an array or object */
extern int cJSON_GetArraySize(const cJSON *array);
/* Retrieve item number "item" from an array */
extern cJSON *cJSON_GetArrayItem(const cJSON *array, int item);
/* Get item "string" from an object */
extern cJSON *cJSON_GetObjectItem(const cJSON *object, const char *string);

/* These functions check the type of a cJSON item */
#define cJSON_IsString(object) (object ? (object->type & cJSON_String) : 0)
#define cJSON_IsNumber(object) (object ? (object->type & cJSON_Number) : 0)
#define cJSON_IsArray(object) (object ? (object->type & cJSON_Array) : 0)
#define cJSON_IsObject(object) (object ? (object->type & cJSON_Object) : 0)

#ifdef __cplusplus
}
#endif

#endif