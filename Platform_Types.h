/*
* Copyright (c) File 2021-10-11,SHENZHEN MINIEYE INNOVATION TECHNOLOGY Co.,Ltd.
* All Rights Reserved.
* Dept.: Software Dept.
* File: Platform_Types.h
* Description: This file contains
               - the basic types and macros definition.
* REQ IDs:
* History:
* 2021-10-11  wangdingwen Draft version
*/

#ifndef PLATFORM_TYPES_H
#define PLATFORM_TYPES_H

/*******************************************************************************
**                      Include Section                                       **
*******************************************************************************/

/*******************************************************************************
**                      Global Symbols                                        **
*******************************************************************************/

/*******************************************************************************
**                      Global Data Types                                     **
*******************************************************************************/

/*
  AUTOSAR integer data types
*/
typedef signed char         sint8;          /*        -128 .. +127            */
typedef unsigned char       uint8;          /*           0 .. 255             */
typedef signed short        sint16;         /*      -32768 .. +32767          */
typedef unsigned short      uint16;         /*           0 .. 65535           */
typedef signed int          sint32;         /* -2147483648 .. +2147483647     */
typedef unsigned int        uint32;         /*           0 .. 4294967295      */
typedef unsigned long long  uint64;
typedef float               float32;
typedef double              float64;

typedef unsigned long       uint8_least;    /* At least 8 bit                 */
typedef unsigned long       uint16_least;   /* At least 16 bit                */
typedef unsigned long       uint32_least;   /* At least 32 bit                */
typedef signed long         sint8_least;    /* At least 7 bit + 1 bit sign    */
typedef signed long         sint16_least;   /* At least 15 bit + 1 bit sign   */
typedef signed long         sint32_least;   /* At least 31 bit + 1 bit sign   */

typedef unsigned char       boolean;        /* for use with TRUE/FALSE        */

#ifndef TRUE                                             /* conditional check */
#define TRUE        1U
#endif

#ifndef FALSE                                            /* conditional check */
#define FALSE       0U
#endif

#ifndef true                                             /* conditional check */
#define true        1U
#endif

#ifndef false                                            /* conditional check */
#define false       0U
#endif

#ifndef True
#define True        1U
#endif

#ifndef False
#define False       0U
#endif

/*******************************************************************************
**                      Global Maros                                  **
*******************************************************************************/
#define E_OK      0U
#define E_NOK  1U

#ifndef NULL
#define NULL (void*)0
#endif

#ifndef NULL_PTR
#define NULL_PTR (void*)0
#endif

#ifndef STD_HIGH
#define STD_HIGH 1U
#endif

#ifndef STD_LOW
#define STD_LOW 0U
#endif

#ifndef STD_ACTIVE
#define STD_ACTIVE 1U
#endif

#ifndef STD_IDLE
#define STD_IDLE 0U
#endif

#ifndef ON
#define ON 1U
#endif

#ifndef OFF
#define OFF 0U
#endif

#ifndef STD_ON
#define STD_ON 1U
#endif

#ifndef STD_OFF
#define STD_OFF 0U
#endif

/*******************************************************************************
**                      Global Data                                           **
*******************************************************************************/

/*******************************************************************************
**                      Global Function Prototypes                            **
*******************************************************************************/

#endif /* PLATFORM_TYPES_H */
