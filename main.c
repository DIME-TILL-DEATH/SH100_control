#include <asf.h>

#include "uart.h"
#include "midi_parser.h"
#include "midi_controller.h"

#include "sh100_hardware.h"
#include "sh100_controller.h"

int main(void)
{
	UART_init();
	SH100HW_Init();
	SH100CTRL_Init();
	MIDICTRL_Init();
	
	// Timer0 init. For main task
	TCCR0B |= 0x05; // psc = 1024
	TIMSK0 |= 0x01; // OVF INT enable, count pulse = 100us
	TCNT0 = 100;
	
	// PCINT10, interrupt for PC2 pin(SW detect)
	PCICR |= 0x02;
	PCMSK1 |= 0x04;
	
	cpu_irq_enable();
	
    while(1)
    {
		MIDI_ParserTask();
	}
}

#define PROTECTION_INTERVAL 20
#define SWITCH_MODE_DELAY 200/PROTECTION_INTERVAL
uint8_t protectionInterval_cnt = PROTECTION_INTERVAL;
uint8_t swProgrammingMode_cnt = 0;
uint8_t leaveDefSettings_cnt = 0;

ISR(TIMER0_OVF_vect)
{
	SH100HW_MainTask();
	SH100CTRL_CheckOutputJacks();
	
	SH100HW_Buttons_t pressedButtons = SH100HW_GetButtonsState();
	
	MIDICTRL_SetMidiChannel(pressedButtons.midiChNum);
	MIDICTRL_OmniModeEn(pressedButtons.midiOmni);
	MIDICTRL_MuteCommEn(pressedButtons.midiMuteComm);
	
	// FOOTSWITCH handling
	
	if(protectionInterval_cnt == 0)
	{		
		//=============BTN CH1==========================
		if(pressedButtons.btnCh1 == BT_ON)
		{
			protectionInterval_cnt = PROTECTION_INTERVAL;
			if(swProgrammingMode_cnt == SWITCH_MODE_DELAY)
			{
				swProgrammingMode_cnt = 0;
				//enter prog mode, or save settings
				if(MIDICTRL_MidiMode() == RUNNING)
				{
					MIDICTRL_SwitchMode(PROGRAMMING);
					SH100CTRL_MuteAmp();
				}
				else
				{
					MIDICTRL_StoreUserCommands();
					SH100CTRL_SetAmpLeds();
					SH100CTRL_UnmuteAmp();
				}
			}
			else
			{		
				if(swProgrammingMode_cnt == 0)
				{
					switch(MIDICTRL_MidiMode())
					{
						case RUNNING: SH100CTRL_BtnSwChannel(SH100_CHANNEL1); break;
						case PROGRAMMING: MIDICTRL_SetProgrammingButton(MIDI_PROG_BTN_CH1); break;
					}
				}
				swProgrammingMode_cnt++;
			}
			return;
		}
		else swProgrammingMode_cnt=0;	
		
		//=============BTN CH2==========================
		if(pressedButtons.btnCh2 == BT_ON)
		{
			protectionInterval_cnt = PROTECTION_INTERVAL;
			switch(MIDICTRL_MidiMode())
			{
				case RUNNING: SH100CTRL_BtnSwChannel(SH100_CHANNEL2); break;
				case PROGRAMMING: MIDICTRL_SetProgrammingButton(MIDI_PROG_BTN_CH2); break;
			}
			return;
		}
		
		//=============BTN CH3==========================
		if(pressedButtons.btnCh3 == BT_ON)
		{
			protectionInterval_cnt = PROTECTION_INTERVAL;
			switch(MIDICTRL_MidiMode())
			{
				case RUNNING: SH100CTRL_BtnSwChannel(SH100_CHANNEL3); break;
				case PROGRAMMING: MIDICTRL_SetProgrammingButton(MIDI_PROG_BTN_CH3); break;
			}
			return;
		}
		
		//=============BTN CH4==========================
		if(pressedButtons.btnCh4 == BT_ON)
		{
			protectionInterval_cnt = PROTECTION_INTERVAL;
			switch(MIDICTRL_MidiMode())
			{
				case RUNNING: SH100CTRL_BtnSwChannel(SH100_CHANNEL4); break;
				case PROGRAMMING: MIDICTRL_SetProgrammingButton(MIDI_PROG_BTN_CH4); break;
			}
			return;
		}
		
		//=============BTN LOOP=========================
		if(pressedButtons.btnLoop == BT_ON)
		{
			protectionInterval_cnt = PROTECTION_INTERVAL;
			switch(MIDICTRL_MidiMode())
			{
				case RUNNING:
				{
					SH100CTRL_SwLoop();
					break;
				}
				case PROGRAMMING: MIDICTRL_SetProgrammingButton(MIDI_PROG_BTN_LOOP); break;
			}
			return;
		}
			
		//=============BTN AB===========================
		if(pressedButtons.btnAB == BT_ON)
		{
			protectionInterval_cnt = PROTECTION_INTERVAL;
			if(leaveDefSettings_cnt == SWITCH_MODE_DELAY)
			{
				if(MIDICTRL_MidiMode() == PROGRAMMING)
				{
					leaveDefSettings_cnt = 0;
					MIDICTRL_DiscardCommands();
					SH100CTRL_SetAmpLeds();
					SH100CTRL_UnmuteAmp();
				}
			}
			else
			{
				leaveDefSettings_cnt++;
				switch(MIDICTRL_MidiMode())
				{
					case RUNNING: 
					{
						SH100CTRL_SwAB(); 
						break;
					}
					case PROGRAMMING: MIDICTRL_SetProgrammingButton(MIDI_PROG_BTN_AB); break;
				}
			}
			return;
		}
		else leaveDefSettings_cnt=0;
	}
	else
	{
		protectionInterval_cnt--;
	}
}

ISR(PCINT1_vect)
{
	// Power off
	// need to check pin level?
	SH100CTRL_StoreAmpState();
}