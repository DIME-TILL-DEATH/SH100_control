#include "sh100_hardware.h"

#include <asf.h>
#include <stdlib.h>

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
	REL_OFF = 0,
	REL_ON = 1
}RelayState_t;

RelayState_t RELAY_13_24;
RelayState_t RELAY_1_3;
RelayState_t RELAY_8_16;
RelayState_t RELAY_2_4;
RelayState_t RELAY_AB;
RelayState_t RELAY_LOOP;

#define LED_COUNT 9
#define LED_PREVIOUS_STATE_OFFSET LED_COUNT
SH100HW_LedState_t* led_ptr[LED_COUNT*2]; // for iterating

//SH100HW_LedState_t ledCh1;	
//SH100HW_LedState_t ledCh2;	
//SH100HW_LedState_t ledCh3;	
//SH100HW_LedState_t ledCh4;	
//SH100HW_LedState_t ledA;	
//SH100HW_LedState_t ledB;	
//SH100HW_LedState_t ledLoop;	
//SH100HW_LedState_t ledPwrGrn;
//SH100HW_LedState_t ledPwrRed;	

SH100HW_Buttons_t buttonsState;
void readButtonsState();
void writeShiftRegs(uint16_t data);

void SH100HW_Init()
{
	// PINS init
	gpio_configure_pin(PIN_MOSI, IOPORT_INIT_LOW | IOPORT_DIR_OUTPUT);
	gpio_configure_pin(PIN_SCK, IOPORT_INIT_LOW | IOPORT_DIR_OUTPUT);
	gpio_configure_pin(PIN_ST, IOPORT_INIT_HIGH| IOPORT_DIR_OUTPUT);

	gpio_configure_pin(PIN_A, IOPORT_INIT_LOW | IOPORT_DIR_OUTPUT);
	gpio_configure_pin(PIN_B, IOPORT_INIT_LOW | IOPORT_DIR_OUTPUT);
	gpio_configure_pin(PIN_C, IOPORT_INIT_LOW | IOPORT_DIR_OUTPUT);

	gpio_configure_pin(PIN_BUTTONS, IOPORT_DIR_INPUT);
	gpio_configure_pin(PIN_MIDI_SWITCH, IOPORT_DIR_INPUT);
	gpio_configure_pin(PIN_FOOTSWITCH, IOPORT_DIR_INPUT);
	
	gpio_configure_pin(PIN_SW_DETECT, IOPORT_DIR_INPUT);
	gpio_configure_pin(PIN_M8_DETECT, IOPORT_DIR_INPUT);
	gpio_configure_pin(PIN_M16_DETECT, IOPORT_DIR_INPUT);
	
	gpio_configure_pin(PIN_SW, IOPORT_INIT_LOW | IOPORT_DIR_OUTPUT);
	gpio_configure_pin(PIN_MUTE, IOPORT_INIT_LOW | IOPORT_DIR_OUTPUT);
	gpio_configure_pin(PIN_RELE_W, IOPORT_INIT_LOW | IOPORT_DIR_OUTPUT);
	gpio_configure_pin(PIN_RELAY_LOOP, IOPORT_INIT_LOW | IOPORT_DIR_OUTPUT);
	
	// forming led pointers for iteration
	//led_ptr[LED_CH1] = &ledCh1;
	//led_ptr[LED_CH2] = &ledCh2;
	//led_ptr[LED_CH3] = &ledCh3;
	//led_ptr[LED_CH4] = &ledCh4;
	//led_ptr[LED_LOOP] = &ledLoop;
	//led_ptr[LED_A] = &ledA;
	//led_ptr[LED_B] = &ledB;
	//led_ptr[LED_PWR_GRN] = &ledPwrGrn;
	//led_ptr[LED_PWR_RED] = &ledPwrRed;
	for(uint8_t i=0; i < LED_COUNT*2; i++)
	{
		led_ptr[i] = malloc(sizeof(SH100HW_LedState_t));
	}
}

SH100HW_Buttons_t SH100HW_GetButtonsState()
{
	return buttonsState;
}

void SH100HW_SwitchCh(uint8_t chNum)
{
	switch(chNum)
	{
		case 0:
		{
			
			break;
		}
		case 1:
		{
			
			break;
		}
		case 2:
		{
			
			break;
		}
		case 3:
		{
			
			break;
		}
		default: break;
	}
}

void SH100HW_LoopEn(bool isEnabled)
{
	
}

void SH100HW_SwitchAB(bool isBEn)
{
	
}

void SH100HW_SetNewLedState(uint8_t ledId, SH100HW_LedState_t newState)
{
	*led_ptr[ledId+LED_PREVIOUS_STATE_OFFSET] = *led_ptr[ledId];
	*led_ptr[ledId] = newState;
}

void SH100HW_SetPreviousLedState(uint8_t ledId)
{
	*led_ptr[ledId] = *led_ptr[ledId+LED_PREVIOUS_STATE_OFFSET];
}

//=================================== PRIVATE FUNCTIONS==============================
void readButtonsState()
{
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
				
				buttonsState.midiOmni = arch_ioport_get_pin_level(PIN_MIDI_SWITCH);
				buttonsState.FS2_sleeve = arch_ioport_get_pin_level(PIN_FOOTSWITCH);
				break;
			}
			
			case 1:
			{
				arch_ioport_set_pin_level(PIN_A, 1);
				arch_ioport_set_pin_level(PIN_B, 0);
				arch_ioport_set_pin_level(PIN_C, 0);
				
				midiChBit[3] = arch_ioport_get_pin_level(PIN_MIDI_SWITCH);
				buttonsState.btnAB = arch_ioport_get_pin_level(PIN_BUTTONS);
				buttonsState.FS2_presence = arch_ioport_get_pin_level(PIN_FOOTSWITCH);
				break;
			}
			
			case 2:
			{
				arch_ioport_set_pin_level(PIN_A, 0);
				arch_ioport_set_pin_level(PIN_B, 1);
				arch_ioport_set_pin_level(PIN_C, 0);
				
				midiChBit[2] = arch_ioport_get_pin_level(PIN_MIDI_SWITCH);
				buttonsState.btnLoop = arch_ioport_get_pin_level(PIN_BUTTONS);
				buttonsState.FS1_presence = arch_ioport_get_pin_level(PIN_FOOTSWITCH);
				break;
			}
			
			case 3:
			{
				arch_ioport_set_pin_level(PIN_A, 1);
				arch_ioport_set_pin_level(PIN_B, 1);
				arch_ioport_set_pin_level(PIN_C, 0);
				
				buttonsState.midiMuteComm = arch_ioport_get_pin_level(PIN_MIDI_SWITCH);
				buttonsState.FS1_sleeve = arch_ioport_get_pin_level(PIN_FOOTSWITCH);
				break;
			}
			
			case 4:
			{
				arch_ioport_set_pin_level(PIN_A, 0);
				arch_ioport_set_pin_level(PIN_B, 0);
				arch_ioport_set_pin_level(PIN_C, 1);
				
				midiChBit[0] = arch_ioport_get_pin_level(PIN_MIDI_SWITCH);
				buttonsState.btnCh2 = arch_ioport_get_pin_level(PIN_BUTTONS);
				buttonsState.FS1_tip = arch_ioport_get_pin_level(PIN_FOOTSWITCH);
				break;
			}
			
			case 5:
			{
				arch_ioport_set_pin_level(PIN_A, 1);
				arch_ioport_set_pin_level(PIN_B, 0);
				arch_ioport_set_pin_level(PIN_C, 1);
				
				buttonsState.btnCh3 = arch_ioport_get_pin_level(PIN_BUTTONS);
				break;
			}
			
			case 6:
			{
				arch_ioport_set_pin_level(PIN_A, 0);
				arch_ioport_set_pin_level(PIN_B, 1);
				arch_ioport_set_pin_level(PIN_C, 1);
				
				midiChBit[2] = arch_ioport_get_pin_level(PIN_MIDI_SWITCH);
				buttonsState.btnCh4 = arch_ioport_get_pin_level(PIN_BUTTONS);
				buttonsState.FS2_tip = arch_ioport_get_pin_level(PIN_FOOTSWITCH);
				break;
			}
			
			case 7:
			{
				arch_ioport_set_pin_level(PIN_A, 1);
				arch_ioport_set_pin_level(PIN_B, 1);
				arch_ioport_set_pin_level(PIN_C, 1);
				
				buttonsState.btnCh1 = arch_ioport_get_pin_level(PIN_BUTTONS);
				break;
			}
			default: break;
		}
	}
	
	buttonsState.midiChNum = midiChBit[0] | (midiChBit[1]<<1) | (midiChBit[2]<<2) | (midiChBit[3]<<3);
}

void writeShiftRegs(uint16_t data)
{
	arch_ioport_set_pin_level(PIN_ST, 1);
	for(uint8_t i=0; i<16; i++)
	{
		data = data << i;
		arch_ioport_set_pin_level(PIN_SCK, 0);
		arch_ioport_set_pin_level(PIN_MOSI, (data & 0x80));
		arch_ioport_set_pin_level(PIN_SCK, 1);
	}
	arch_ioport_set_pin_level(PIN_SCK, 0);
	arch_ioport_set_pin_level(PIN_ST, 1);
}

uint8_t blinkCounter = 0;
bool slowBlink = false;
bool fastBlink = false;
uint8_t indErrorCnt = 0;
void SH100HW_MainTask()
{
	readButtonsState();
	
	// blink work----------------------------------------------
	bool isLedOn[LED_COUNT];	
	if((blinkCounter % 25) == 0 && blinkCounter != 0)
	{
		fastBlink = !fastBlink;
	}
	
	if(blinkCounter == 100)
	{
		blinkCounter = 0;
		slowBlink = !slowBlink;
	}
	else
	{
		blinkCounter++;
	}
	
	for(uint8_t i=0; i<LED_COUNT; i++)
	{
		switch(*led_ptr[i])
		{
			case LED_OFF: isLedOn[i] = false; break;
			case LED_ON: isLedOn[i] = true; break;
			case LED_FAST_BLINKING: 
			{
				isLedOn[i] = fastBlink; 
				//isLedOn[LED_PWR_GRN] = !fastBlink; // Green led blink 180deg phase of red led
				break;
			}
			case LED_SLOW_BLINKING: 
			{
				isLedOn[i] = slowBlink;
				isLedOn[LED_PWR_GRN] = !slowBlink; // Green led slow blink 180deg phase of red led
				break;
			}	
		}
	}
	
	// form result--------------------------------------------------------------
	uint16_t resultSendWord =	((uint16_t)RELAY_13_24)				|
								((uint16_t)RELAY_1_3 << 1)			|
								((uint16_t)isLedOn[LED_CH3] << 2)	|
								((uint16_t)isLedOn[LED_CH1] << 3)	|
								((uint16_t)isLedOn[LED_CH2] << 4)	|
								((uint16_t)isLedOn[LED_CH4] << 5)	|
								(0 << 6)					|
								((uint16_t)RELAY_8_16 << 7)			|
								((uint16_t)RELAY_2_4 << 8)			|
								((uint16_t)isLedOn[LED_A] << 9)	|
								((uint16_t)isLedOn[LED_B] << 10)	|
								((uint16_t)isLedOn[LED_PWR_GRN] << 11)	|
								((uint16_t)isLedOn[LED_PWR_RED] << 12)	|
								((uint16_t)isLedOn[LED_LOOP] << 13);

	writeShiftRegs(resultSendWord);
}