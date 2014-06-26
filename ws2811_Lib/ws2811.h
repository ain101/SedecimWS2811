//////////////////////////////////////////////////////////////////////////
///                                                                    ///
/// SedecimWS2811                                                      ///
///                                                                    ///
/// Copyright (c) Frederik Wenigwieser, frederik.wenigwieser@atn.ac.at ///
///                                                                    ///
//////////////////////////////////////////////////////////////////////////
#ifndef WS2811_H
#define WS2811_H

#include <stm32f4xx.h>

// enter number of LEDs here!

//#define DEBUG_WS2811

#define LED_XRES 68
#define LED_YRES 42
#define LED_ROWS_PER_PIN 3
#define LED_STRIPLEN (68*LED_ROWS_PER_PIN)
#define FRAMEBUFFER_SIZE (LED_XRES*LED_YRES)

#define LED_NUM_STRIPES 14


// Size of Buffer
#define PWM_BUFFER_SIZE (FRAMEBUFFER_SIZE / LED_NUM_STRIPES *24) // 24 Bit/Pixel
#define LED_RESET_TIME 50// in µs // >10 // time the ws2811 chip needs to latch


int led_update_in_progress;

unsigned int led_frame_finished;


void Ws2811_init(void);
void LedShow ();
int LedBusy();
void LedsetPixel(uint32_t num, uint32_t color);
void LedStripPixel(uint8_t strip, uint32_t num, uint32_t color);

#endif //WS2811_H
