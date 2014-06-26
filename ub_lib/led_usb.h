
#ifndef _LED_USB_H_
#define _LED_USB_H_

#include "usb_mem.h"
#include <string.h>
#include <stdint.h>

#define NUM_BUFFS_PER_FRAME (140)// (68*42*24 / 8) / 64 =~135 USB packets for one frame //
#define tSize (512) // sollte durch 32 teilbar sein


typedef struct ledPacketBuffer_struct{
	usb_packet_t *packets[tSize];
	uint32_t	pCounter;
	uint32_t	pByteCount;
	uint8_t	offset;
}ledPacketBuffer;

void init_ledPacketBuffer(ledPacketBuffer *const pBuffer);

void ledStorePacket(ledPacketBuffer *const pBuffer, usb_packet_t *const packet);



#endif // _LED_USB_H_
