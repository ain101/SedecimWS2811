/* Teensyduino Core Library
 * http://www.pjrc.com/teensy/
 * Copyright (c) 2013 PJRC.COM, LLC.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * 1. The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * 2. If the Software is incorporated into a build system that allows 
 * selection among a list of target devices, then similar target
 * devices manufactured by PJRC.COM must be included in the list of
 * target devices and selectable in the same manner.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

//#include "mk20dx128.h"
//#include "HardwareSerial.h"
//#include "usb_dev.h"
#include "usb_mem.h"

#define __disable_irq() asm volatile("CPSID i");
#define __enable_irq()  asm volatile("CPSIE i");

//__attribute__ ((section(".usbbuffers"), used))//DEBUG
unsigned char usb_buffer_memory[NUM_USB_BUFFERS * sizeof(usb_packet_t)];

static uint32_t usb_buffer_available[16] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};//[x] = (NUM_USB_BUFFERS / 32) + 0.5

// use bitmask and CLZ instruction to implement fast free list
// http://www.archivum.info/gnu.gcc.help/2006-08/00148/Re-GCC-Inline-Assembly.html
// http://gcc.gnu.org/ml/gcc/2012-06/msg00015.html
// __builtin_clz()
void init_usb_mem()
{
	if((sizeof(usb_buffer_memory) / sizeof(uint32_t)) >= (NUM_USB_BUFFERS / 32) + 0.5){
		for(int i = 0; i < (NUM_USB_BUFFERS / 32) + 0.5; i++){
			usb_buffer_available[i] = 0xFFFFFFFF;
		}
	}else{
		while(1){};//TODO ERROR
	}
}

usb_packet_t * usb_malloc(void)
{
    unsigned int n, avail, idx = 0;
    uint8_t *p;

    __disable_irq();

    do {
        avail = usb_buffer_available[idx];
        if (avail) {
            break;
        }
        idx++;
    } while (idx < sizeof(usb_buffer_available) / sizeof(usb_buffer_available[0]));

    n = __builtin_clz(avail) + (idx << 5);
    if (n >= NUM_USB_BUFFERS) {
        __enable_irq();
        return NULL;
    }

    usb_buffer_available[idx] = avail & ~(0x80000000 >> (n & 31));
    __enable_irq();

    p = usb_buffer_memory + (n * sizeof(usb_packet_t));

    *(uint32_t *)p = 0;
    *(uint32_t *)(p + 4) = 0;
    return (usb_packet_t *)p;
}

// for the receive endpoints to request memory
extern uint8_t usb_rx_memory_needed;
extern void usb_rx_memory(usb_packet_t *packet);

void usb_free(usb_packet_t *p)
{
    unsigned int n, mask, idx;

    n = ((uint8_t *)p - usb_buffer_memory) / sizeof(usb_packet_t);
    if (n >= NUM_USB_BUFFERS) return;
/*
    // if any endpoints are starving for memory to receive	//TODO
    // packets, give this memory to them immediately!
    if (usb_rx_memory_needed && usb_configuration) {
        usb_rx_memory(p);
        return;
    }
*/
    idx = n >> 5;
    mask = 0x80000000 >> (n & 31);
    __disable_irq();
    usb_buffer_available[idx] |= mask;
    __enable_irq();
}

