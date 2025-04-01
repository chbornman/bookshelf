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

/* cJSON */
/* JSON parser in C. */

#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <float.h>
#include <limits.h>
#include <ctype.h>
#include "cJSON.h"

/* define our own boolean type */
#define true 1
#define false 0
typedef int bool;

static const char *global_ep = NULL;

/* Helper functions */
static const char *skip(const char *in)
{
    while (in && *in && (unsigned char)*in <= 32)
    {
        in++;
    }

    return in;
}

/* Parse the input text to generate a number, and populate the result into item. */
static const char *parse_number(cJSON *item, const char *num)
{
    double n = 0;
    double sign = 1;
    double scale = 0;
    int subscale = 0;
    int signsubscale = 1;

    /* Could use sscanf for this? */
    if (*num == '-')
    {
        sign = -1;
        num++;
    }
    if (*num == '0')
    {
        num++;
    }
    if (*num >= '1' && *num <= '9')
    {
        do
        {
            n = (n * 10.0) + (*num++ - '0');
        } while (*num >= '0' && *num <= '9');
    }
    if (*num == '.' && num[1] >= '0' && num[1] <= '9')
    {
        num++;
        do
        {
            n = (n * 10.0) + (*num++ - '0');
            scale--;
        } while (*num >= '0' && *num <= '9');
    }
    if (*num == 'e' || *num == 'E')
    {
        num++;
        if (*num == '+')
        {
            num++;
        }
        else if (*num == '-')
        {
            signsubscale = -1;
            num++;
        }
        while (*num >= '0' && *num <= '9')
        {
            subscale = (subscale * 10) + (*num++ - '0');
        }
    }

    /* number = +/- number.fraction * 10^+/- exponent */
    n = sign * n * pow(10.0, (scale + subscale * signsubscale));

    item->valuedouble = n;
    item->valueint = (int)n;
    item->type = cJSON_Number;

    return num;
}

/* Parse the input text into an unescaped cstring, and populate item. */
static const unsigned char firstByteMark[7] = { 0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };
static const char *parse_string(cJSON *item, const char *str)
{
    const char *ptr = str + 1;
    char *ptr2;
    char *out;
    int len = 0;
    unsigned uc;
    unsigned uc2;

    if (*str != '\"')
    {
        global_ep = str;
        return NULL;
    }
    
    while (*ptr != '\"' && *ptr && ++len)
    {
        if (*ptr++ == '\\')
        {
            ptr++; /* Skip escaped quotes. */
        }
    }
    
    out = (char*)malloc(len + 1); /* This is how long we need for the string, roughly. */
    if (!out)
    {
        return NULL;
    }
    
    ptr = str + 1;
    ptr2 = out;
    while (*ptr != '\"' && *ptr)
    {
        if (*ptr != '\\')
        {
            *ptr2++ = *ptr++;
        }
        else
        {
            ptr++;
            switch (*ptr)
            {
                case 'b':
                    *ptr2++ = '\b';
                    break;
                case 'f':
                    *ptr2++ = '\f';
                    break;
                case 'n':
                    *ptr2++ = '\n';
                    break;
                case 'r':
                    *ptr2++ = '\r';
                    break;
                case 't':
                    *ptr2++ = '\t';
                    break;
                case 'u':
                    /* transcode utf16 to utf8. */
                    sscanf(ptr + 1, "%4x", &uc);
                    ptr += 4; /* get the hex value. */
                    
                    if ((uc >= 0xDC00 && uc <= 0xDFFF) || uc == 0)
                    {
                        break; /* check for invalid. */
                    }
                    
                    /* Check for a surrogate pair */
                    if (uc >= 0xD800 && uc <= 0xDBFF)
                    {
                        if (ptr[1] != '\\' || ptr[2] != 'u')
                        {
                            break; /* invalid surrogate pair */
                        }
                        
                        sscanf(ptr + 3, "%4x", &uc2);
                        ptr += 6;
                        if (uc2 < 0xDC00 || uc2 > 0xDFFF)
                        {
                            break; /* invalid surrogate pair */
                        }
                        
                        uc = 0x10000 + (((uc & 0x3FF) << 10) | (uc2 & 0x3FF));
                    }
                    
                    len = 4;
                    if (uc < 0x80)
                    {
                        len = 1;
                    }
                    else if (uc < 0x800)
                    {
                        len = 2;
                    }
                    else if (uc < 0x10000)
                    {
                        len = 3;
                    }
                    ptr2 += len;
                    
                    switch (len)
                    {
                        case 4:
                            *--ptr2 = ((uc | 0x80) & 0xBF);
                            uc >>= 6;
                        case 3:
                            *--ptr2 = ((uc | 0x80) & 0xBF);
                            uc >>= 6;
                        case 2:
                            *--ptr2 = ((uc | 0x80) & 0xBF);
                            uc >>= 6;
                        case 1:
                            *--ptr2 = (uc | firstByteMark[len]);
                    }
                    ptr2 += len;
                    break;
                default:
                    *ptr2++ = *ptr;
                    break;
            }
            ptr++;
        }
    }
    *ptr2 = 0;
    if (*ptr == '\"')
    {
        ptr++;
    }
    
    item->valuestring = out;
    item->type = cJSON_String;
    
    return ptr;
}

/* Forward declarations */
static const char *parse_value(cJSON *item, const char *value);
static const char *parse_array(cJSON *item, const char *value);
static const char *parse_object(cJSON *item, const char *value);

/* Utility to jump whitespace and cr/lf */
static const char *parse_value(cJSON *item, const char *value)
{
    if (!value)
    {
        return NULL; /* Fail on null. */
    }
    
    if (!strncmp(value, "null", 4))
    {
        item->type = cJSON_NULL;
        return value + 4;
    }
    if (!strncmp(value, "false", 5))
    {
        item->type = cJSON_False;
        return value + 5;
    }
    if (!strncmp(value, "true", 4))
    {
        item->type = cJSON_True;
        return value + 4;
    }
    if (*value == '\"')
    {
        return parse_string(item, value);
    }
    if (*value == '-' || (*value >= '0' && *value <= '9'))
    {
        return parse_number(item, value);
    }
    if (*value == '[')
    {
        return parse_array(item, value);
    }
    if (*value == '{')
    {
        return parse_object(item, value);
    }
    
    global_ep = value;
    return NULL; /* failure. */
}

/* Build an array from input text. */
static const char *parse_array(cJSON *item, const char *value)
{
    cJSON *child = NULL;
    
    if (*value != '[')
    {
        global_ep = value;
        return NULL;
    }
    
    item->type = cJSON_Array;
    value = skip(value + 1);
    if (*value == ']')
    {
        return value + 1; /* empty array. */
    }
    
    item->child = (cJSON*)malloc(sizeof(cJSON));
    if (!item->child)
    {
        return NULL;
    }
    memset(item->child, 0, sizeof(cJSON));
    child = item->child;
    
    value = skip(parse_value(child, skip(value)));
    if (!value)
    {
        return NULL;
    }
    
    while (*value == ',')
    {
        cJSON *new_item = (cJSON*)malloc(sizeof(cJSON));
        if (!new_item)
        {
            return NULL;
        }
        memset(new_item, 0, sizeof(cJSON));
        
        child->next = new_item;
        new_item->prev = child;
        child = new_item;
        
        value = skip(parse_value(child, skip(value + 1)));
        if (!value)
        {
            return NULL;
        }
    }
    
    if (*value == ']')
    {
        return value + 1; /* end of array */
    }
    
    global_ep = value;
    return NULL; /* malformed. */
}

/* Build an object from the text. */
static const char *parse_object(cJSON *item, const char *value)
{
    cJSON *child = NULL;
    
    if (*value != '{')
    {
        global_ep = value;
        return NULL;
    }
    
    item->type = cJSON_Object;
    value = skip(value + 1);
    if (*value == '}')
    {
        return value + 1; /* empty object. */
    }
    
    item->child = (cJSON*)malloc(sizeof(cJSON));
    if (!item->child)
    {
        return NULL;
    }
    memset(item->child, 0, sizeof(cJSON));
    child = item->child;
    
    value = skip(parse_string(child, skip(value)));
    if (!value)
    {
        return NULL;
    }
    
    child->string = child->valuestring;
    child->valuestring = NULL;
    
    if (*value != ':')
    {
        global_ep = value;
        return NULL;
    }
    
    value = skip(parse_value(child, skip(value + 1)));
    if (!value)
    {
        return NULL;
    }
    
    while (*value == ',')
    {
        cJSON *new_item = (cJSON*)malloc(sizeof(cJSON));
        if (!new_item)
        {
            return NULL;
        }
        memset(new_item, 0, sizeof(cJSON));
        
        child->next = new_item;
        new_item->prev = child;
        child = new_item;
        
        value = skip(parse_string(child, skip(value + 1)));
        if (!value)
        {
            return NULL;
        }
        
        child->string = child->valuestring;
        child->valuestring = NULL;
        
        if (*value != ':')
        {
            global_ep = value;
            return NULL;
        }
        
        value = skip(parse_value(child, skip(value + 1)));
        if (!value)
        {
            return NULL;
        }
    }
    
    if (*value == '}')
    {
        return value + 1; /* end of object */
    }
    
    global_ep = value;
    return NULL;
}

/* Parser core - when encountering text, process appropriately. */
cJSON *cJSON_Parse(const char *value)
{
    cJSON *c = (cJSON*)malloc(sizeof(cJSON));
    if (!c)
    {
        return NULL;
    }
    
    memset(c, 0, sizeof(cJSON));
    
    global_ep = NULL;
    
    if (!parse_value(c, skip(value)))
    {
        cJSON_Delete(c);
        return NULL;
    }
    
    return c;
}

/* Delete a cJSON structure. */
void cJSON_Delete(cJSON *c)
{
    cJSON *next = NULL;
    
    while (c)
    {
        next = c->next;
        
        if (c->child)
        {
            cJSON_Delete(c->child);
        }
        
        if (c->valuestring)
        {
            free(c->valuestring);
        }
        
        if (c->string)
        {
            free(c->string);
        }
        
        free(c);
        
        c = next;
    }
}

/* Get Array size/item / object item. */
int cJSON_GetArraySize(const cJSON *array)
{
    cJSON *c = array->child;
    int i = 0;
    
    while (c)
    {
        i++;
        c = c->next;
    }
    
    return i;
}

cJSON *cJSON_GetArrayItem(const cJSON *array, int item)
{
    cJSON *c = array->child;
    
    while (c && item > 0)
    {
        item--;
        c = c->next;
    }
    
    return c;
}

cJSON *cJSON_GetObjectItem(const cJSON *object, const char *string)
{
    cJSON *c = object->child;
    
    while (c && (c->string == NULL || strcmp(c->string, string)))
    {
        c = c->next;
    }
    
    return c;
}

/* Create basic types */
cJSON *cJSON_CreateString(const char *string)
{
    cJSON *item = (cJSON*)malloc(sizeof(cJSON));
    if (item)
    {
        memset(item, 0, sizeof(cJSON));
        item->type = cJSON_String;
        item->valuestring = (char*)malloc(strlen(string) + 1);
        if (item->valuestring)
        {
            strcpy(item->valuestring, string);
        }
    }
    
    return item;
}

cJSON *cJSON_CreateObject(void)
{
    cJSON *item = (cJSON*)malloc(sizeof(cJSON));
    if (item)
    {
        memset(item, 0, sizeof(cJSON));
        item->type = cJSON_Object;
    }
    
    return item;
}

/* Add item to object */
void cJSON_AddItemToObject(cJSON *object, const char *string, cJSON *item)
{
    if (!item)
    {
        return;
    }
    
    if (item->string)
    {
        free(item->string);
    }
    
    item->string = (char*)malloc(strlen(string) + 1);
    if (item->string)
    {
        strcpy(item->string, string);
    }
    
    cJSON *c = object->child;
    
    if (!c)
    {
        object->child = item;
    }
    else
    {
        /* Find the end of the linked-list */
        while (c && c->next)
        {
            c = c->next;
        }
        c->next = item;
        item->prev = c;
    }
}