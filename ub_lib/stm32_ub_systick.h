//--------------------------------------------------------------
// File     : stm32_ub_systick.h
//--------------------------------------------------------------

//--------------------------------------------------------------
#ifndef __STM32F4_UB_SYSTICK_H
#define __STM32F4_UB_SYSTICK_H



//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "stm32f4xx.h"
#include "stm32f4xx_rcc.h"


//--------------------------------------------------------------
// Aufl�sung der Systick
// (entweder 1us oder 1000us als Aufl�sung einstellen)
//--------------------------------------------------------------
#define  SYSTICK_RESOLUTION   1    // 1us Aufl�sung
//#define  SYSTICK_RESOLUTION   1000   // 1ms Aufl�sung




//--------------------------------------------------------------
// Status von einem Timer
//--------------------------------------------------------------
#if SYSTICK_RESOLUTION==1
  typedef enum {
    TIMER_STOP =0,  // Timer stoppen und r�cksetzen
    TIMER_START_us, // Timer starten im us-Mode
    TIMER_START_ms, // Timer starten im ms-Mode
    TIMER_START_s,  // Timer starten im s-Mode
    TIMER_CHECK     // Test ob Timer abgelaufen
  }TIMER_STATUS_t;
#else
  typedef enum {
    TIMER_STOP =0,  // Timer stoppen und r�cksetzen
    TIMER_START_ms, // Timer starten im ms-Mode
    TIMER_START_s,  // Timer starten im s-Mode
    TIMER_CHECK     // Test ob Timer abgelaufen
  }TIMER_STATUS_t;
#endif

typedef enum {
  TIMER_HOLD =0,  // Timer ist abgelaufen
  TIMER_RUN       // Timer l�uft noch
}TIMER_CHECK_t;


//--------------------------------------------------------------
// Status von einem Counter
//--------------------------------------------------------------
#if SYSTICK_RESOLUTION==1
  typedef enum {
    COUNTER_STOP =0,  // Counter stoppen
    COUNTER_START_us, // Counter starten im us-Mode
    COUNTER_START_ms, // Counter starten im ms-Mode
    COUNTER_START_s,  // Counter starten im s-Mode
    COUNTER_CHECK     // Test wieviel Zeit vergangen
  }COUNTER_STATUS_t;
#else
  typedef enum {
    COUNTER_STOP =0,  // Counter stoppen
    COUNTER_START_ms, // Counter starten im ms-Mode
    COUNTER_START_s,  // Counter starten im s-Mode
    COUNTER_CHECK     // Test wieviel Zeit vergangen
  }COUNTER_STATUS_t;
#endif

typedef struct {
  uint32_t faktor;
  uint32_t wert;
}COUNTER_t;


//--------------------------------------------------------------
// Globale Pausen-Funktionen
//--------------------------------------------------------------
void UB_Systick_Init(void);
#if SYSTICK_RESOLUTION==1
  void UB_Systick_Pause_us(volatile uint32_t pause);
#endif
void UB_Systick_Pause_ms(volatile uint32_t pause);
void UB_Systick_Pause_s(volatile uint32_t pause);


//--------------------------------------------------------------
// Globale Timer-Funktionen
//--------------------------------------------------------------
TIMER_CHECK_t UB_Systick_Timer1(TIMER_STATUS_t status, uint32_t wert);
TIMER_CHECK_t UB_Systick_Timer2(TIMER_STATUS_t status, uint32_t wert);


//--------------------------------------------------------------
// Globale Counter-Funktionen
//--------------------------------------------------------------
uint32_t UB_Systick_Counter1(COUNTER_STATUS_t status);
uint32_t UB_Systick_Counter2(COUNTER_STATUS_t status);




//--------------------------------------------------------------
#endif // __STM32F4_UB_SYSTICK_H


