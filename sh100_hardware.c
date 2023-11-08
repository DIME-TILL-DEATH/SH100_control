#include "sh100_hardware.h"
#include "sh100_controller.h"

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

#define ADC_REF_MASK 0xC0
#define ADC_CHANNEL_MASK 0x0F

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
SH100HW_LedState_t* led_ptr[LED_COUNT*2]; 

SH100HW_Buttons_t buttonsState;
void readButtonsState();
void writeShiftRegs(uint16_t data);

bool isPAOk;

void SH100HW_Init()
{
	isPAOk = false;
	
	// PINS init
	gpio_configure_pin(PIN_MOSI, IOPORT_INIT_LOW | IOPORT_DIR_OUTPUT);
	gpio_configure_pin(PIN_SCK, IOPORT_INIT_LOW | IOPORT_DIR_OUTPUT);
	gpio_configure_pin(PIN_ST, IOPORT_INIT_HIGH| IOPORT_DIR_OUTPUT);

	gpio_configure_pin(PIN_A, IOPORT_INIT_LOW | IOPORT_DIR_OUTPUT);
	gpio_configure_pin(PIN_B, IOPORT_INIT_LOW | IOPORT_DIR_OUTPUT);
	gpio_configure_pin(PIN_C, IOPORT_INIT_LOW | IOPORT_DIR_OUTPUT);

	gpio_configure_pin(PIN_BUTTONS, IOPORT_DIR_INPUT);
	gpio_configure_pin(PIN_MIDI_SWITCH, IOPORT_DIR_INPUT | IOPORT_PULL_UP);
	gpio_configure_pin(PIN_FOOTSWITCH, IOPORT_DIR_INPUT);
	
	gpio_configure_pin(PIN_SW_DETECT, IOPORT_DIR_INPUT);
	gpio_configure_pin(PIN_M8_DETECT, IOPORT_DIR_INPUT);
	gpio_configure_pin(PIN_M16_DETECT, IOPORT_DIR_INPUT);
	
	//gpio_configure_pin(PIN_SW, IOPORT_INIT_LOW | IOPORT_DIR_OUTPUT);
	gpio_configure_pin(PIN_MUTE, IOPORT_INIT_LOW | IOPORT_DIR_OUTPUT);
	gpio_configure_pin(PIN_RELE_W, IOPORT_INIT_LOW | IOPORT_DIR_OUTPUT);
	gpio_configure_pin(PIN_RELAY_LOOP, IOPORT_INIT_LOW | IOPORT_DIR_OUTPUT);
	
	for(uint8_t i=0; i < LED_COUNT*2; i++)
	{
		led_ptr[i] = malloc(sizeof(SH100HW_LedState_t));
	}
}

void SH100HW_SetPAFailure(bool isFail)
{
	isPAOk = !isFail;
	if(isFail)
	{
		SH100CTRL_MuteAmp();
	}
}

bool SH100HW_GetPAFailure()
{
	return !isPAOk;
}

SH100HW_Buttons_t SH100HW_GetButtonsState()
{
	return buttonsState;
}

void SH100HW_SetCh(uint8_t chNum)
{
	switch(chNum)
	{
		case 0:
		{
			RELAY_13_24 = REL_OFF;
			RELAY_1_3 = REL_OFF;
			RELAY_2_4 = REL_OFF;
			break;
		}
		case 1:
		{
			RELAY_13_24 = REL_ON;
			RELAY_1_3 = REL_OFF;
			RELAY_2_4 = REL_OFF;
			break;
		}
		case 2:
		{
			RELAY_13_24 = REL_OFF;
			RELAY_1_3 = REL_ON;
			RELAY_2_4 = REL_OFF;
			break;
		}
		case 3:
		{
			RELAY_13_24 = REL_ON;
			RELAY_1_3 = REL_OFF;
			RELAY_2_4 = REL_ON;
			break;
		}
		default: break;
	}
}

void SH100HW_LoopEn(bool isEnabled)
{
	RELAY_LOOP = isEnabled;
	ioport_set_pin_level(PIN_RELAY_LOOP, RELAY_LOOP);
}

void SH100HW_SetAB(bool isBEn)
{
	RELAY_AB = isBEn;
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

SH100HW_OutputJacks_t SH100HW_GetOutputJacks()
{
	bool is16Ohm = ioport_get_pin_level(PIN_M16_DETECT);
	bool is8Ohm = ioport_get_pin_level(PIN_M8_DETECT);
	return ((is16Ohm) | (is8Ohm<<1));
}

void SH100HW_SetPAState(SH100HW_OutputState_t state)
{
	if(isPAOk)
	{
		ioport_set_pin_level(PIN_MUTE, !state);
		ioport_set_pin_level(PIN_RELE_W, state);
	}
	else
	{		
		// PA failure, mute AMP
		ioport_set_pin_level(PIN_MUTE, 1);
		ioport_set_pin_level(PIN_RELE_W, 0);
	}
}

void SH100HW_SetOutputMode(SH100HW_PAMode_t mode)
{
	RELAY_8_16 = mode;
}

void SH100HW_StartADConvertion(ADC_Channels_t channel)
{
	ADMUX = (1<<REFS0) | channel; // AREF ext pin, Left-adjustment result
	DIDR0 = 0x01; // Disable digital io on PC0
	ADCSRA = (1<<ADEN) | (1<<ADSC) | (1<<ADIE) | (1<<ADPS2) | (1<<ADPS1); // ADC enable, INT enable, prescaler = 64
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
				ioport_set_pin_level(PIN_A, 0);
				ioport_set_pin_level(PIN_B, 0);
				ioport_set_pin_level(PIN_C, 0);
				
				buttonsState.midiOmni = ioport_get_pin_level(PIN_MIDI_SWITCH);
				buttonsState.FS2_sleeve = ioport_get_pin_level(PIN_FOOTSWITCH);
				break;
			}
			
			case 1:
			{
				ioport_set_pin_level(PIN_A, 1);
				ioport_set_pin_level(PIN_B, 0);
				ioport_set_pin_level(PIN_C, 0);
				
				midiChBit[3] = ioport_get_pin_level(PIN_MIDI_SWITCH);
				buttonsState.btnAB = ioport_get_pin_level(PIN_BUTTONS);
				buttonsState.FS2_presence = ioport_get_pin_level(PIN_FOOTSWITCH);
				break;
			}
			
			case 2:
			{
				ioport_set_pin_level(PIN_A, 0);
				ioport_set_pin_level(PIN_B, 1);
				ioport_set_pin_level(PIN_C, 0);
				
				midiChBit[2] = ioport_get_pin_level(PIN_MIDI_SWITCH);
				buttonsState.btnLoop = ioport_get_pin_level(PIN_BUTTONS);
				buttonsState.FS1_presence = ioport_get_pin_level(PIN_FOOTSWITCH);
				break;
			}
			
			case 3:
			{
				ioport_set_pin_level(PIN_A, 1);
				ioport_set_pin_level(PIN_B, 1);
				ioport_set_pin_level(PIN_C, 0);
				
				buttonsState.midiMuteComm = ioport_get_pin_level(PIN_MIDI_SWITCH);
				buttonsState.FS1_sleeve = ioport_get_pin_level(PIN_FOOTSWITCH);
				break;
			}
			
			case 4:
			{
				ioport_set_pin_level(PIN_A, 0);
				ioport_set_pin_level(PIN_B, 0);
				ioport_set_pin_level(PIN_C, 1);
				
				midiChBit[0] = ioport_get_pin_level(PIN_MIDI_SWITCH);
				buttonsState.btnCh2 = ioport_get_pin_level(PIN_BUTTONS);
				buttonsState.FS1_tip = ioport_get_pin_level(PIN_FOOTSWITCH);
				break;
			}
			
			case 5:
			{
				ioport_set_pin_level(PIN_A, 1);
				ioport_set_pin_level(PIN_B, 0);
				ioport_set_pin_level(PIN_C, 1);
				
				buttonsState.btnCh3 = ioport_get_pin_level(PIN_BUTTONS);
				break;
			}
			
			case 6:
			{
				ioport_set_pin_level(PIN_A, 0);
				ioport_set_pin_level(PIN_B, 1);
				ioport_set_pin_level(PIN_C, 1);
				
				midiChBit[1] = ioport_get_pin_level(PIN_MIDI_SWITCH);
				buttonsState.btnCh4 = ioport_get_pin_level(PIN_BUTTONS);
				buttonsState.FS2_tip = ioport_get_pin_level(PIN_FOOTSWITCH);
				break;
			}
			
			case 7:
			{
				ioport_set_pin_level(PIN_A, 1);
				ioport_set_pin_level(PIN_B, 1);
				ioport_set_pin_level(PIN_C, 1);
				
				buttonsState.btnCh1 = ioport_get_pin_level(PIN_BUTTONS);
				break;
			}
			default: break;
		}
	}
	
	buttonsState.midiChNum = midiChBit[0] | (midiChBit[1]<<1) | (midiChBit[2]<<2) | (midiChBit[3]<<3);
}

void writeShiftRegs(uint16_t data)
{
	ioport_set_pin_level(PIN_ST, 0);
	for(uint8_t i=0; i<16; i++)
	{
		ioport_set_pin_level(PIN_SCK, 0);
		ioport_set_pin_level(PIN_MOSI, (data & 0x8000));
		ioport_set_pin_level(PIN_SCK, 1);
		data = data << 1;
	}
	ioport_set_pin_level(PIN_SCK, 0);
	ioport_set_pin_level(PIN_ST, 1);
}

uint8_t blinkCounter = 0;
bool slowBlink = false;
bool fastBlink = false;
uint8_t indErrorCnt = 0;
void SH100HW_MainTask()
{
	if(!isPAOk) SH100CTRL_MuteAmp();
	
	readButtonsState();
	
	// blink work----------------------------------------------
	bool isLedOn[LED_COUNT];	
	if((blinkCounter % 25) == 0 && blinkCounter != 0)
	{
		fastBlink = !fastBlink;
	}
	
	if(blinkCounter == 50)
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
			default: isLedOn[i] = LED_OFF;
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
								((uint16_t)RELAY_2_4 << (8+0))			|
								((uint16_t)isLedOn[LED_A] << (8+1))	|
								((uint16_t)isLedOn[LED_B] << (8+2))	|
								((uint16_t)isLedOn[LED_PWR_GRN] << (8+3))	|
								((uint16_t)isLedOn[LED_PWR_RED] << (8+4))	|
								((uint16_t)isLedOn[LED_LOOP] << (8+5))		|
								((uint16_t)RELAY_AB << (8+6)) |
								(0 << (8+7));

	writeShiftRegs(resultSendWord);
}