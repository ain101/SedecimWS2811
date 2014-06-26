//--------------------------------------------------------------
// File     : stm32_ub_irmp.h
//--------------------------------------------------------------

//--------------------------------------------------------------
#ifndef __STM32F4_UB_IRMP_H
#define __STM32F4_UB_IRMP_H


//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "stm32f4xx.h"
#include "irmp.h"
#include "stm32f4xx_tim.h"
#include "misc.h"



//--------------------------------------------------------------
// Timer Einstellungen (Frequenz)
//
// Grundfrequenz = 2*APB1 (APB1=42MHz) => TIM_CLK=84MHz
// periode   : 0 bis 0xFFFF
// prescale  : 0 bis 0xFFFF
//
// Timer-Frq = TIM_CLK/(periode+1)/(vorteiler+1)
//--------------------------------------------------------------
#define  F_CPU 84000000UL  // TIM_CLK

#define  IRPM_TIM2_PERIODE  7
#define  IRMP_TIM2_PRESCALE ((F_CPU / F_INTERRUPTS)/8) - 1


//--------------------------------------------------------------
// Globale Funktionen
//--------------------------------------------------------------
void UB_IRMP_Init(void);
uint8_t UB_IRMP_Read(IRMP_DATA * irmp_data_p);


//--------------------------------------------------------------
#endif // __STM32F4_UB_IRMP_H
