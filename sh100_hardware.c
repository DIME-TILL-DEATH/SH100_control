#include "sh100_hardware.h"

#include <asf.h>

#ifndef SH100_PINS
#define SH100_PINS

#define PIN_RXD	IOPORT_CREATE_PIN(PORTD, 0)
#define PIN_TXD IOPORT_CREATE_PIN(PORTD, 1)

#define PIN_MOSI	IOPORT_CREATE_PIN(PORTB, 3)
#define PIN_MISO	IOPORT_CREATE_PIN(PORTB, 4)
#define PIN_SCK		IOPORT_CREATE_PIN(PORTB, 5)
#define PIN_ST		IOPORT_CREATE_PIN(PORTC, 4)

#define PIN_A	IOPORT_CREATE_PIN(PORTD, 5)
#define PIN_B	IOPORT_CREATE_PIN(PORTD, 6)
#define PIN_C	IOPORT_CREATE_PIN(PORTD, 7)

#define PIN_BUTTONS		IOPORT_CREATE_PIN(PORTD, 2)
#define PIN_MIDI_SWITCH	IOPORT_CREATE_PIN(PORTD, 4)
#define PIN_FOOTSWITCH	IOPORT_CREATE_PIN(PORTC, 5)

#define PIN_SW			IOPORT_CREATE_PIN(PORTD, 3)
#define PIN_MUTE		IOPORT_CREATE_PIN(PORTC, 1)
#define PIN_RELE_W		IOPORT_CREATE_PIN(PORTC, 3)
#define PIN_RELAY_LOOP	IOPORT_CREATE_PIN(PORTB, 2)

#define PIN_SW_DETECT	IOPORT_CREATE_PIN(PORTC, 2)
#define PIN_M8_DETECT	IOPORT_CREATE_PIN(PORTB, 1)
#define PIN_M16_DETECT	IOPORT_CREATE_PIN(PORTB, 0)

#endif

typedef enum
{
	REL_ON,
	REL_OFF
}RelayState_t;

RelayState_t RELAY13_24;
RelayState_t RELAY1_3;
RelayState_t RELAY8_16;
RelayState_t RELAY2_4;

void SH100HW_Init()
{
	// PINS dir, mode
	
	// Timer
}


SH100HW_Buttons_t SH100HW_GetButtonsState()
{
	SH100HW_Buttons_t result;
	
	uint8_t midiChBit[4];
	for(uint8_t cnt = 0; cnt<8; cnt++)
	{
		
		switch(cnt)
		{
			case 0:
			{
				arch_ioport_set_pin_level(PIN_A, 0);
				arch_ioport_set_pin_level(PIN_B, 0);
				arch_ioport_set_pin_level(PIN_C, 0);
				
				result.midiOmni = arch_ioport_get_pin_level(PIN_MIDI_SWITCH);
				result.FS2_sleeve = arch_ioport_get_pin_level(PIN_FOOTSWITCH);
				break;
			}
			
			case 1:
			{
				arch_ioport_set_pin_level(PIN_A, 1);
				arch_ioport_set_pin_level(PIN_B, 0);
				arch_ioport_set_pin_level(PIN_C, 0);
				
				midiChBit[3] = arch_ioport_get_pin_level(PIN_MIDI_SWITCH);
				result.btnAB = arch_ioport_get_pin_level(PIN_BUTTONS);
				result.FS2_presence = arch_ioport_get_pin_level(PIN_FOOTSWITCH);
				break;
			}
			
			case 2:
			{
				arch_ioport_set_pin_level(PIN_A, 0);
				arch_ioport_set_pin_level(PIN_B, 1);
				arch_ioport_set_pin_level(PIN_C, 0);
				
				midiChBit[2] = arch_ioport_get_pin_level(PIN_MIDI_SWITCH);
				result.btnLoop = arch_ioport_get_pin_level(PIN_BUTTONS);
				result.FS1_presence = arch_ioport_get_pin_level(PIN_FOOTSWITCH);
				break;
			}
			
			case 3:
			{
				arch_ioport_set_pin_level(PIN_A, 1);
				arch_ioport_set_pin_level(PIN_B, 1);
				arch_ioport_set_pin_level(PIN_C, 0);
				
				result.midiMuteComm = arch_ioport_get_pin_level(PIN_MIDI_SWITCH);
				result.FS1_sleeve = arch_ioport_get_pin_level(PIN_FOOTSWITCH);
				break;
			}
			
			case 4:
			{
				arch_ioport_set_pin_level(PIN_A, 0);
				arch_ioport_set_pin_level(PIN_B, 0);
				arch_ioport_set_pin_level(PIN_C, 1);
				
				midiChBit[0] = arch_ioport_get_pin_level(PIN_MIDI_SWITCH);
				result.btnCh2 = arch_ioport_get_pin_level(PIN_BUTTONS);
				result.FS1_tip = arch_ioport_get_pin_level(PIN_FOOTSWITCH);
				break;
			}
			
			case 5:
			{
				arch_ioport_set_pin_level(PIN_A, 1);
				arch_ioport_set_pin_level(PIN_B, 0);
				arch_ioport_set_pin_level(PIN_C, 1);
				
				result.btnCh3 = arch_ioport_get_pin_level(PIN_BUTTONS);
				break;
			}
			
			case 6:
			{
				arch_ioport_set_pin_level(PIN_A, 0);
				arch_ioport_set_pin_level(PIN_B, 1);
				arch_ioport_set_pin_level(PIN_C, 1);
				
				midiChBit[2] = arch_ioport_get_pin_level(PIN_MIDI_SWITCH);
				result.btnCh4 = arch_ioport_get_pin_level(PIN_BUTTONS);
				result.FS2_tip = arch_ioport_get_pin_level(PIN_FOOTSWITCH);
				break;
			}
			
			case 7:
			{
				arch_ioport_set_pin_level(PIN_A, 1);
				arch_ioport_set_pin_level(PIN_B, 1);
				arch_ioport_set_pin_level(PIN_C, 1);
				
				result.btnCh1 = arch_ioport_get_pin_level(PIN_BUTTONS);
				break;
			}
			default: break;
		}
	}
	
	result.midiChNum = midiChBit[0] | (midiChBit[1]<<1) | (midiChBit[2]<<2) | (midiChBit[3]<<3);
	return result;
}