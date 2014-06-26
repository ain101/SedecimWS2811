//--------------------------------------------------------------
// File     : stm32_ub_systick.c
// Datum    : 13.03.2013
// Version  : 1.4
// Autor    : UB
// EMail    : mc-4u(@)t-online.de
// Web      : www.mikrocontroller-4u.de
// CPU      : STM32F4
// IDE      : CooCox CoIDE 1.7.0
// Module   : keine
// Funktion : Pausen- Timer- und Counter-Funktionen
//            Zeiten im [us,ms,s] Raster
//--------------------------------------------------------------

//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "stm32_ub_systick.h"



#if ((SYSTICK_RESOLUTION!=1) && (SYSTICK_RESOLUTION!=1000))
  #error print WRONG SYSTICK RESOLUTION !
#endif


//--------------------------------------------------------------
// Globale Pausen-Variabeln
//--------------------------------------------------------------
static volatile uint32_t Systick_Delay;  // Globaler Pausenzaehler

//--------------------------------------------------------------
// Globale Timer-Variabeln
//--------------------------------------------------------------
static volatile uint32_t Systick_T1;     // Globaler Timer1
static volatile uint32_t Systick_T2;     // Globaler Timer2

//--------------------------------------------------------------
// Globale Counter-Variabeln
//--------------------------------------------------------------
static COUNTER_t Systick_C1;             // Globaler Counter1
static COUNTER_t Systick_C2;             // Globaler Counter2



//--------------------------------------------------------------
// Init vom System-Counter
// entweder im 1us-Takt oder 1ms-Takt
//--------------------------------------------------------------
void UB_Systick_Init(void) {
  RCC_ClocksTypeDef RCC_Clocks;

  // alle Variabeln zur�cksetzen
  Systick_Delay=0;
  Systick_T1=0;        // Timer1 STOP
  Systick_T2=0;        // Timer2 STOP
  Systick_C1.faktor=0; // Counter1 STOP
  Systick_C1.wert=0;
  Systick_C2.faktor=0; // Counter2 STOP
  Systick_C2.wert=0;


  #if SYSTICK_RESOLUTION==1
    // Timer auf 1us einstellen
    RCC_GetClocksFreq(&RCC_Clocks);
    SysTick_Config(RCC_Clocks.HCLK_Frequency / 1000000);
  #else
    // Timer auf 1ms einstellen
    RCC_GetClocksFreq(&RCC_Clocks);
    SysTick_Config(RCC_Clocks.HCLK_Frequency / 1000);
  #endif
}


#if SYSTICK_RESOLUTION==1
//--------------------------------------------------------------
// Pausenfunktion (in us)
// die CPU wartet bis die Zeit abgelaufen ist
//--------------------------------------------------------------
void UB_Systick_Pause_us(volatile uint32_t pause)
{

  Systick_Delay = pause;

  while(Systick_Delay != 0);
}
#endif


//--------------------------------------------------------------
// Pausenfunktion (in ms)
// die CPU wartet bis die Zeit abgelaufen ist
//--------------------------------------------------------------
void UB_Systick_Pause_ms(volatile uint32_t pause)
{
  #if SYSTICK_RESOLUTION==1
    uint32_t ms;

    for(ms=0;ms<pause;ms++) {
      UB_Systick_Pause_us(1000);
    }
  #else
    Systick_Delay = pause;

    while(Systick_Delay != 0);
  #endif
}


//--------------------------------------------------------------
// Pausenfunktion (in s)
// die CPU wartet bis die Zeit abgelaufen ist
//--------------------------------------------------------------
void UB_Systick_Pause_s(volatile uint32_t pause)
{
  uint32_t s;

  for(s=0;s<pause;s++) {
    UB_Systick_Pause_ms(1000);
  }
}


//--------------------------------------------------------------
// Timer1
// Timer kann mit einer Zeit gestartet werden
// und kann dann zyklisch abgefragt werden ob sie abgelaufen ist
// status : [TIMER_STOP, TIMER_START_us/ms/s, TIMER_CHECK]
// wert : Startwert in [us,ms,sec]
// Return Wert bei TIMER_CHECK :
//  -> wenn Timer abgelaufen = TIMER_HOLD
//  -> wenn Timer noch l�uft = TIMER_RUN
//--------------------------------------------------------------
TIMER_CHECK_t UB_Systick_Timer1(TIMER_STATUS_t status, uint32_t wert)
{
  TIMER_CHECK_t ret_wert=TIMER_RUN;

  if(status==TIMER_STOP) {
    // Timer1 schnell anhalten
    Systick_T1=1;
    ret_wert=TIMER_HOLD;
  }
  #if SYSTICK_RESOLUTION==1
    else if(status==TIMER_START_us) {
      // Timer1 im us-Mode starten
      Systick_T1=wert;
    }
    else if(status==TIMER_START_ms) {
      // Timer1 im ms-Mode starten
      Systick_T1=1000*wert;
    }
    else if(status==TIMER_START_s) {
      // Timer1 im s-Mode starten
      if(wert>4293) wert=4293;
      Systick_T1=1000*1000*wert;
    }
  #else
    else if(status==TIMER_START_ms) {
      // Timer1 im ms-Mode starten
      Systick_T1=wert;
    }
    else if(status==TIMER_START_s) {
      // Timer1 im s-Mode starten
      Systick_T1=1000*wert;
    }
  #endif
  else {
    // Staus zur�ckgeben
    if(Systick_T1!=0x00) {
      // Timer l�uft noch
      ret_wert=TIMER_RUN;
    }
    else {
      // Timer ist abgelaufen
      ret_wert=TIMER_HOLD;
    }
  }

  return(ret_wert);
}


//--------------------------------------------------------------
// Timer2
// Timer kann mit einer Zeit gestartet werden
// und kann dann zyklisch abgefragt werden ob sie abgelaufen ist
// status : [TIMER_STOP, TIMER_START_us/ms/s, TIMER_CHECK]
// wert : Startwert in [us,ms,sec]
// Return Wert bei TIMER_CHECK :
//  -> wenn Timer abgelaufen = TIMER_HOLD
//  -> wenn Timer noch l�uft = TIMER_RUN
//--------------------------------------------------------------
TIMER_CHECK_t UB_Systick_Timer2(TIMER_STATUS_t status, uint32_t wert)
{
  TIMER_CHECK_t ret_wert=TIMER_RUN;

  if(status==TIMER_STOP) {
    // Timer2 schnell anhalten
    Systick_T2=1;
    ret_wert=TIMER_HOLD;
  }
  #if SYSTICK_RESOLUTION==1
    else if(status==TIMER_START_us) {
      // Timer2 im us-Mode starten
      Systick_T2=wert;
    }
    else if(status==TIMER_START_ms) {
      // Timer2 im ms-Mode starten
      Systick_T2=1000*wert;
    }
    else if(status==TIMER_START_s) {
      // Timer2 im s-Mode starten
      if(wert>4293) wert=4293;
      Systick_T2=1000*1000*wert;
    }
  #else
    else if(status==TIMER_START_ms) {
      // Timer2 im ms-Mode starten
      Systick_T2=wert;
    }
    else if(status==TIMER_START_s) {
      // Timer2 im s-Mode starten
      Systick_T2=1000*wert;
    }
  #endif
  else {
    // Staus zur�ckgeben
    if(Systick_T2!=0x00) {
      // Timer l�uft noch
      ret_wert=TIMER_RUN;
    }
    else {
      // Timer ist abgelaufen
      ret_wert=TIMER_HOLD;
    }
  }

  return(ret_wert);
}


//--------------------------------------------------------------
// Counter1
// Counter kann bei 0 gestartet werden und zu einem zweiten
// Zeitpunkt kann die Abgelaufene Zeit ausgelesen werden
// status : [COUNTER_STOP, COUNTER_START_us/ms/s, COUNTER_CHECK]
// Return Wert bei COUNTER_CHECK :
//  -> abgelaufene Zeit in [us,ms,sec]
//--------------------------------------------------------------
uint32_t UB_Systick_Counter1(COUNTER_STATUS_t status)
{
  uint32_t ret_wert=0;

  if(status==COUNTER_STOP) {
    // Counter1 anhalten
    Systick_C1.faktor=0;
  }
  #if SYSTICK_RESOLUTION==1
    else if(status==COUNTER_START_us) {
      // Counter1 im us-Mode starten
      Systick_C1.faktor=1;
      Systick_C1.wert=0;
    }
    else if(status==COUNTER_START_ms) {
      // Counter1 im ms-Mode starten
      Systick_C1.faktor=1000;
      Systick_C1.wert=0;
    }
    else if(status==COUNTER_START_s) {
      // Counter1 im s-Mode starten
      Systick_C1.faktor=1000*1000;
      Systick_C1.wert=0;
    }
  #else
    else if(status==COUNTER_START_ms) {
      // Counter1 im ms-Mode starten
      Systick_C1.faktor=1;
      Systick_C1.wert=0;
    }
    else if(status==COUNTER_START_s) {
      // Counter1 im s-Mode starten
      Systick_C1.faktor=1000;
      Systick_C1.wert=0;
    }
  #endif
  else if(status==COUNTER_CHECK) {
    // abgelaufene Zeit zur�ckgeben
    ret_wert=Systick_C1.wert/Systick_C1.faktor;
  }

  return(ret_wert);
}


//--------------------------------------------------------------
// Counter2
// Counter kann bei 0 gestartet werden und zu einem zweiten
// Zeitpunkt kann die Abgelaufene Zeit ausgelesen werden
// status : [COUNTER_STOP, COUNTER_START_us/ms/s, COUNTER_CHECK]
// Return Wert bei COUNTER_CHECK :
//  -> abgelaufene Zeit in [us,ms,sec]
//--------------------------------------------------------------
uint32_t UB_Systick_Counter2(COUNTER_STATUS_t status)
{
  uint32_t ret_wert=0;

  if(status==COUNTER_STOP) {
    // Counter2 anhalten
    Systick_C2.faktor=0;
  }
  #if SYSTICK_RESOLUTION==1
    else if(status==COUNTER_START_us) {
      // Counter2 im us-Mode starten
      Systick_C2.faktor=1;
      Systick_C2.wert=0;
    }
    else if(status==COUNTER_START_ms) {
      // Counter2 im ms-Mode starten
      Systick_C2.faktor=1000;
      Systick_C2.wert=0;
    }
    else if(status==COUNTER_START_s) {
      // Counter2 im s-Mode starten
      Systick_C2.faktor=1000*1000;
      Systick_C2.wert=0;
    }
  #else
    else if(status==COUNTER_START_ms) {
      // Counter2 im ms-Mode starten
      Systick_C2.faktor=1;
      Systick_C2.wert=0;
    }
    else if(status==COUNTER_START_s) {
      // Counter2 im s-Mode starten
      Systick_C2.faktor=1000;
      Systick_C2.wert=0;
    }
  #endif
  else if(status==COUNTER_CHECK) {
    // abgelaufene Zeit zur�ckgeben
    ret_wert=Systick_C2.wert/Systick_C2.faktor;
  }

  return(ret_wert);
}


//--------------------------------------------------------------
// Systic-Interrupt
//--------------------------------------------------------------

void SysTick_Handler(void)
{
  // Tick f�r Pause
  if(Systick_Delay != 0x00) {
    Systick_Delay--;
  }
  // Tick f�r Timer1
  if(Systick_T1 != 0x00) {
    Systick_T1--;
  }
  // Tick f�r Timer2
  if(Systick_T2 != 0x00) {
    Systick_T2--;
  }
  // Tick f�r Counter1
  if(Systick_C1.faktor != 0x00) {
    Systick_C1.wert++;
  }
  // Tick f�r Counter2
  if(Systick_C2.faktor != 0x00) {
    Systick_C2.wert++;
  }
}



