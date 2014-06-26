/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * irmpsystem.h - system specific includes and defines
 *
 * Copyright (c) 2009-2013 Frank Meyer - frank(at)fli4l.de
 *
 * $Id: irmpsystem.h,v 1.8 2013/01/17 07:33:13 fm Exp $
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */

//--------------------------------------------------------------
// 09.05.2013
// Angepasst für STM32F4
// UB
// EMail    : mc-4u(@)t-online.de
// Web      : www.mikrocontroller-4u.de
//--------------------------------------------------------------


#ifndef _IRMPSYSTEM_H_
#define _IRMPSYSTEM_H_

#if !defined(_IRMP_H_) && !defined(_IRSND_H_)
#  error please include only irmp.h or irsnd.h, not irmpsystem.h
#endif

#if defined(__18CXX)                                                                // Microchip PIC C18 compiler
#  define PIC_C18
#elif defined(__PCM__) || defined(__PCB__) || defined(__PCH__)                      // CCS PIC compiler
#  define PIC_CCS
#elif defined(STM32L1XX_MD) || defined(STM32L1XX_MDP) || defined(STM32L1XX_HD)      // ARM STM32
#  include <stm32l1xx.h>
#  define ARM_STM32
#  define ARM_STM32L1XX
#elif defined(STM32F10X_LD) || defined(STM32F10X_LD_VL) \
   || defined(STM32F10X_MD) || defined(STM32F10X_MD_VL) \
   || defined(STM32F10X_HD) || defined(STM32F10X_HD_VL) \
   || defined(STM32F10X_XL) || defined(STM32F10X_CL)                                // ARM STM32
#  include <stm32f10x.h>
#  define ARM_STM32
#  define ARM_STM32F10X
#elif defined(STM32F4XX)                                                            // ARM STM32
#  include <stm32f4xx.h>
#  define ARM_STM32
#  define ARM_STM32F4XX
#elif defined(TARGET_IS_BLIZZARD_RA2)                                                                                           // TI Stellaris (tested on Stellaris Launchpad with Code Composer Studio)
#  define STELLARIS_ARM_CORTEX_M4
#  define F_CPU (SysCtlClockGet())
#elif defined(unix) || defined(WIN32)                                               // Unix/Linux or Windows
#  define UNIX_OR_WINDOWS
#else
#  define ATMEL_AVR                                                                 // ATMEL AVR
#endif

#include <string.h>

#ifdef UNIX_OR_WINDOWS                                                              // Analyze on Unix/Linux or Windows
#  include <stdio.h>
#  include <stdlib.h>
#  define F_CPU 8000000L
#  define ANALYZE
#  define DEBUG
#  ifdef unix
#    include <stdint.h>
#  else
typedef unsigned char                   uint8_t;
typedef unsigned short                  uint16_t;
#  endif
#endif



#if defined(ARM_STM32F4XX)
#  include "stm32f4xx_gpio.h"
#  include "stm32f4xx_rcc.h"
#  define PROGMEM
#  define memcpy_P                      memcpy
#else
#  error WRONG CPU TYPE
#endif

#if defined(PIC_CCS) || defined(PIC_C18) || defined(ARM_STM32) || defined(STELLARIS_ARM_CORTEX_M4)
typedef unsigned char                   uint8_t;
typedef unsigned short                  uint16_t;
#endif

#if defined (PIC_C18)
#  include <p18cxxx.h>                                                              // main PIC18 h file
#  include <timers.h>                                                               // timer lib
#  include <pwm.h>                                                                  // pwm lib
#  define IRSND_PIC_CCP1                1                                           // PIC C18 RC2 = PWM1 module
#  define IRSND_PIC_CCP2                2                                           // PIC C18 RC1 = PWM2 module
#endif

#ifndef TRUE
#  define TRUE                          1
#  define FALSE                         0
#endif

typedef struct
{
  uint8_t                               protocol;                                   // protocol, i.e. NEC_PROTOCOL
  uint16_t                              address;                                    // address
  uint16_t                              command;                                    // command
  uint8_t                               flags;                                      // flags, e.g. repetition
} IRMP_DATA;

#endif // _IRMPSYSTEM_H_
