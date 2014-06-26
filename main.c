#include "stm32f4xx_conf.h"
#include <stm32f4xx.h>
#include <stm32f4xx_rcc.h>
#include <stm32f4xx_gpio.h>
#include <stm32f4xx_tim.h>
#include <stm32f4xx_dma.h>
#include "ws2811.h"

#include "stm32_ub_led.h"
#include "stm32_ub_button.h"
#include "stm32_ub_systick.h"
#include "stm32_ub_irmp.h"
#include "stm32_ub_ee_flash.h"

#include "stm32_ub_usb_cdc.h"

#include "WMath.h"
#include  "rc5.h"
#include <string.h>
#include <stdlib.h>

#include <math.h>

#include "pictures.h"

//#include "led_usb.h"

void draw_Pixel (unsigned int posx, unsigned int posy, unsigned int red, unsigned int green, unsigned int blue);

#define numPixels (LED_XRES*LED_YRES)
#define PLATINENLEDS (15)// Leds auf der Platine
#define NUMPLATINENLEDS (5)// anzahl Leds auf der Platine
#define GAMMAADDR	(0x00)

uint8_t imgData[2][numPixels * 3], // Data for 2 strips worth of imagery
     alphaMask[numPixels],      // Alpha channel for compositing images
     backImgIdx = 0,            // Index of 'back' image (always 0 or 1)
     frontImgIdx = 1,
     fxIdx[3];                  // Effect # for back & front images + alpha
int  fxVars[3][10],             // Effect instance variables (explained later)
     tCounter   = -1,           // Countdown to next transition
     transitionTime;            // Duration (in frames) of current transition

volatile uint8_t	nextEffekt = 0,
					nextAlpha = 0,
					nextTransitionTime = 1,
					transitionAllowed = 0;
int  modiVars[15];
uint8_t T1_render_strip_flag = 0;
uint8_t T1_render_platinen_strip_flag = 0;
uint32_t plCurrPos = 0;
uint8_t T1_ir_flag = 0;
volatile int current_effect_time=0;

uint8_t ir_code = 255;
uint8_t ir_rep_code = 255;


// function prototypes, leave these be :)
void renderEffect00(uint8_t idx);
void renderEffect01(uint8_t idx);
void renderEffect02(uint8_t idx);
void renderEffect03(uint8_t idx);
void renderEffect04(uint8_t idx);
void renderEffect05(uint8_t idx);
void renderEffect06(uint8_t idx);
void renderEffect07(uint8_t idx);
void renderEffect08(uint8_t idx);
void renderEffect09(uint8_t idx);
void renderAlpha00(void);
void renderAlpha01(void);
void renderAlpha02(void);
//void renderAlpha03(void);
void callback();
uint8_t mygamma(uint8_t x);
void GammaTableGen(double_t gammaVal);
long hsv2rgb(long h, uint8_t s, uint8_t v);
int8_t fixSin(int angle);
int8_t fixCos(int angle);

void mode0();
void mode1();
void mode2();
void mode3();
void mode4();
void mode5();
void mode6();
void mode7();
void mode8();
void mode9();

void render_stripe();

void startEffekt(uint8_t effect, uint8_t alpha, uint8_t transition);
void ir_polling();
uint8_t get_ir();

void plHandleEffekt(uint8_t r1, uint8_t g1, uint8_t b1, uint8_t r2, uint8_t g2, uint8_t b2);

// List of image effect and alpha channel rendering functions; the code for
// each of these appears later in this file.  Just a few to start with...
// simply append new ones to the appropriate list here:

void (*renderEffect[])(uint8_t) = {
  renderEffect00,
  renderEffect01,
  renderEffect02,
  renderEffect03,
  renderEffect04,
  renderEffect05,
  renderEffect06,
  renderEffect07,
  renderEffect08,
  renderEffect09},
(*renderAlpha[])(void)  = {
  renderAlpha00,
  renderAlpha01,
  renderAlpha02};

void (*modi[])() = {
		mode0,
		mode1,
		mode2,
		mode3,
		mode4,
		mode5,
		mode6,
		mode7,
		mode8,
		mode9};


int main(void)
{
	//INIT effekte
	fxIdx[frontImgIdx] = 4;
	uint8_t modi_aktuell = 7;
	double_t gammaVal;// = 2;
	fxIdx[backImgIdx] = 8;
	ErrorStatus eepromCheck;

	USB_CDC_STATUS_t lastStatus = UB_USB_CDC_GetStatus();

	SystemInit();
	UB_Led_Init();
	UB_Button_Init();

	// init EEProm, GammaTable
	// power down while flashing is bad (loose power connection...))
	eepromCheck=UB_EE_FLASH_Init();
	if(eepromCheck==SUCCESS && UB_Button_OnClick(BTN_USER)==false &&
			(double_t)UB_EE_FLASH_Read(GAMMAADDR) >= 1 && (double_t)UB_EE_FLASH_Read(GAMMAADDR) <=5) {// BUTTon for reset
		gammaVal = (double_t)UB_EE_FLASH_Read(GAMMAADDR);
	}else{
		gammaVal = 2;
	}
	GammaTableGen(gammaVal);

	UB_Systick_Init();
	Ws2811_init();
	UB_IRMP_Init();
	UB_USB_CDC_Init();
	lastStatus = UB_USB_CDC_GetStatus();
///////////////// INIT ENDE //////////////////////////////////

	while (1) {
		// IR-Daten pollen
		IRMP_DATA  myIRData;
		if (GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_11) == 1){//DEBUG
			UB_Led_On(LED_ORANGE);
		} else {
			UB_Led_Off(LED_ORANGE);
		}
		if((UB_IRMP_Read(&myIRData)==TRUE)){	//PE11
			//UB_Led_Toggle(LED_ORANGE);
			ir_rep_code = myIRData.command;
			if (!(myIRData.flags & IRMP_FLAG_REPETITION)){
				ir_code = myIRData.command;
			} else{
				ir_code = 255;
			}
		} else {
			ir_code = 255;
			ir_rep_code = 255;
		}


		//Tastenabfrage Anfang
		if(ir_code == 0){
			modiVars[0] = 0;
			modi_aktuell = 0;
		}else if(ir_code == 1){
			modiVars[0] = 0;
			modi_aktuell = 1;
		}else if(ir_code == 2){
			modiVars[0] = 0;
			modi_aktuell = 2;
		}else if(ir_code == 3){
			modiVars[0] = 0;
			modi_aktuell = 4;
		}else if(ir_code == 4){
			modiVars[0] = 0;
			modi_aktuell = 5;
		}else if(ir_code == 5){
			modiVars[0] = 0;
			modi_aktuell = 6;
		}else if(ir_code == 6){
			modiVars[0] = 0;
			modi_aktuell = 7;
		}else if(ir_code == 7){
			modiVars[0] = 0;
			modi_aktuell = 8;
		}else if(ir_code == 8){
			modiVars[0] = 0;
			modi_aktuell = 9;
		}else if(ir_code == BUTTON_POWER){
			modiVars[0] = 0;
			modi_aktuell = 3;
		}else if(ir_rep_code == BUTTON_MENUE || ir_rep_code == BUTTON_RADIO || ir_rep_code == BUTTON_THREE_LINES) {
			if(ir_rep_code == BUTTON_MENUE) {
				if(gammaVal - 0.1 > 1){
					gammaVal -= 0.1;
				}else{
					gammaVal = 1;
				}
			}else if(ir_code == BUTTON_RADIO) {
						gammaVal = 2;
			}else if(ir_rep_code == BUTTON_THREE_LINES) {
				if(gammaVal + 0.1 <= 5){
					gammaVal += 0.1;
				}
			}

			if(gammaVal != UB_EE_FLASH_Read(GAMMAADDR)) {
				// (UB_EE_FLASH_Write has delays which interfere with interrupts...)
				do {
					GammaTableGen(gammaVal);
				} while (led_update_in_progress);
				UB_EE_FLASH_Write(GAMMAADDR, (int32_t)gammaVal);
			} else {
				GammaTableGen(gammaVal);
			}
		}
		//Tastenabfrage ENDE


		if(UB_USB_CDC_GetStatus() != lastStatus) {
			lastStatus = UB_USB_CDC_GetStatus();
			if (UB_USB_CDC_GetStatus() == USB_CDC_CONNECTED) {
				if (modi_aktuell != 0) {
					modiVars[0] = 0;
					modi_aktuell = 0;
				}
			} else if (UB_USB_CDC_GetStatus() != USB_CDC_CONNECTED) {
				if (modi_aktuell != 7) {
					modiVars[0] = 0;
					modi_aktuell = 7;
				}
			}
		}

		plHandleEffekt(255, 0, 0, 0, 0,0);//TODO render_sripe()^^
		//Modi werden ausgeführt
		(*modi[modi_aktuell])();

		//Renderei
		if(modi_aktuell != 0){// Ausnahme für USB stream
			if (T1_render_strip_flag == 1){
				T1_render_strip_flag = 0;
				render_stripe();
				UB_Led_Toggle(LED_BLUE);
			}
		}

		if(UB_Systick_Timer1(TIMER_CHECK,0)==TIMER_HOLD) {

			UB_Led_Toggle(LED_GREEN);

			T1_render_strip_flag = 1;
			T1_render_platinen_strip_flag = 1;

			UB_Systick_Timer1(TIMER_START_ms, 16);// 60 = 16 Frames
		}
	}
}

void plHandleEffekt(uint8_t r1, uint8_t g1, uint8_t b1, uint8_t r2, uint8_t g2, uint8_t b2)
{
	if(T1_render_platinen_strip_flag){
		T1_render_platinen_strip_flag = 0;

	  //if(fxVars[idx][0] == 0) { // Initialize effect?
		//fxVars[idx][1] = random(1536); // Random hue
		const int PLCOLOR = 200;//1280//200//768
		// Number of repetitions (complete loops around color wheel);
		// any more than 4 per meter just looks too chaotic.			 (jetzt 0.25)
		// Store as distance around complete belt in half-degree units:
		//fxVars[idx][2] = (1 + (0.25 * ((NUMPLATINENLEDS + 31) / 32))) * 720;
		//((1 + (0.025 * ((NUMPLATINENLEDS + NUMPLATINENLEDS - 1) / NUMPLATINENLEDS))) * 720)
		const int PLNUMREP = 180/2;
		// Frame-to-frame increment (speed) -- may be positive or negative,
		// but magnitude shouldn't be so small as to be boring.  It's generally
		// still less than a full pixel per frame, making motion very smooth.
		//fxVars[idx][3] = 4 + 0.015 * random((fxVars[idx][1]) / NUMPLATINENLEDS);//mal 0.125
		const int PLINCRE = (2);
		// Reverse direction half the time.
		//if(random(2) == 0) fxVars[idx][3] = -fxVars[idx][3];
		//fxVars[idx][4] = 0; // Current position
		//fxVars[idx][0] = 1; // Effect initialized
	  //}

		int32_t  foo;
		uint32_t color;

		for(long i=0; i<NUMPLATINENLEDS; i++) {
			foo = fixSin((plCurrPos + PLNUMREP * i) % 720);//fixSin(300 + PLNUMREP * i);

			color = (foo >= 0) ?
			   hsv2rgb(PLCOLOR, 254 - (foo * 2), 255) :
			   hsv2rgb(PLCOLOR, 255, 254 + foo * 2);

			LedStripPixel(PLATINENLEDS, i, color);
			//LedStripPixel(14, i, 0xFFFFFF);
		}
		plCurrPos += PLINCRE;
		//render_stripe();//DEBUG
	}
}

#ifdef  USE_FULL_ASSERT
void assert_failed(uint8_t* file, uint32_t line)
{
	while(1)
	{
		;//TODO
	}
}
#endif /* __STM32F4xx_CONF_H */

void draw_Pixel (unsigned int posx, unsigned int posy, unsigned int red, unsigned int green, unsigned int blue)
{
	uint32_t color = 0;
	color |= red << 16;
	color |= green << 8;
	color |= blue;

	LedsetPixel(LED_XRES * posx + posy, color);
}

void render_stripe() {
  // Very first thing here is to issue the strip data generated from the
  // *previous* callback.  It's done this way on purpose because show() is
  // roughly constant-time, so the refresh will always occur on a uniform
  // beat with respect to the Timer1 interrupt.  The various effects
  // rendering and compositing code is not constant-time, and that
  // unevenness would be apparent if show() were called at the end.

  LedShow();

  frontImgIdx = 1 - backImgIdx;



	// Count up to next transition (or end of current one):
	tCounter++;
	if(transitionAllowed){
		transitionAllowed = 0;

		if(tCounter <= 0){
			// Transition start
			fxIdx[2]           = nextAlpha;
			transitionTime     = nextTransitionTime;
			fxVars[2][0]       = 0; // Transition not yet initialized
		}
		fxIdx[frontImgIdx] = nextEffekt;
		fxVars[frontImgIdx][0] = 0; // Effect not yet initialized
		current_effect_time = 0;
	} else if(tCounter == 0) {
		tCounter          = -1;
		current_effect_time++;
	} else if(tCounter >= transitionTime) { // End transition
		fxIdx[backImgIdx] = fxIdx[frontImgIdx]; // Move front effect index to back
		backImgIdx        = 1 - backImgIdx;     // Invert back index
		frontImgIdx = 1 - backImgIdx;// TODO DEBUG steht schon oben
		tCounter          = -1;
	}
	  uint8_t  *backPtr    = &imgData[backImgIdx][0],
	       r, g, b;
	  int  i;

  // Always render back image based on current effect index:
  (*renderEffect[fxIdx[backImgIdx]])(backImgIdx);

  // Front render and composite only happen during transitions...
  if(tCounter > 0) {
		// Transition in progress
		uint8_t *frontPtr = &imgData[frontImgIdx][0];
		int  alpha, inv;

		// Render front image and alpha mask based on current effect indices...
		(*renderEffect[fxIdx[frontImgIdx]])(frontImgIdx);
		(*renderAlpha[fxIdx[2]])();

		// ...then composite front over back:
		for(i=0; i<numPixels; i++) {
			alpha = alphaMask[i] + 1; // 1-256 (allows shift rather than divide)
			inv   = 257 - alpha;      // 1-256 (ditto)
			// r, g, b are placed in variables (rather than directly in the
			// setPixelColor parameter list) because of the postincrement pointer
			// operations -- C/C++ leaves parameter evaluation order up to the
			// implementation; left-to-right order isn't guaranteed.
			r = mygamma((*frontPtr++ * alpha + *backPtr++ * inv) >> 8);
			g = mygamma((*frontPtr++ * alpha + *backPtr++ * inv) >> 8);
			b = mygamma((*frontPtr++ * alpha + *backPtr++ * inv) >> 8);
			//setPixelColor(i, r, g, b);
			LedsetPixel(i, r << 16 | g << 8 | b);
		}
	} else {
		// No transition in progress; just show back image
		for(i=0; i<numPixels; i++) {
			  // See note above re: r, g, b vars.
			  r = mygamma(*backPtr++);
			  g = mygamma(*backPtr++);
			  b = mygamma(*backPtr++);
			  //setPixelColor(i, r, g, b);
			  LedsetPixel(i, r << 16 | g << 8 | b);
			}
	}
}

// ---------------------------------------------------------------------------
//Eigene Funktionen
void startEffekt(uint8_t effect, uint8_t alpha, uint8_t transition)
{
	nextEffekt = effect;
	nextAlpha = alpha;
	nextTransitionTime = transition;
	transitionAllowed = 1;
}

// ---------------------------------------------------------------------------
//Modi:

//Befehle über USART empfangen
void mode0()
{
	if(modiVars[0] == 0) {
		modiVars[1] = 0;//num_received
		modiVars[2] = 0;//rgb
		modiVars[3] = 0;//color
		//chooseGamma = 1;// Gamma Table 1
		modiVars[0] = 1;
	}

	while (1) {
		usb_packet_t *packet = usb_cdc_rx();
		if (!packet) {
			break;
		}

		//uint32_t *buffer = (uint32_t*) packet->buf;

		for (int i = 0; i < packet->len; i++) {
			if (packet->buf[i] == 1 || modiVars[1] > FRAMEBUFFER_SIZE){
				modiVars[3] = 0;
				modiVars[2] = 0;
				modiVars[1] = 0;
				LedShow();
				UB_Led_Toggle(LED_BLUE);
			} else {
				if(modiVars[2] == 0){//21|22
					modiVars[3] |= mygamma((uint8_t)packet->buf[i]) << 16;
					modiVars[2]++;
				} else if (modiVars[2] == 1) {
					modiVars[3] |= mygamma((uint8_t)packet->buf[i]) << 8;
					modiVars[2]++;
				} else if (modiVars[2] == 2) {
					modiVars[3] |= mygamma((uint8_t)packet->buf[i]);

					LedsetPixel(modiVars[1], modiVars[3]);
					modiVars[3] = 0;
					modiVars[2] = 0;
					modiVars[1]+=1;
					if(modiVars[1] > FRAMEBUFFER_SIZE){
						modiVars[1] = 0;
						LedShow();
						UB_Led_Toggle(LED_BLUE);
					}
				}
			}
		}
		usb_free(packet);
	}
}

//zufall
void mode1()
{
	if(modiVars[0] == 0) {
			modiVars[0] = 1;//initialized
			modiVars[1] = 120 + random(240);// intervall zwischen effekten
			modiVars[2] = 1;// effekt manuel
			modiVars[3] = 1;// bool automatisch
			startEffekt(random(4), random(3), random_maxmin(20, 100));
	}else{
		if(current_effect_time > modiVars[1] && modiVars[3] == 1){
			modiVars[2] = random(4);
			startEffekt(modiVars[2], random(3), random_maxmin(20, 100));
		}
		if(ir_rep_code == BUTTON_VOL_PLUS){
			if(modiVars[1] <= 500){
				modiVars[1] += 10;
			}
		}else if(ir_rep_code == BUTTON_VOL_MINUS){
			if(modiVars[1] >= 10){
				modiVars[1] -= 10;
			}
		}else if(ir_code == BUTTON_PLAYPAUSE){//
			modiVars[3] = 1;
		}else if(ir_code == BUTTON_STOP2){//
			modiVars[3] = 0;
		}else if(ir_code == BUTTON_MINUS){
			if(modiVars[2] >= 2){//effekt0 is boring
				modiVars[2] -= 1;
				startEffekt(modiVars[2], 1, 100);
			}
		}else if(ir_code == BUTTON_PLUS){
			if(modiVars[2] <= 2){
				modiVars[2] += 1;
				startEffekt(modiVars[2], 1, 100);
			}
		}
	}
}

//Farbe wählen
void mode2()
{
	//uint8_t idx = modiVars[1] = frontImgIdx;// bind to renderEffect instance;
	long color;
	uint8_t hsv_renderflag = 0;
	if(modiVars[0] == 0) {
		modiVars[1] = frontImgIdx;// bind to renderEffect instance
		//for renderEffect04
		//fxVars[idx][1];//R
		//fxVars[idx][2];//G
		//fxVars[idx][3];//B
		modiVars[2] = 0;
		modiVars[3] = 0;
		modiVars[4] = 0;
		modiVars[5] = 2;//farbe zum modifizieren
		modiVars[6] = 0;//HSV wert
		modiVars[7] = 100;//255;//Helligkeit
		modiVars[8] = 255;//Sättigung
		modiVars[0] = 1;//initialized

		//startEffekt(4, 1, 100);
		hsv_renderflag = 1;
	}
	if(ir_code == BUTTON_ROT){
		modiVars[5] = 2;//R
	}else if(ir_code == BUTTON_GRUEN){
		modiVars[5] = 3;//G
	}else if(ir_code == BUTTON_BLAU){
		modiVars[5] = 4;//B
	}else if(ir_code == BUTTON_GELB){
		modiVars[5] = 7;//Helligkeit
	}else if(ir_code == BUTTON_IPLUS){
		modiVars[5] = 8;//Sättigung
	}else{
		if(ir_rep_code == BUTTON_VOL_PLUS || ir_rep_code == BUTTON_VOL_MINUS){
			if(ir_rep_code == BUTTON_VOL_PLUS){
				if(modiVars[modiVars[5]] <= 250){
					modiVars[modiVars[5]] += 5;
				}else{
					modiVars[modiVars[5]] = 255;
				}
			}else{
				if(modiVars[modiVars[5]] >= 5){
					modiVars[modiVars[5]] -= 5;
				}else{
					modiVars[modiVars[5]] = 0;
				}
			}
			if(modiVars[5] == 7 || modiVars[5] == 8){
				hsv_renderflag = 1;
			}else{
				modiVars[1] = frontImgIdx;// bind to renderEffect instance
				fxVars[modiVars[1]][1] = modiVars[2];//R
				fxVars[modiVars[1]][2] = modiVars[3];//G
				fxVars[modiVars[1]][3] = modiVars[4];//B

				startEffekt(4, 0, 10);
			}
		}else if(ir_code == BUTTON_RECHTS){
			modiVars[6] += 100;
			modiVars[6] %= 1536;
			hsv_renderflag = 1;
		}else if(ir_code == BUTTON_LINKS){
			modiVars[6] -= 100;
			modiVars[6] %= 1536;
			hsv_renderflag = 1;
		}
	}
	if(hsv_renderflag){
		hsv_renderflag = 0;

		modiVars[1] = frontImgIdx;// bind to renderEffect instance
		color = hsv2rgb(modiVars[6], modiVars[8], modiVars[7]);
		fxVars[ modiVars[1]][1] = (uint8_t)(color >> 16);
		fxVars[ modiVars[1]][2] = (uint8_t)(color >> 8);
		fxVars[ modiVars[1]][3] = (uint8_t)color;
		modiVars[2] = fxVars[ modiVars[1]][1];//R
		modiVars[3] = fxVars[ modiVars[1]][2];//G
		modiVars[4] = fxVars[ modiVars[1]][3];//B
		startEffekt(4, 0, 20);
	}
}

//Power off
void mode3()
{
	if(modiVars[0] == 0) {
		modiVars[0] = 1;

		startEffekt(8, 1, 100);
	}
}
//Fire
void mode4()
{
	if(modiVars[0] == 0) {
		modiVars[0] = 1;

		startEffekt(5, 1, 100);
	}
}
// Regebogen//Lauflicht
void mode5()
{
	if(modiVars[0] == 0) {
		modiVars[0] = 1;

		startEffekt(1, 1, 100);//startEffekt(6, 1, 100);
	}
}

void mode6()
{
	if(modiVars[0] == 0) {
		modiVars[0] = 1;

		startEffekt(2, 1, 100);
	}
}

void mode7() //display BMP
{
	const GUI_BITMAP *pictureArray[] = {&bmATN_Logo, &bmATN_Rainbow, &bmFZero, &bmich, &bmLinkbig,
										&bmMario, &bmstrawhat, &bmZorro, &ATN_Random, &bmbio};

	if(modiVars[0] == 0) {
		modiVars[1] = frontImgIdx;// bind to renderEffect instance
		modiVars[2] = 0; // number of picture which gets displayed
		//for renderEffect07()
		fxVars[modiVars[1]][1] = (int)pictureArray[modiVars[2]];// gets displayed by renderEffect07

		startEffekt(7, 1, 100);

		modiVars[0] = 1;
	}
	if (ir_code == BUTTON_RECHTS || ir_code == BUTTON_LINKS) {
		modiVars[1] = frontImgIdx;
		if(ir_code == BUTTON_RECHTS){
			if (modiVars[2] + 1 < sizeof(pictureArray) / sizeof(void*)) {
				modiVars[2] += 1;
			}
		}else if(ir_code == BUTTON_LINKS){
			if (modiVars[2] - 1 >= 0) {
				modiVars[2] -= 1;
			}
		}
//		if(modiVars[2] == 0){
//			modiVars[1] = (int) &bmblack;
//		}else if(modiVars[2] == 1){
//			modiVars[1] = (int) &bmregenbogen1;
//		}else if(modiVars[2] == 2){
//			modiVars[1] = (int) &bmRGB;
//		}
		fxVars[modiVars[1]][1] = (int)pictureArray[modiVars[2]];
		startEffekt(7, 1, 100);
	}
}

// display Gif
void mode8()
{
	if(modiVars[0] == 0) {
		modiVars[1] = frontImgIdx;// bind to renderEffect instance
		modiVars[2] = 0; // number of gif which gets displayed
		modiVars[3] = 5;// speed
		// for renderEffect09()
		fxVars[modiVars[1]][3] = (int)apbmmario2;// gets displayed by renderEffect07
		fxVars[modiVars[1]][4] = sizeof(apbmmario2) / sizeof(GUI_BITMAP*);//48;// frames
		fxVars[modiVars[1]][5] = modiVars[3];// speed
		fxVars[modiVars[1]][6] = 1;// loop

		startEffekt(9, 1, 100);

		modiVars[0] = 1;
	}

	if (ir_code == BUTTON_RECHTS || ir_code == BUTTON_LINKS ) {
		modiVars[1] = frontImgIdx;// new effect, new Idx
		if(ir_code == BUTTON_RECHTS){
			if (modiVars[2] + 1 <= ANZAHL_GIFS - 1) {
				modiVars[2] += 1;
			}
		}else if(ir_code == BUTTON_LINKS){
			if (modiVars[2] - 1 >= 0) {
				modiVars[2] -= 1;
			}
		}
		if(modiVars[2] == 0){
			fxVars[modiVars[1]][3] = (int) apbmmario2;
			fxVars[modiVars[1]][4] = sizeof(apbmmario2) / sizeof(GUI_BITMAP*);
			modiVars[3] = (aDelaymario2[0] * 0.06) + 0,5;// speed
		}else if(modiVars[2] == 1){
			fxVars[modiVars[1]][3] = (int) apbmsonic;
			fxVars[modiVars[1]][4] = sizeof(apbmsonic) / sizeof(GUI_BITMAP*);
			modiVars[3] = (aDelaysonic[0] * 0.06) + 0,5;// speed
		}else if(modiVars[2] == 2){
			fxVars[modiVars[1]][3] = (int) apbmtunnel;
			fxVars[modiVars[1]][4] = sizeof(apbmtunnel) / sizeof(GUI_BITMAP*);
			modiVars[3] = (aDelaytunnel[0] * 0.06) + 0,5;// speed
		}else if(modiVars[2] == 3){
			fxVars[modiVars[1]][3] = (int) apbmBlueFalcon2;
			fxVars[modiVars[1]][4] = sizeof(apbmBlueFalcon2) / sizeof(GUI_BITMAP*);
			modiVars[3] = (aDelayBlueFalcon2[0] * 0.06) + 0,5;// speed
		}else if(modiVars[2] == 4){
			fxVars[modiVars[1]][3] = (int) apbmMarioAtack2;
			fxVars[modiVars[1]][4] = sizeof(apbmMarioAtack2) / sizeof(GUI_BITMAP*);
			modiVars[3] = (aDelayMarioAtack2[0] * 0.06) + 0,5;// speed
		}else if(modiVars[2] == 5){
			fxVars[modiVars[1]][3] = (int) apbmaxterix;
			fxVars[modiVars[1]][4] = sizeof(apbmaxterix) / sizeof(GUI_BITMAP*);
			modiVars[3] = (aDelayaxterix[0] * 0.06) + 0,5;// speed
		}else if(modiVars[2] == 6){
			fxVars[modiVars[1]][3] = (int) apbmavatar;
			fxVars[modiVars[1]][4] = sizeof(apbmavatar) / sizeof(GUI_BITMAP*);
			modiVars[3] = (aDelayavatar[0] * 0.06) + 0,5;// speed
		}else if(modiVars[2] == 7){
			fxVars[modiVars[1]][3] = (int) apbmRayman;
			fxVars[modiVars[1]][4] = sizeof(apbmRayman) / sizeof(GUI_BITMAP*);
			modiVars[3] = (aDelayRayman[0] * 0.06) + 0,5;// speed
		}else if(modiVars[2] == 8){
			fxVars[modiVars[1]][3] = (int) apbmATN_rotate;
			fxVars[modiVars[1]][4] = sizeof(apbmATN_rotate) / sizeof(GUI_BITMAP*);
			modiVars[3] = (aDelayATN_rotate[0] * 0.06) + 0,5;// speed
		}

		fxVars[modiVars[1]][6] = 1;// loop
		fxVars[modiVars[1]][5] = modiVars[3];// speed
		startEffekt(9, 1, 100);
	}else if(ir_rep_code == BUTTON_VOL_PLUS || ir_rep_code == BUTTON_VOL_MINUS){
		if(ir_code == BUTTON_VOL_PLUS){
			if (modiVars[3] + 1 <= 30) {
				modiVars[3] += 1;
			}
		}else if(ir_code == BUTTON_VOL_MINUS){
			if (modiVars[3] - 1 >= 0) {
				modiVars[3] -= 1;
			}
		}
		fxVars[modiVars[1]][5] = modiVars[3];// speed
	}
}

// Präsentation
void mode9()
{
	const GUI_BITMAP *pictureArray[] = {&bmATN_Logo, &bmFZero, &bmich, &bmLinkbig,
															&bmMario, &bmstrawhat, &bmZorro};

	if(modiVars[0] == 0) {
		modiVars[1] = frontImgIdx;// bind to renderEffect instance
		modiVars[2] = 0; // number of gif which gets displayed
		modiVars[3] = (aDelaymario2[0] * 0.06) + 0,5;// speed
		// for renderEffect07()
		fxVars[modiVars[1]][1] = (int)pictureArray[modiVars[2]];// gets displayed by renderEffect07
		// for renderEffect09()
		fxVars[modiVars[1]][3] = (int)apbmmario2;// gets displayed
		fxVars[modiVars[1]][4] = sizeof(apbmmario2) / sizeof(GUI_BITMAP*);//48;// frames
		fxVars[modiVars[1]][5] = modiVars[3];// speed
		fxVars[modiVars[1]][6] = 1;// loop

		//startEffekt(9, 1, 100);
		startEffekt(7, 1, 100);

		modiVars[0] = 1;
	}

	if (ir_code == BUTTON_RECHTS || ir_code == BUTTON_LINKS ) {
		modiVars[1] = frontImgIdx;// new effect, new Idx
		if(ir_code == BUTTON_RECHTS){
			if (modiVars[2] + 1 <= 9) {
				modiVars[2] += 1;
			}
		}else if(ir_code == BUTTON_LINKS){
			if (modiVars[2] - 1 >= 0) {
				modiVars[2] -= 1;
			}
		}
		if(modiVars[2] == 0){
			fxVars[modiVars[1]][1] = (int)&bmATN_Logo;

			startEffekt(7, 1, 100);
		}else if(modiVars[2] == 1){
			fxVars[modiVars[1]][1] = (int)&bmATN_Rainbow;

			startEffekt(7, 1, 100);
		}else if(modiVars[2] == 2){
			fxVars[modiVars[1]][3] = (int) apbmATN_rotate;
			fxVars[modiVars[1]][4] = sizeof(apbmATN_rotate) / sizeof(GUI_BITMAP*);
			modiVars[3] = (aDelayATN_rotate[0] * 0.06) + 0,5;// speed
			fxVars[modiVars[1]][6] = 0;// loop

			fxVars[modiVars[1]][5] = modiVars[3];// speed
			startEffekt(9, 1, 0);
		}else if(modiVars[2] == 3){
			fxVars[modiVars[1]][1] = (int)&bmATN_Rainbow;

			startEffekt(7, 1, 100);
		}else if(modiVars[2] == 4){
			fxVars[modiVars[1]][1] = (int)&bmstm32;

			startEffekt(7, 1, 100);
		}else if(modiVars[2] == 5){
			fxVars[modiVars[1]][1] = (int)&bmstm32F4;

			startEffekt(7, 1, 100);
		}else if(modiVars[2] == 6){
			fxVars[modiVars[1]][1] = (int)&bmGAMMA;

			startEffekt(7, 1, 100);
		}else if(modiVars[2] == 7){
			fxVars[modiVars[1]][1] = (int)&bmLinkbig;

			startEffekt(7, 1, 100);
		}else if(modiVars[2] == 8){
			fxVars[modiVars[1]][3] = (int) apbmsonic;
			fxVars[modiVars[1]][4] = sizeof(apbmsonic) / sizeof(GUI_BITMAP*);
			modiVars[3] = (aDelaysonic[0] * 0.06) + 0,5;// speed
			fxVars[modiVars[1]][6] = 1;// loop

			fxVars[modiVars[1]][5] = modiVars[3];// speed
			startEffekt(9, 1, 100);
		}else if(modiVars[2] == 9){
			fxVars[modiVars[1]][1] = (int)&bmUSB;

			startEffekt(7, 1, 100);
		}
	}else if(ir_rep_code == BUTTON_VOL_PLUS || ir_rep_code == BUTTON_VOL_MINUS){
		if(ir_code == BUTTON_VOL_PLUS){
			if (modiVars[3] + 1 <= 30) {
				modiVars[3] += 1;
			}
		}else if(ir_code == BUTTON_VOL_MINUS){
			if (modiVars[3] - 1 >= 0) {
				modiVars[3] -= 1;
			}
		}
		fxVars[modiVars[1]][5] = modiVars[3];// speed
	}
}

// ---------------------------------------------------------------------------
// Image effect rendering functions.  Each effect is generated parametrically
// (that is, from a set of numbers, usually randomly seeded).  Because both
// back and front images may be rendering the same effect at the same time
// (but with different parameters), a distinct block of parameter memory is
// required for each image.  The 'fxVars' array is a two-dimensional array
// of integers, where the major axis is either 0 or 1 to represent the two
// images, while the minor axis holds 50 elements -- this is working scratch
// space for the effect code to preserve its "state."  The meaning of each
// element is generally unique to each rendering effect, but the first element
// is most often used as a flag indicating whether the effect parameters have
// been initialized yet.  When the back/front image indexes swap at the end of
// each transition, the corresponding set of fxVars, being keyed to the same
// indexes, are automatically carried with them.

// Simplest rendering effect: fill entire image with solid color
void renderEffect00(uint8_t idx) {
  // Only needs to be rendered once, when effect is initialized:
  if(fxVars[idx][0] == 0) {
    uint8_t *ptr = &imgData[idx][0],
      r = random(256), g = random(256), b = random(256);
    for(int i=0; i<numPixels; i++) {
      *ptr++ = r; *ptr++ = g; *ptr++ = b;
    }
    fxVars[idx][0] = 1; // Effect initialized
  }
}

// Rainbow effect (1 or more full loops of color wheel at 100% saturation).
// Not a big fan of this pattern (it's way overused with LED stuff), but it's
// practically part of the Geneva Convention by now.
void renderEffect01(uint8_t idx) {
  if(fxVars[idx][0] == 0) { // Initialize effect?
    // Number of repetitions (complete loops around color wheel); any
    // more than 4 per meter just looks too chaotic and un-rainbow-like. (jetzt 0.025)
    // Store as hue 'distance' around complete belt:
    fxVars[idx][1] = (1 + random(0.025 * ((numPixels + 31) / 32))) * 1536;
    // Frame-to-frame hue increment (speed) -- may be positive or negative,
    // but magnitude shouldn't be so small as to be boring.  It's generally
    // still less than a full pixel per frame, making motion very smooth.
    fxVars[idx][2] = 4 + 0.125 * (random(fxVars[idx][1]) / numPixels);//*0.125
    // Reverse speed and hue shift direction half the time.
    if(random(2) == 0) fxVars[idx][1] = -fxVars[idx][1];
    if(random(2) == 0) fxVars[idx][2] = -fxVars[idx][2];
    fxVars[idx][3] = 0; // Current position
    fxVars[idx][0] = 1; // Effect initialized
  }

  uint8_t *ptr = &imgData[idx][0];
  long color, i;
  for(i=0; i<numPixels; i++) {
    color = hsv2rgb(fxVars[idx][3] + fxVars[idx][1] * i / numPixels,
      255, 255);
    *ptr++ = color >> 16; *ptr++ = color >> 8; *ptr++ = color;
  }
  fxVars[idx][3] += fxVars[idx][2];
}

// Sine wave chase effect
void renderEffect02(uint8_t idx) {
  if(fxVars[idx][0] == 0) { // Initialize effect?
    fxVars[idx][1] = random(1536); // Random hue
    // Number of repetitions (complete loops around color wheel);
    // any more than 4 per meter just looks too chaotic.			 (jetzt 0.25)
    // Store as distance around complete belt in half-degree units:
    //fxVars[idx][2] = (1 + random(0.25 * ((numPixels + 31) / 32))) * 720;
    fxVars[idx][2] = (1 + 1 * ((numPixels + 67) / 68.0)) * 720;
    // Frame-to-frame increment (speed) -- may be positive or negative,
    // but magnitude shouldn't be so small as to be boring.  It's generally
    // still less than a full pixel per frame, making motion very smooth.
    fxVars[idx][3] = 4 + 0.015 * random((fxVars[idx][1]) / numPixels);//mal 0.125
    // Reverse direction half the time.
    if(random(2) == 0) fxVars[idx][3] = -fxVars[idx][3];
    fxVars[idx][4] = 0; // Current position
    fxVars[idx][0] = 1; // Effect initialized
  }

  uint8_t *ptr = &imgData[idx][0];
  int  foo;
  long color;
  for(long i=0; i<numPixels; i++) {
    foo = fixSin(fxVars[idx][4] + fxVars[idx][2] * i / numPixels);
    // Peaks of sine wave are white, troughs are black, mid-range
    // values are pure hue (100% saturated).
    color = (foo >= 0) ?
       hsv2rgb(fxVars[idx][1], 254 - (foo * 2), 255) :
       hsv2rgb(fxVars[idx][1], 255, 254 + foo * 2);
    *ptr++ = color >> 16; *ptr++ = color >> 8; *ptr++ = color;
  }
  fxVars[idx][4] += fxVars[idx][3];
}

// Picture: Brown White brown... (20 pixels)
// This gets "stretched" as needed
// to the full LED strip length in the flag effect code, below.
// Can change this data to the colors of the picture,
#define C_RED   200,   15,   0
#define C_WHITE 255, 255, 255
#define C_BLUE    0,   0, 100
unsigned char flagTable[]  = {
  C_RED , C_WHITE, C_RED , C_WHITE, C_RED , C_WHITE, C_RED,
  C_WHITE, C_RED  , C_WHITE, C_RED,  C_RED  , C_WHITE, C_RED ,
  C_WHITE, C_RED  , C_WHITE, C_RED  , C_WHITE, C_RED };

// Wavy flag effect
void renderEffect03(uint8_t idx) {
  long i, sum, s, x;
  int  idx1, idx2, a, b;
  if(fxVars[idx][0] == 0) { // Initialize effect?
    fxVars[idx][1] = 720 + random(720); // Wavyness
    fxVars[idx][2] = 1 + random(5);    // Wave speed
    fxVars[idx][3] = 200 + random(200); // Wave 'puckeryness'
    fxVars[idx][4] = 0;                 // Current  position
    fxVars[idx][0] = 1;                 // Effect initialized
  }
  for(sum=0, i=0; i<numPixels-1; i++) {
    sum += fxVars[idx][3] + fixCos(fxVars[idx][4] + fxVars[idx][1] *
      i / numPixels);
  }

  uint8_t *ptr = &imgData[idx][0];
  for(s=0, i=0; i<numPixels; i++) {
    x = 256L * ((sizeof(flagTable) / 3) - 1) * s / sum;
    idx1 =  (x >> 8)      * 3;
    idx2 = ((x >> 8) + 1) * 3;
    b    = (x & 255) + 1;
    a    = 257 - b;
    *ptr++ = ((flagTable[idx1    ] * a) +
              (flagTable[idx2    ] * b)) >> 8;
    *ptr++ = ((flagTable[idx1 + 1] * a) +
              (flagTable[idx2 + 1] * b)) >> 8;
    *ptr++ = ((flagTable[idx1 + 2] * a) +
              (flagTable[idx2 + 2] * b)) >> 8;
    s += fxVars[idx][3] + fixCos(fxVars[idx][4] + fxVars[idx][1] *
      i / numPixels);
  }

  fxVars[idx][4] += fxVars[idx][2];
  if(fxVars[idx][4] >= 720) fxVars[idx][4] -= 720;
}
//bestimmte Farbe (mode2)
void renderEffect04(uint8_t idx){
	if(fxVars[idx][0] == 0) { // Initialize effect?
		//from mode2()
		//fxVars[idx][1];//R
		//fxVars[idx][2];//G
		//fxVars[idx][3];//B

		uint8_t *ptr = &imgData[idx][0];
		for(int i=0; i<numPixels; i++) {
			*ptr++ =  fxVars[idx][1]; *ptr++ = fxVars[idx][2]; *ptr++ = fxVars[idx][3];
		}
		fxVars[idx][0] = 1; // Effect initialized
	}
}

//fire effekt mode3
void renderEffect05(uint8_t idx){
	if(fxVars[idx][0] == 0) { // Initialize effect?
	fxVars[idx][1] = 0;//zaehler
	fxVars[idx][0] = 1; // Effect initialized
	}
	if(++fxVars[idx][1] > random_maxmin(8,12)){
		fxVars[idx][1] = 0;

		uint8_t *ptr = &imgData[idx][0];
		uint8_t r = 0;
		uint8_t g = 0;
		uint8_t b = 0;

		for(int i=0; i<numPixels; i++) {
			r = random_maxmin(200,255);
			g = random_maxmin(100,120);
			*ptr++ =  r; *ptr++ = g; *ptr++ = b;
		}
	}
}
//Lauflicht mode5
void renderEffect06(uint8_t idx){
	if(fxVars[idx][0] == 0) { // Initialize effect?
	//fxVars[idx][1] = 0;//i
	//fxVars[idx][2] = 0;//j
	fxVars[idx][3] = 0 ;//pos
	fxVars[idx][4] = 1;//dir
	fxVars[idx][0] = 1; // Effect initialized
	}

	uint8_t *ptr = &imgData[idx][0];
	uint8_t r = 0;
	uint8_t g = 0;
	uint8_t b = 0;

	for(int i=0; i<numPixels; i++) {
		if (i == fxVars[idx][3]){
			r = 255;
			g = 255;
			b = 255;
		}
		else{
			r = 0;
			g = 0;
			b = 0;
		}
		*ptr++ =  r; *ptr++ = g; *ptr++ = b;
	}
	if(fxVars[idx][3] < numPixels)
		fxVars[idx][3]++;
	else
		fxVars[idx][0] = 0;

}

// display picture mode7()
void renderEffect07(uint8_t idx)
{
	if(fxVars[idx][0] == 0) {
		uint8_t *ptr = &imgData[idx][0];
		//from mode7()
		//fxVars[idx][1];// pointer to picture
		drawBitmap(ptr, (GUI_BITMAP*) fxVars[idx][1]);

		fxVars[idx][0] = 1; // Effect initialized
	}
}

// only black
void renderEffect08(uint8_t idx)
{
	if(fxVars[idx][0] == 0) {
		uint8_t *ptr = &imgData[idx][0];

		for(int i=0; i<numPixels; i++) {
		  *ptr++ = 0; *ptr++ = 0; *ptr++ = 0;
		}
		fxVars[idx][0] = 1; // Effect initialized
	}
}

// display Gif mode8()
void renderEffect09(uint8_t idx)
{
	//const GUI_BITMAP **picture = (const GUI_BITMAP**) fxVars[idx][1];
	uint8_t *ptr = &imgData[idx][0];

	if(fxVars[idx][0] == 0) {
		fxVars[idx][1] = 0;// Animation Time
		fxVars[idx][2] = 0;// Animation Step
		fxVars[idx][7] = 0;// animation complete
		//from mode8()
		//fxVars[idx][3];// pointer to picture GIF
		//fxVars[idx][4];// frames
		//fxVars[idx][5];// speed
		fxVars[idx][0] = 1; // Effect initialized
	}
	if(fxVars[idx][1] >=fxVars[idx][5]){
		fxVars[idx][1] = 0;

		fxVars[idx][2]++;
		if(fxVars[idx][2] >= fxVars[idx][4]){
			fxVars[idx][7] = 1;
			if (fxVars[idx][6] >= 1) {// if loop
				fxVars[idx][2] = 0;
			}else{
				//fxVars[idx][2] = fxVars[idx][4] - 1;
				fxVars[idx][2] -= 1;
			}
		}
	}

	drawGif(ptr, (const GUI_BITMAP**) fxVars[idx][3], fxVars[idx][2]);

	fxVars[idx][1]++;
}
// TO DO: Add more effects here...Larson scanner, etc.

// ---------------------------------------------------------------------------
// Alpha channel effect rendering functions.  Like the image rendering
// effects, these are typically parametrically-generated...but unlike the
// images, there is only one alpha renderer "in flight" at any given time.
// So it would be okay to use local static variables for storing state
// information...but, given that there could end up being many more render
// functions here, and not wanting to use up all the RAM for static vars
// for each, a third row of fxVars is used for this information.

// Simplest alpha effect: fade entire strip over duration of transition.
void renderAlpha00(void) {
  uint8_t fade = 255L * tCounter / transitionTime;
  for(int i=0; i<numPixels; i++) alphaMask[i] = fade;
}

// Straight left-to-right or right-to-left wipe
void renderAlpha01(void) {
  long x, y, b;
  if(fxVars[2][0] == 0) {
    fxVars[2][1] = random_maxmin(1, numPixels); // run, in pixels
    fxVars[2][2] = 255;//(random(2) == 0) ? 255 : -255; // rise
    fxVars[2][0] = 1; // Transition initialized
  }

  b = (fxVars[2][2] > 0) ?
    (255L + (numPixels * fxVars[2][2] / fxVars[2][1])) *
      tCounter / transitionTime - (numPixels * fxVars[2][2] / fxVars[2][1]) :
    (255L - (numPixels * fxVars[2][2] / fxVars[2][1])) *
      tCounter / transitionTime;
  for(x=0; x<numPixels; x++) {
    y = x * fxVars[2][2] / fxVars[2][1] + b; // y=mx+b, fixed-point style
    if(y < 0)         alphaMask[x] = 0;
    else if(y >= 255) alphaMask[x] = 255;
    else              alphaMask[x] = (uint8_t)y;
  }
}

// Dither reveal between images
void renderAlpha02(void) {
  long fade;
  int  i, bit, reverse, hiWord;

  if(fxVars[2][0] == 0) {
    // Determine most significant bit needed to represent pixel count.
    int hiBit, n = (numPixels - 1) >> 1;
    for(hiBit=1; n; n >>=1) hiBit <<= 1;
    fxVars[2][1] = hiBit;
    fxVars[2][0] = 1; // Transition initialized
  }

  for(i=0; i<numPixels; i++) {
    // Reverse the bits in i for ordered dither:
    for(reverse=0, bit=1; bit <= fxVars[2][1]; bit <<= 1) {
      reverse <<= 1;
      if(i & bit) reverse |= 1;
    }
    fade   = 256L * numPixels * tCounter / transitionTime;
    hiWord = (fade >> 8);
    if(reverse == hiWord)     alphaMask[i] = (fade & 255); // Remainder
    else if(reverse < hiWord) alphaMask[i] = 255;
    else                      alphaMask[i] = 0;
  }
}

// TO DO: Add more transitions here...triangle wave reveal, etc.

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
// Assorted fixed-point utilities below this line.  Not real interesting.


// Gamma correction
unsigned char gammaTable[]  = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
	1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 4, 4, 4, 4,
	5, 5, 5, 5, 6, 6, 6, 7, 7, 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11,
	11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16, 17, 17, 18, 18,
	19, 19, 20, 21, 21, 22, 22, 23, 23, 24, 25, 25, 26, 27, 27, 28,
	29, 29, 30, 31, 31, 32, 33, 34, 34, 35, 36, 37, 37, 38, 39, 40,
	40, 41, 42, 43, 44, 45, 46, 46, 47, 48, 49, 50, 51, 52, 53, 54,
	55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70,
	71, 72, 73, 74, 76, 77, 78, 79, 80, 81, 83, 84, 85, 86, 88, 89,
	90, 91, 93, 94, 95, 96, 98, 99,100,102,103,104,106,107,109,110,
	111,113,114,116,117,119,120,121,123,124,126,128,129,131,132,134,
	135,137,138,140,142,143,145,146,148,150,151,153,155,157,158,160,
	162,163,165,167,169,170,172,174,176,178,179,181,183,185,187,189,
	191,193,194,196,198,200,202,204,206,208,210,212,214,216,218,220,
	222,224,227,229,231,233,235,237,239,241,244,246,248,250,252,255
};
void GammaTableGen(double_t gammaVal)
{
	for (int i = 0; i <= 255; i++) {
		gammaTable[i] = 255 * pow((double_t)i / 255, gammaVal);
	}
}

inline uint8_t mygamma(uint8_t x)
{
	return gammaTable[x];
}

// Fixed-point colorspace conversion: HSV (hue-saturation-value) to RGB.
// This is a bit like the 'Wheel' function from the original strandtest
// code on steroids.  The angular units for the hue parameter may seem a
// bit odd: there are 1536 increments around the full color wheel here --
// not degrees, radians, gradians or any other conventional unit I'm
// aware of.  These units make the conversion code simpler/faster, because
// the wheel can be divided into six sections of 256 values each, very
// easy to handle on an 8-bit microcontroller.  Math is math, and the
// rendering code elsehwere in this file was written to be aware of these
// units.  Saturation and value (brightness) range from 0 to 255.
long hsv2rgb(long h, uint8_t s, uint8_t v) {
  uint8_t r, g, b, lo;
  int  s1;
  long v1;

  // Hue
  h %= 1536;           // -1535 to +1535
  if(h < 0) h += 1536; //     0 to +1535
  lo = h & 255;        // Low uint8_t  = primary/secondary color mix
  switch(h >> 8) {     // High uint8_t = sextant of colorwheel
    case 0 : r = 255     ; g =  lo     ; b =   0     ; break; // R to Y
    case 1 : r = 255 - lo; g = 255     ; b =   0     ; break; // Y to G
    case 2 : r =   0     ; g = 255     ; b =  lo     ; break; // G to C
    case 3 : r =   0     ; g = 255 - lo; b = 255     ; break; // C to B
    case 4 : r =  lo     ; g =   0     ; b = 255     ; break; // B to M
    default: r = 255     ; g =   0     ; b = 255 - lo; break; // M to R
  }

  // Saturation: add 1 so range is 1 to 256, allowig a quick shift operation
  // on the result rather than a costly divide, while the type upgrade to int
  // avoids repeated type conversions in both directions.
  s1 = s + 1;
  r = 255 - (((255 - r) * s1) >> 8);
  g = 255 - (((255 - g) * s1) >> 8);
  b = 255 - (((255 - b) * s1) >> 8);

  // Value (brightness) and 24-bit color concat merged: similar to above, add
  // 1 to allow shifts, and upgrade to long makes other conversions implicit.
  v1 = v + 1;
  return (((r * v1) & 0xff00) << 8) |
          ((g * v1) & 0xff00)       |
         ( (b * v1)           >> 8);
}

// The fixed-point sine and cosine functions use marginally more
// conventional units, equal to 1/2 degree (720 units around full circle),
// chosen because this gives a reasonable resolution for the given output
// range (-127 to +127).  Sine table intentionally contains 181 (not 180)
// elements: 0 to 180 *inclusive*.  This is normal.

unsigned char sineTable[181]  = {
    0,  1,  2,  3,  5,  6,  7,  8,  9, 10, 11, 12, 13, 15, 16, 17,
   18, 19, 20, 21, 22, 23, 24, 25, 27, 28, 29, 30, 31, 32, 33, 34,
   35, 36, 37, 38, 39, 40, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51,
   52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67,
   67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 77, 78, 79, 80, 81,
   82, 83, 83, 84, 85, 86, 87, 88, 88, 89, 90, 91, 92, 92, 93, 94,
   95, 95, 96, 97, 97, 98, 99,100,100,101,102,102,103,104,104,105,
  105,106,107,107,108,108,109,110,110,111,111,112,112,113,113,114,
  114,115,115,116,116,117,117,117,118,118,119,119,120,120,120,121,
  121,121,122,122,122,123,123,123,123,124,124,124,124,125,125,125,
  125,125,126,126,126,126,126,126,126,127,127,127,127,127,127,127,
  127,127,127,127,127
};

int8_t fixSin(int angle) {
  angle %= 720;               // -719 to +719
  if(angle < 0) angle += 720; //    0 to +719
  return (angle <= 360) ?
		  sineTable[(angle <= 180) ?
       angle          : // Quadrant 1
      (360 - angle)] : // Quadrant 2
    -sineTable[(angle <= 540) ?
      (angle - 360)   : // Quadrant 3
      (720 - angle)] ; // Quadrant 4
}

int8_t fixCos(int angle) {
  angle %= 720;               // -719 to +719
  if(angle < 0) angle += 720; //    0 to +719
  return (angle <= 360) ?
    ((angle <= 180) ?  sineTable[180 - angle]  : // Quad 1
                      -sineTable[angle - 180]) : // Quad 2
    ((angle <= 540) ? -sineTable[540 - angle]  : // Quad 3
    		sineTable[angle - 540]) ; // Quad 4
}
