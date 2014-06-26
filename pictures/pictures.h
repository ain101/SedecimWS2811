
#ifndef _PICTURES_H_
#define _PICTURES_H_

#include <stdlib.h>
#include "stdint.h"
#include "ws2811.h"

#ifndef GUI_CONST_STORAGE
  #define GUI_CONST_STORAGE const
#endif

//#define NUM_PICTURES 7
#define TRANSPARENT_COLOR 0x000000

enum GUI_DRAW_ENUM {
	GUI_DRAW_BMP24 = 0
};

typedef unsigned int GUI_COLOR;

typedef struct GUI_LOGPALETTE_STR {
	const unsigned int size;
	const unsigned char transparrent;
	const GUI_COLOR * colors;
}GUI_LOGPALETTE;

typedef struct GUI_BITMAP_STR {
	const unsigned int xSize;
	const unsigned int ySize;
	const unsigned int BytesPerLine;
	const unsigned int BitsPerPixel;
	const unsigned char* const data;  // Pointer to picture data
	const GUI_LOGPALETTE* const palette; //Pointer to palette
	const enum GUI_DRAW_ENUM picType;
}GUI_BITMAP;

typedef unsigned int GUI_COLOR;


int drawBitmap(uint8_t *ptr, const GUI_BITMAP * const picture);

int drawGif(uint8_t *ptr, const GUI_BITMAP **picture, uint32_t frame);

// every Picture must be declared here
GUI_CONST_STORAGE GUI_BITMAP bmATN_Logo;
GUI_CONST_STORAGE GUI_BITMAP bmFZero;
GUI_CONST_STORAGE GUI_BITMAP bmich;
GUI_CONST_STORAGE GUI_BITMAP bmLinkbig;
GUI_CONST_STORAGE GUI_BITMAP bmMario;
GUI_CONST_STORAGE GUI_BITMAP bmstrawhat;
GUI_CONST_STORAGE GUI_BITMAP bmZorro;
GUI_CONST_STORAGE GUI_BITMAP ATN_Random;
GUI_CONST_STORAGE GUI_BITMAP bmbio;

GUI_CONST_STORAGE GUI_BITMAP bmATN_Rainbow;
GUI_CONST_STORAGE GUI_BITMAP bmGAMMA;
GUI_CONST_STORAGE GUI_BITMAP bmUSB;
GUI_CONST_STORAGE GUI_BITMAP bmstm32;
GUI_CONST_STORAGE GUI_BITMAP bmstm32F4;


#define ANZAHL_GIFS 9
GUI_CONST_STORAGE GUI_BITMAP *apbmmario2[48];
GUI_CONST_STORAGE unsigned     aDelaymario2[48];

GUI_CONST_STORAGE GUI_BITMAP *apbmtunnel[14];
GUI_CONST_STORAGE unsigned     aDelaytunnel[14];

GUI_CONST_STORAGE GUI_BITMAP * apbmBlueFalcon2[26];
GUI_CONST_STORAGE unsigned     aDelayBlueFalcon2[26];

GUI_CONST_STORAGE GUI_BITMAP * apbmMarioAtack2[9];
GUI_CONST_STORAGE unsigned     aDelayMarioAtack2[9];

GUI_CONST_STORAGE GUI_BITMAP * apbmaxterix[14];
GUI_CONST_STORAGE unsigned     aDelayaxterix[14];

GUI_CONST_STORAGE GUI_BITMAP * apbmavatar[27];
GUI_CONST_STORAGE unsigned     aDelayavatar[27];

GUI_CONST_STORAGE GUI_BITMAP * apbmRayman[18];
GUI_CONST_STORAGE unsigned     aDelayRayman[18];

GUI_CONST_STORAGE GUI_BITMAP * apbmsonic[35];
GUI_CONST_STORAGE unsigned     aDelaysonic[35];

GUI_CONST_STORAGE GUI_BITMAP * apbmATN_rotate[51];
GUI_CONST_STORAGE unsigned     aDelayATN_rotate[51];


#endif // _PICTURES_H_
