//////////////////////////////////////////////////////////////////////////
///                                                                    ///
/// SedecimWS2811                                                      ///
///                                                                    ///
/// Copyright (c) Frederik Wenigwieser, frederik.wenigwieser@atn.ac.at ///
///                                                                    ///
//////////////////////////////////////////////////////////////////////////

#include "ws2811.h"
#include <stm32f4xx_rcc.h>
#include <stm32f4xx_gpio.h>
#include <stm32f4xx_tim.h>
#include <stm32f4xx_dma.h>

#include "stm32_ub_systick.h"

#ifdef DEBUG_WS2811
	#include "stm32_ub_led.h"//DEBUG
#endif //DEBUG_WS2811


static uint16_t buf1 [PWM_BUFFER_SIZE];
static uint16_t buf2 [PWM_BUFFER_SIZE];

static uint16_t (*frameBuffer) [PWM_BUFFER_SIZE];
static uint16_t (*drawBuffer) [PWM_BUFFER_SIZE];

static const uint16_t ones = 0xFFFF;//0x7FFF;//0xFFFF;// PB15 is used by USB OTG

static void Init_dma(void);
static void init_buffers(void);

static void Init_dma(void)
{
	static DMA_InitTypeDef dma_init2 =		//Reference Manual S 164
	{
			.DMA_BufferSize 		= PWM_BUFFER_SIZE,
			.DMA_Channel 			= DMA_Channel_6,
			.DMA_DIR 				= DMA_DIR_MemoryToPeripheral,
			.DMA_FIFOMode 			= DMA_FIFOMode_Disable,
			.DMA_FIFOThreshold 		= DMA_FIFOThreshold_HalfFull,
			.DMA_Memory0BaseAddr 	= (uint32_t) &ones,
			.DMA_MemoryBurst 		= DMA_MemoryBurst_Single,
			.DMA_MemoryDataSize 	= DMA_MemoryDataSize_HalfWord,
			.DMA_MemoryInc 			= DMA_MemoryInc_Disable,
			.DMA_Mode 				= DMA_Mode_Normal,
			.DMA_PeripheralBaseAddr = (uint32_t) &GPIOB->BSRRL,
			.DMA_PeripheralBurst 	= DMA_PeripheralBurst_Single,
			.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord,
			.DMA_PeripheralInc	 	= DMA_PeripheralInc_Disable,
			.DMA_Priority 			= DMA_Priority_Medium
	};
	// DMA2_Stream6 generates rising edge
	DMA_Init(DMA2_Stream6, &dma_init2);

	//DMA_StructInit(&dma_init)
	static DMA_InitTypeDef dma_init =		//Reference Manual S 164
	{
			.DMA_BufferSize 		= PWM_BUFFER_SIZE,
			.DMA_Channel 			= DMA_Channel_6,
			.DMA_DIR 				= DMA_DIR_MemoryToPeripheral,
			.DMA_FIFOMode 			= DMA_FIFOMode_Disable,
			.DMA_FIFOThreshold 		= DMA_FIFOThreshold_HalfFull,
			//.DMA_Memory0BaseAddr 	= (uint32_t) drawBuffer,// drawBuffer not Constant
			.DMA_MemoryBurst 		= DMA_MemoryBurst_Single,
			.DMA_MemoryDataSize 	= DMA_MemoryDataSize_HalfWord,
			.DMA_MemoryInc 			= DMA_MemoryInc_Enable,
			.DMA_Mode 				= DMA_Mode_Normal,
			.DMA_PeripheralBaseAddr = (uint32_t) &GPIOB->BSRRH,
			.DMA_PeripheralBurst 	= DMA_PeripheralBurst_Single,
			.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord,
			.DMA_PeripheralInc	 	= DMA_PeripheralInc_Disable,
			.DMA_Priority 			= DMA_Priority_Medium
	};
	dma_init.DMA_Memory0BaseAddr 	= (uint32_t) frameBuffer;
	// DMA2_Stream3 generates falling edge if databit is low
	DMA_Init(DMA2_Stream3, &dma_init);

	static DMA_InitTypeDef dma_init3 =		//Reference Manual S 164
	{
			.DMA_BufferSize 		= PWM_BUFFER_SIZE,
			.DMA_Channel 			= DMA_Channel_6,
			.DMA_DIR 				= DMA_DIR_MemoryToPeripheral,
			.DMA_FIFOMode 			= DMA_FIFOMode_Disable,
			.DMA_FIFOThreshold 		= DMA_FIFOThreshold_HalfFull,
			.DMA_Memory0BaseAddr 	= (uint32_t) &ones,
			.DMA_MemoryBurst 		= DMA_MemoryBurst_Single,
			.DMA_MemoryDataSize 	= DMA_MemoryDataSize_HalfWord,
			.DMA_MemoryInc 			= DMA_MemoryInc_Disable,
			.DMA_Mode 				= DMA_Mode_Normal,
			.DMA_PeripheralBaseAddr = (uint32_t) &GPIOB->BSRRH,
			.DMA_PeripheralBurst 	= DMA_PeripheralBurst_Single,
			.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord,
			.DMA_PeripheralInc 		= DMA_PeripheralInc_Disable,
			.DMA_Priority 			= DMA_Priority_Medium
	};
	// DMA2_Stream2 generates final falling edge
	DMA_Init(DMA2_Stream2, &dma_init3);
}

static void init_buffers(void)
{
	frameBuffer = &buf1;
	drawBuffer = &buf2;

	for(int i = 0; i < PWM_BUFFER_SIZE; i++){
		buf1[i] = (uint16_t)0xFFFF;
		buf2[i] = (uint16_t)0xFFFF;
	}
}

void Ws2811_init() {
	init_buffers();

	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_OCInitTypeDef TIM_OCInitStructure;

	// GPIOE clock enable

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
#ifdef DEBUG_WS2811
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);

	// TIM1 channel 1 pin configuration
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOE, &GPIO_InitStructure);

	// TIM1 channel 2 pin configuration
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOE, &GPIO_InitStructure);
#endif //DEBUG_WS2811
	// DATA OUT PORT (16 data streams for ws2811)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;

    GPIO_Init(GPIOB, &GPIO_InitStructure);
    //GPIO_ResetBits(GPIOB, GPIO_Pin_All);

	GPIO_PinAFConfig(GPIOE, GPIO_PinSource9, GPIO_AF_TIM1);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource11, GPIO_AF_TIM1);

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);

	// TIM1
	TIM_TimeBaseStructInit (&TIM_TimeBaseStructure);
	TIM_TimeBaseStructure.TIM_Prescaler = 0;
	TIM_TimeBaseStructure.TIM_Period = 104*2;// Timing for WS2811 (frequency)
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

	TIM_TimeBaseInit(TIM1 , &TIM_TimeBaseStructure);

	// PWM1 Mode configuration
	TIM_OCStructInit (&TIM_OCInitStructure);
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;

	TIM_OC3Init(TIM1 , &TIM_OCInitStructure);
	TIM_OC1Init(TIM1 , &TIM_OCInitStructure);
	TIM_OC2Init(TIM1 , &TIM_OCInitStructure);
	//TIM1->RCR = 0;//DEBUG
	//TIM_OC2PreloadConfig(TIM1, TIM_OCPreload_Enable);// to automatically disable TIM1
#ifdef DEBUG_WS2811
	TIM_CtrlPWMOutputs(TIM1, ENABLE);		//DEBUG Signals to trigger DMAs
#endif //DEBUG_WS2811

	// Set Timing for WS2811 (pulse width)
	TIM_SetCompare3(TIM1, 0);
	TIM_SetCompare1(TIM1, 10*2);//29*2)
	TIM_SetCompare2(TIM1, 70*2);//58*2

	// enable 3 PWM
	TIM_CCxCmd(TIM1, TIM_Channel_3, TIM_CCx_Enable);
	TIM_CCxCmd(TIM1, TIM_Channel_1, TIM_CCx_Enable);
	TIM_CCxCmd(TIM1, TIM_Channel_2, TIM_CCx_Enable);

	// Enable Timer
	TIM_Cmd(TIM1 , ENABLE);

	// DMA
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);

	Init_dma();

	// DMA Transfer complete interrupt
#ifdef DEBUG_WS2811
	DMA_ITConfig(DMA2_Stream6, DMA_IT_TC, ENABLE);
	DMA_ITConfig(DMA2_Stream3, DMA_IT_TC, ENABLE);
#endif //DEBUG_WS2811
	DMA_ITConfig(DMA2_Stream2, DMA_IT_TC, ENABLE);

	// DMA Transfer error
#ifdef DEBUG_WS2811
	DMA_ITConfig(DMA2_Stream6, DMA_IT_TE, ENABLE);
	DMA_ITConfig(DMA2_Stream3, DMA_IT_TE, ENABLE);
	DMA_ITConfig(DMA2_Stream2, DMA_IT_TE, ENABLE);
#endif //DEBUG_WS2811

	// NVIC for DMA
	NVIC_InitTypeDef nvic_init;
#ifdef DEBUG_WS2811
	nvic_init.NVIC_IRQChannel = DMA2_Stream6_IRQn;
	nvic_init.NVIC_IRQChannelPreemptionPriority = 4;
	nvic_init.NVIC_IRQChannelSubPriority = 0;
	nvic_init.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvic_init);

	nvic_init.NVIC_IRQChannel = DMA2_Stream3_IRQn;
	nvic_init.NVIC_IRQChannelPreemptionPriority = 4;
	nvic_init.NVIC_IRQChannelSubPriority = 0;
	nvic_init.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvic_init);
#endif //DEBUG_WS2811
	nvic_init.NVIC_IRQChannel = DMA2_Stream2_IRQn;
	nvic_init.NVIC_IRQChannelPreemptionPriority = 4;
	nvic_init.NVIC_IRQChannelSubPriority = 0;
	nvic_init.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvic_init);


	// Start DMA
	DMA_Cmd(DMA2_Stream6, ENABLE);
	DMA_Cmd(DMA2_Stream3, ENABLE);
	DMA_Cmd(DMA2_Stream2, ENABLE);

	// connect DMA streams with timer
	TIM_DMACmd(TIM1, TIM_DMA_CC3 | TIM_DMA_CC1 | TIM_DMA_CC2, ENABLE);
}

#ifdef DEBUG_WS2811
void DMA2_Stream6_IRQHandler(void)
{
	// ERROR
	if (DMA_GetITStatus(DMA2_Stream6, DMA_IT_TEIF6)) {
		DMA_ClearITPendingBit(DMA2_Stream6, DMA_IT_TEIF6);
		//TODO some error handling
	}
	// Transfer completed
	// first DMA which finishes
	if (DMA_GetITStatus(DMA2_Stream6, DMA_IT_TCIF6)) {
		DMA_ClearITPendingBit(DMA2_Stream6, DMA_IT_TCIF6);
		UB_Led_Toggle(LED_ORANGE);//DEBUG
	}
}

void DMA2_Stream3_IRQHandler(void)
{
	// ERROR
	if (DMA_GetITStatus(DMA2_Stream3, DMA_IT_TEIF3)) {
		DMA_ClearITPendingBit(DMA2_Stream3, DMA_IT_TEIF3);
		//TODO
	}
	// Transfer completed
	// second DMA which finishes
	if (DMA_GetITStatus(DMA2_Stream3, DMA_IT_TCIF3)) {
		DMA_ClearITPendingBit(DMA2_Stream3, DMA_IT_TCIF3);
		UB_Led_Toggle(LED_RED);//DEBUG
	}
}
#endif //DEBUG_WS2811

void DMA2_Stream2_IRQHandler(void)
{
#ifdef DEBUG_WS2811
	// ERROR
	if (DMA_GetITStatus(DMA2_Stream2, DMA_IT_TEIF2)) {
		DMA_ClearITPendingBit(DMA2_Stream2, DMA_IT_TEIF2);
		//TODO
	}
#endif //DEBUG_WS2811
	// Transfer completed
	// last(third) DMA which finishes
	if (DMA_GetITStatus(DMA2_Stream2, DMA_IT_TCIF2)) {
		DMA_ClearITPendingBit(DMA2_Stream2, DMA_IT_TCIF2);

		led_update_in_progress = 0;
#ifdef DEBUG_WS2811
		UB_Led_Toggle(LED_BLUE);//DEBUG
#endif //DEBUG_WS2811
		UB_Systick_Counter2(COUNTER_START_us);// counter for ws2811 latch
	}
}

void LedShow()
{
	// perform latch
	while(UB_Systick_Counter2(COUNTER_CHECK) < LED_RESET_TIME);
	UB_Systick_Counter2(COUNTER_STOP);
	led_update_in_progress = 1;

	// interrupt flags must be cleared to restart DMA
	DMA_ClearITPendingBit(DMA2_Stream6, DMA_IT_TCIF6);
	DMA_ClearITPendingBit(DMA2_Stream3, DMA_IT_TCIF3);
	TIM_DMACmd(TIM1, TIM_DMA_CC3 | TIM_DMA_CC1 | TIM_DMA_CC2, DISABLE);

	DMA_Cmd(DMA2_Stream6, DISABLE);
	while(DMA_GetCmdStatus(DMA2_Stream6));// TODO some timeout
	DMA_Cmd(DMA2_Stream3, DISABLE);
	while(DMA_GetCmdStatus(DMA2_Stream3));
	DMA_Cmd(DMA2_Stream2, DISABLE);
	while(DMA_GetCmdStatus(DMA2_Stream2));

	// clear interrupt flags
	DMA2->LISR = 0x00000000;
	DMA2->HISR = 0x00000000;

	static uint16_t (*pBuf) [PWM_BUFFER_SIZE];
	pBuf = drawBuffer;
	drawBuffer = frameBuffer;
	frameBuffer = pBuf;

	DMA_Cmd(DMA2_Stream6, ENABLE);
	DMA_Cmd(DMA2_Stream3, ENABLE);
	DMA_Cmd(DMA2_Stream2, ENABLE);

	//clear TIM_DMA_CC3 | TIM_DMA_CC1 | TIM_DMA_CC2
	//TIM1->EGR &= ~0x38;
	TIM_Cmd(TIM1 ,DISABLE);

	TIM1->CNT = TIM1->CCR2; // force one last timer overflow
	TIM_DMACmd(TIM1, TIM_DMA_CC3 | TIM_DMA_CC1 | TIM_DMA_CC2, ENABLE);

	TIM_Cmd(TIM1 , ENABLE);
}

int LedBusy()
{
	if (led_update_in_progress >= 1){
		return 1;
	}
	if (UB_Systick_Counter2(COUNTER_CHECK) < LED_RESET_TIME){ //for latch
		return 1;
	}
	return 0;
}

// num == 0 is the upper left LED
// num == 68 is the upper right LED
// num == 68 * 42 is the lower right LED
void LedsetPixel(uint32_t num, uint32_t color)
{
	// there are 3 * 68 Pixels on each pin (3 rows)
	// uc ->->...->-)
	//  (<-<-...<-<-)
	//  (->->...->->

	uint32_t strip, offset, mask;
	uint16_t bit, *p;

	// WS2811 needs the data in GRB rather than in RGB order
	color = ((color<<8)&0xFF0000) | ((color>>8)&0x00FF00) | (color&0x0000FF);

	strip = num / LED_STRIPLEN;
	offset = num % LED_STRIPLEN;
	if ((offset / LED_XRES) == 1){
		offset = LED_XRES * 2 - offset % LED_XRES - 1;
	}
	bit = (1<<strip);
	p = ((uint16_t *)drawBuffer) + offset * 24;
	for (mask = (1<<23); mask; mask >>= 1) {
		if (color & mask) {
			*p++ &= ~bit;
		} else {
			*p++ |= bit;
		}
	}
}


void LedStripPixel(uint8_t strip, uint32_t num, uint32_t color)
{
	uint32_t offset, mask;
	uint16_t bit, *p;

	// WS2811 needs the data in GRB rather than in RGB order
	color = ((color<<8)&0xFF0000) | ((color>>8)&0x00FF00) | (color&0x0000FF);

	offset = num;// % LED_STRIPLEN;

	bit = (1<<strip);
	p = ((uint16_t *)frameBuffer) + offset * 24;
	for (mask = (1<<23); mask; mask >>= 1) {
		if (color & mask) {
			*p++ &= ~bit;
		} else {
			*p++ |= bit;
		}
	}
}

