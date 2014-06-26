
#include "pictures.h"

int drawBitmap(uint8_t *ptr, const GUI_BITMAP *picture)
{
	if(picture->picType == GUI_DRAW_BMP24 && picture->xSize == LED_XRES && picture->ySize == LED_YRES){
		for(int i=0; i<LED_XRES * LED_YRES * 3; i++) {
			ptr[i] =  picture->data[i];
		}
		return 1;
	}
	return 0;
}

int drawGif(uint8_t *ptr, const GUI_BITMAP **picture, uint32_t frame)
{
	uint32_t color;

	for(int i=0; i<LED_XRES * LED_YRES; i++) {
		if((picture[frame]->data[i]) < (picture[frame]->palette->size)  &&
				picture[frame]->xSize == LED_XRES && picture[frame]->ySize == LED_YRES)
		{
			if (picture[frame]->palette->transparrent == 1 && picture[frame]->data[i] == 0x00){
				color = TRANSPARENT_COLOR;
			} else {
				color =  picture[frame]->palette->colors[picture[frame]->data[i]];
			}
			//color = ((color<<16)&0xFF0000) | (color&0x00FF00) | (color>>16&0x0000FF);

			*ptr++ = color; *ptr++ = color >> 8; *ptr++ = color >> 16;
		}else{
			return 0;
		}
	}
	return 1;
}

