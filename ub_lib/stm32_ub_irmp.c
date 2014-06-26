//--------------------------------------------------------------
// File     : stm32_ub_irmp.c
// Datum    : 09.05.2013
// Version  : 1.0
// Autor    : UB
// EMail    : mc-4u(@)t-online.de
// Web      : www.mikrocontroller-4u.de
// CPU      : STM32F4
// IDE      : CooCox CoIDE 1.7.0
// Module   : IRMP, TIM, MISC (UART bei Logging enable)
// Funktion : Infrarot-Fernbedienungs Auswertung (per IRMP)
// Hinweis  : Benutzt Timer2
//            IR-Empfänger IC (z.B. SFH-5110) wird benötigt
//
//            Im File : "irmpconfig.h" wird eingestellt
//              1. Pinbelegung vom IR-Empfänger (im Beispiel : PC13)
//              2. Benutzte Protokolle (im Beispiel : 6 standard Typen)
//              3. Logging enable per UART an PA2 (115200 Baud)
//
// IRMP     : Quelle = svn://mikrocontroller.net/irmp
//            Version : 2.3.10    Datum : 09.04.2013
//--------------------------------------------------------------


//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "stm32_ub_irmp.h"



//--------------------------------------------------------------
// interne Funktionen
//--------------------------------------------------------------
void P_IRMP_InitTIM(void);
void P_IRMP_InitNVIC(void);


//--------------------------------------------------------------
// init vom IRMP-Modul
//--------------------------------------------------------------
void UB_IRMP_Init(void)
{
  // Init vom Timer
  P_IRMP_InitTIM();
  // Init vom Interrupt
  P_IRMP_InitNVIC();
  // init vom IRMP
  irmp_init();
}


//--------------------------------------------------------------
// Auslesen der IR-Daten
// diese Funktion muss gepollt werden
// Return_Wert :
//   TRUE  => es sind Daten vorhanden
//   FALSE => es sind keine Daten vorhanden
//
// Die Daten stehen in der Struktur "IRMP_DATA"
//   .protocol = Nr. vom IR-Protokol  [1..35]
//   .adress  = Adresse vom IR-Device [0...65535]
//   .command = Commando-Nr           [0...65535]
//   .flags   = Flag-Bits             [0...255]
//                Bit0 = Tastenwiederholung
//--------------------------------------------------------------
uint8_t UB_IRMP_Read(IRMP_DATA * irmp_data_p)
{
  return irmp_get_data(irmp_data_p);
}


//--------------------------------------------------------------
// interne Funktion
// init vom Timer
//--------------------------------------------------------------
void P_IRMP_InitTIM(void)
{
  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

  // Clock enable
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

  // Timer Init
  TIM_TimeBaseStructure.TIM_Period = IRPM_TIM2_PERIODE;
  TIM_TimeBaseStructure.TIM_Prescaler = IRMP_TIM2_PRESCALE;
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

  // Timer Interrupt Enable
  TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

  // Timer enable
  TIM_Cmd(TIM2, ENABLE);
}

//--------------------------------------------------------------
// interne Funktion
// init vom Interrupt
//--------------------------------------------------------------
void P_IRMP_InitNVIC(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;

  // NVIC konfig
  NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0F;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0F;
  NVIC_Init(&NVIC_InitStructure);
}


//--------------------------------------------------------------
// Interrupt
// wird bei Timer2 Interrupt aufgerufen
//--------------------------------------------------------------
void TIM2_IRQHandler(void)
{
  TIM_ClearITPendingBit(TIM2, TIM_IT_Update);

  // IRMP ISR aufrufen
  irmp_ISR();
}
