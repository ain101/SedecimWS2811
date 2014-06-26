/*
 * rc5.h
 *
 *  Created on: 30.04.2012
 *      Author: Frederik
 */

#ifndef RC5_H_
#define RC5_H_
/*
#define RC5TIME 	1.778e-3		// 1.778ms
#define T1_init_wert (1770/14)
#define T1_eine_sek	(1000000/T1_init_wert)
#define PULSE_MIN	(unsigned char)(T1_eine_sek * RC5TIME * 0.4 + 0.5)//+0.5 zum runden
#define PULSE_1_2	(unsigned char)(T1_eine_sek * RC5TIME * 0.8 + 0.5)
#define PULSE_MAX	(unsigned char)(T1_eine_sek * RC5TIME * 1.2 + 0.5)

#define INFRAROT 			((PINC&0x01)!=0x01)//=arduino pin 23
#define TOGGLEBIT			 ((rc5_data & 0x800) == 0x800)
#define C6_BIT				((rc5_data&0x1000)!=0x1000)
*/
#define BUTTON_POWER		12
#define BUTTON_HINAUF		80
#define BUTTON_HINUNTER		81
#define BUTTON_LINKS		85
#define BUTTON_RECHTS		86
#define BUTTON_PLUS			32//Play rechts
#define BUTTON_MINUS		33//Play links
#define BUTTON_ENTER		87
#define BUTTON_PLAY			63
#define BUTTON_PAUSE		41
#define BUTTON_VOL_PLUS		16
#define BUTTON_VOL_MINUS	17
#define BUTTON_REW_RECHTS	46
#define BUTTON_REW_LINKS	60
#define BUTTON_THREE_LINES	60
#define BUTTON_STOP			109
#define BUTTON_ROT			107
#define BUTTON_GRUEN		108
#define BUTTON_GELB			109
#define BUTTON_BLAU			110
#define BUTTON_RADIO		113
#define BUTTON_IPLUS		15
#define BUTTON_MENUE		82
#define BUTTON_REJECT		13

#define BUTTON_PLAYPAUSE	53
#define BUTTON_STOP2	54
#define BUTTON_DSC		79
#define BUTTON_DBB		70
#define BUTTON_IS		64
#define BUTTON_MUTE		13
#define BUTTON_NEW		90

#define IR_ADDR_CD			20
#define IR_ADDR_TUNER		17
#define IR_ADDR_TAPE		18
#define IR_ADDR_AUX			21


//volatile unsigned int  rc5_data = 0;


#endif /* RC5_H_ */
