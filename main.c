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
	
	cpu_irq_enable();
	
    while(1)
    {
		MIDI_ParserTask();
	}
}

uint8_t swProgrammingMode_cnt = 0;
uint8_t leaveDefSettings_cnt = 0;

ISR(TIMER0_OVF_vect)
{
	SH100HW_MainTask();
	
	SH100HW_Buttons_t pressedButtons = SH100HW_GetButtonsState();
	
	MIDICTRL_SetMidiChannel(pressedButtons.midiChNum);
	MIDICTRL_OmniModeEn(pressedButtons.midiOmni);
	MIDICTRL_MuteCommEn(pressedButtons.midiMuteComm);
	
	//=============BTN AB===========================
	if(pressedButtons.btnAB == BT_ON)
	{
		if(leaveDefSettings_cnt == 200)
		{
			leaveDefSettings_cnt = 0;
			MIDICTRL_DiscardCommands();
			SH100CTRL_SetAmpLeds();
			// unmute amp
		}
		else
		{
			leaveDefSettings_cnt++;
			switch(MIDICTRL_MidiMode())
			{
				case RUNNING: SH100CTRL_SwAB(); break;
				case PROGRAMMING: MIDICTRL_SetProgrammingButton(MIDI_PROG_BTN_AB); break;
			}
		}
	}
	else leaveDefSettings_cnt=0;

	//=============BTN LOOP=========================
	if(pressedButtons.btnCh2 == BT_ON)
	{
		switch(MIDICTRL_MidiMode())
		{
			case RUNNING: SH100CTRL_SwLoop(); break;
			case PROGRAMMING: MIDICTRL_SetProgrammingButton(MIDI_PROG_BTN_LOOP); break;
		}
	}
		
	//=============BTN CH4==========================
	if(pressedButtons.btnCh2 == BT_ON)
	{
		switch(MIDICTRL_MidiMode())
		{
			case RUNNING: SH100CTRL_SwChannel(SH100_CHANNEL4); break;
			case PROGRAMMING: MIDICTRL_SetProgrammingButton(MIDI_PROG_BTN_CH4); break;
		}
	}
		
	//=============BTN CH3==========================
	if(pressedButtons.btnCh2 == BT_ON)
	{
		switch(MIDICTRL_MidiMode())
		{
			case RUNNING: SH100CTRL_SwChannel(SH100_CHANNEL3); break;
			case PROGRAMMING: MIDICTRL_SetProgrammingButton(MIDI_PROG_BTN_CH3); break;
		}
	}
		
	//=============BTN CH2==========================
	if(pressedButtons.btnCh2 == BT_ON)
	{
		switch(MIDICTRL_MidiMode())
		{
			case RUNNING: SH100CTRL_SwChannel(SH100_CHANNEL2); break;
			case PROGRAMMING: MIDICTRL_SetProgrammingButton(MIDI_PROG_BTN_CH2); break;
		}
	}
	
	//=============BTN CH1==========================
	if(pressedButtons.btnCh1 == BT_ON)
	{
		if(swProgrammingMode_cnt == 200)
		{
			swProgrammingMode_cnt = 0;
			//enter prog mode, or save settings
			if(MIDICTRL_MidiMode() == RUNNING)
			{
				MIDICTRL_SwitchMode(PROGRAMMING);
				//mute amp
			}
			else
			{
				MIDICTRL_StoreUserCommands();
				SH100CTRL_SetAmpLeds();
				// unmute amp
			}
		}
		else
		{
			swProgrammingMode_cnt++;
			switch(MIDICTRL_MidiMode()) 
			{
				case RUNNING: SH100CTRL_SwChannel(SH100_CHANNEL1); break;
				case PROGRAMMING: MIDICTRL_SetProgrammingButton(MIDI_PROG_BTN_CH1); break;
			}			
		}		
	}
	else swProgrammingMode_cnt=0;
}