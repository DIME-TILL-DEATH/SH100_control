#include <asf.h>
#include <math.h>

#include "uart.h"
#include "midi_parser.h"
#include "midi_controller.h"

#include "sh100_hardware.h"
#include "sh100_controller.h"

#include "front_buttons.h"

bool isAmpStarted = false;
int16_t negVdd;
int16_t posVdd;
void initTest()
{
	SH100HW_SetNewLedState(LED_PWR_GRN, LED_FAST_BLINKING);
	delay_ms(20000);
	
	SH100HW_StartADConvertion(ADC_V_SIGNAL);
	while(ADCSRA & (1<<ADSC)){}
		
	int32_t outLineValue = ADC;
	
	if(fabs(outLineValue - 0x3FF/2) > 100)
	{
		// DC on output!
		SH100HW_SetPAFailure(true);
		SH100HW_SetNewLedState(LED_PWR_GRN, LED_OFF);
		SH100HW_SetNewLedState(LED_PWR_RED, LED_FAST_BLINKING);
		while(1) {}
	}
	
	SH100HW_StartADConvertion(ADC_V_NEGATIVE);
	while(ADCSRA & (1<<ADSC)){}
		
	negVdd = ADC;
	
	SH100HW_StartADConvertion(ADC_V_POSITIVE);
	while(ADCSRA & (1<<ADSC)){}
		
	posVdd = ADC;
	
	if(fabs(negVdd-posVdd) > 100)
	{
		// one of Vdd rail is out
		SH100HW_SetPAFailure(true);
		SH100HW_SetNewLedState(LED_PWR_GRN, LED_OFF);
		SH100HW_SetNewLedState(LED_PWR_RED, LED_FAST_BLINKING);
		while(1) {}
	}
		
	SH100HW_SetPAFailure(false);
	SH100CTRL_CheckOutputJacks();
	isAmpStarted = true;
}

void ISRInit()
{
	// Timer0 init. For main task
	TCCR0B |= 0x05; // psc = 1024
	TIMSK0 |= 0x01; // OVF INT enable, count pulse = 100us
	TCNT0 = 100;
		
	// PCINT10, interrupt for PC2 pin(SW detect)
	PCICR |= 0x02;
	PCMSK1 |= 0x04;
}

int main(void)
{
	UART_init();
	SH100HW_Init();
	SH100CTRL_Init();
	MIDICTRL_Init();
	
	ISRInit();	
	cpu_irq_enable();
	
	initTest();
		
    while(1)
    {
		MIDI_ParserTask();
	}
}

//==========================Main AMP task=======================================
ISR(TIMER0_OVF_vect)
{
	SH100HW_MainTask();
	
	if(isAmpStarted)
	{
		SH100CTRL_CheckOutputJacks();
		SH100HW_StartADConvertion(ADC_V_NEGATIVE);
	}
	
	SH100HW_Controls_t pressedButtons = SH100HW_GetControlsState();
	
	MIDICTRL_SetMidiChannel(pressedButtons.midiChNum);
	MIDICTRL_OmniModeEn(pressedButtons.midiOmni);
	MIDICTRL_MuteCommEn(pressedButtons.midiMuteComm);
	
	FBTNS_MainTask(&pressedButtons);
}

int16_t negVdd;
int16_t posVdd;
bool measSwitch = false;
ISR(ADC_vect)
{
	if(isAmpStarted)
	{
		if(measSwitch)
		{
			posVdd = ADC;
			measSwitch = false;
			//SH100HW_StartADConvertion(ADC_V_NEGATIVE);
		}
		else
		{
			negVdd = ADC;
			measSwitch = true;
			SH100HW_StartADConvertion(ADC_V_POSITIVE);
		}
		
		if(fabs(negVdd-posVdd) > 100)
		{
			// one of Vdd rail is out
			SH100HW_SetPAFailure(true);
			SH100HW_SetNewLedState(LED_PWR_GRN, LED_OFF);
			SH100HW_SetNewLedState(LED_PWR_RED, LED_FAST_BLINKING);
		}
	}
}

//=========================PWR Turn off INT=================================
ISR(PCINT1_vect)
{
	// Power off
	// need to check pin level?
	SH100CTRL_StoreAmpState();
}

