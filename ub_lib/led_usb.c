

#include "led_usb.h"

void init_ledPacketBuffer(ledPacketBuffer *const pBuffer)
{
	pBuffer->pCounter = 0;
	pBuffer->pByteCount = 0;
	pBuffer-> offset = 0;
	// Allocate packets. They'll have zero'ed contents initially.
	for (unsigned i = 0; i < NUM_BUFFS_PER_FRAME; ++i) {
		usb_packet_t *p = usb_malloc();
		if(p){
			memset(p->buf, 0, sizeof p->buf);
			pBuffer->packets[i] = p;
		}else{
			//UB_Led_Toggle(LED_RED);//ERROR TODO
		}
	}
}

void ledStorePacket(ledPacketBuffer *const pBuffer, usb_packet_t *const packet)
{
	if (pBuffer->pCounter < NUM_BUFFS_PER_FRAME) {
		// Store a packet, holding a reference to it.
		usb_packet_t *prev = pBuffer->packets[pBuffer->pCounter];
		pBuffer->packets[pBuffer->pCounter] = packet;
		usb_free(prev);
		pBuffer->pByteCount += packet->len;
		pBuffer->pCounter++;
	} else {
		// Error; ignore this packet.
		usb_free(packet);
	}
}



