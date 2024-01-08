#include <asf.h>

#include "midi_parser.h"
#include "midi_controller.h"

#include "sh100_hardware.h"
#include "sh100_controller.h"

#include "front_buttons.h"
#include "footswitch.h"

#define BTN_PROTECTION_INTERVAL 20
#define SWITCH_MODE_DELAY 200/BTN_PROTECTION_INTERVAL

uint8_t factoryReset_cnt = 0;
uint8_t protectionInterval_cnt = BTN_PROTECTION_INTERVAL;

uint8_t swProgrammingMode_cnt = 0;
uint8_t leaveDefSettings_cnt = 0;

uint8_t setFSWRingMode_cnt = 0;
uint8_t setFSWZigzagMode_cnt = 0;
uint8_t setFSWRelayMode_cnt = 0;
uint8_t setFSWDiscretMode_cnt = 0;
uint8_t swAuxFollowMode_cnt = 0;

struct 
{
	bool shortCh1 : 1;
	bool shortCh2 : 1;
	bool shortCh3 : 1;
	bool shortCh4 : 1;
	bool shortLoop : 1;
	bool shortAb : 1;
	
	bool longCh1 : 1;
	bool longCh2 : 1;
	bool longCh3 : 1;
	bool longCh4 : 1;
	bool longLoop : 1;
	bool longAb : 1;
}pressProcessed;

void changeMidiProgrammingMode()
{
	//enter prog mode, or save settings
	if(MIDICTRL_MidiMode() == RUNNING)
	{
		MIDICTRL_EnterProgrammingMode();
	}
	else
	{
		MIDICTRL_StoreUserCommands();
		SH100CTRL_SetAmpLeds();
	}
}

void discardMidiProgrammingMode()
{
	if(MIDICTRL_MidiMode() == PROGRAMMING)
	{
		MIDICTRL_DiscardCommands();
		SH100CTRL_SetAmpLeds();
		SH100CTRL_UnmuteAmp();
	}
}



bool factoryReset = false;
void FBTNS_MainTask(const SH100HW_Controls_t* pressedButtons)
{
	if(protectionInterval_cnt == 0)
	{
		if(!isAmpStarted)
		{
			if((pressedButtons->btnAB == BT_ON) && (pressedButtons->btnLoop == BT_ON) && (!factoryReset))
			{
				protectionInterval_cnt = BTN_PROTECTION_INTERVAL;
				if(factoryReset_cnt == SWITCH_MODE_DELAY*2)
				{
					factoryReset = false;
					MIDICTRL_FactoryReset();
					SH100CTRL_FactoryReset();
					
					for (int i=0; i<LED_COUNT; i++)
					{
						SH100HW_SetLedBlinkCount(i, 4);
					}
				}
				factoryReset_cnt++;
			}
			else factoryReset_cnt = 0;
			return;
		}
		//************STARTED***************************
		
		//============BTN LOOP+AB=======================
		if((pressedButtons->btnLoop == BT_ON) && (pressedButtons->btnAB == BT_ON))
		{
			protectionInterval_cnt = BTN_PROTECTION_INTERVAL;
			if(swProgrammingMode_cnt == SWITCH_MODE_DELAY)
			{
				changeMidiProgrammingMode();
				setFSWDiscretMode_cnt=0;
				
				pressProcessed.longLoop = true;
				pressProcessed.longAb = true;
			}
			swProgrammingMode_cnt++;
			return;
		}
		else
		{
			swProgrammingMode_cnt=0;
		}
			
		//=============BTN CH1==========================
		if(pressedButtons->btnCh1 == BT_ON)
		{
			protectionInterval_cnt = BTN_PROTECTION_INTERVAL;
			if((setFSWDiscretMode_cnt == SWITCH_MODE_DELAY) && !pressProcessed.longCh1)
			{
				pressProcessed.longCh1 = true;
				if(MIDICTRL_MidiMode() == RUNNING)
				{
					FSW_SetMode(FSW_DISCRET);
					SH100HW_SetLedBlinkCount(LED_CH1, 2);
				}	
			}
			
			if(!pressProcessed.shortCh1)
			{
				pressProcessed.shortCh1 = true;
				SH100CTRL_BtnSetChannel(SH100_CHANNEL1);
				if(MIDICTRL_MidiMode() == PROGRAMMING) MIDICTRL_SetProgrammingButton(MIDI_PROG_BTN_CH1);
			}
			setFSWDiscretMode_cnt++;			
			return;
		}
		else 
		{
			setFSWDiscretMode_cnt=0;
			pressProcessed.shortCh1 = false;
			pressProcessed.longCh1 = false;
		}
		
		//=============BTN CH2==========================
		if(pressedButtons->btnCh2 == BT_ON)
		{
			protectionInterval_cnt = BTN_PROTECTION_INTERVAL;
			if((setFSWRelayMode_cnt == SWITCH_MODE_DELAY) && !pressProcessed.longCh2)
			{
				if(MIDICTRL_MidiMode() == RUNNING)
				{
					FSW_SetMode(FSW_RELAY);
					SH100HW_SetLedBlinkCount(LED_CH2, 2);
				}				
			}
			
			if(!pressProcessed.shortCh2)
			{
				pressProcessed.shortCh2 = true;
				SH100CTRL_BtnSetChannel(SH100_CHANNEL2);
				if(MIDICTRL_MidiMode() == PROGRAMMING) MIDICTRL_SetProgrammingButton(MIDI_PROG_BTN_CH2);
			}
			setFSWRelayMode_cnt++;
			return;
		}
		else 
		{
			setFSWRelayMode_cnt = 0;
			pressProcessed.shortCh2 = false;
			pressProcessed.longCh2 = false;
		}
		
		//=============BTN CH3==========================
		if(pressedButtons->btnCh3 == BT_ON)
		{
			protectionInterval_cnt = BTN_PROTECTION_INTERVAL;
			if((setFSWRingMode_cnt == SWITCH_MODE_DELAY) && !pressProcessed.longCh3)
			{
				pressProcessed.longCh3 = true;
				if(MIDICTRL_MidiMode() == RUNNING)
				{
					FSW_SetMode(FSW_RING);
					SH100HW_SetLedBlinkCount(LED_CH3, 2);
				}
			}
			if(!pressProcessed.shortCh3)
			{
				pressProcessed.shortCh3 = true;
				SH100CTRL_BtnSetChannel(SH100_CHANNEL3);
				if(MIDICTRL_MidiMode() == PROGRAMMING) MIDICTRL_SetProgrammingButton(MIDI_PROG_BTN_CH3);			
			}
			setFSWRingMode_cnt++;
			return;			
		}
		else 
		{
			setFSWRingMode_cnt = 0;
			pressProcessed.shortCh3 = false;
			pressProcessed.longCh3 = false;
		}
		
		//=============BTN CH4==========================
		if(pressedButtons->btnCh4 == BT_ON)
		{
			protectionInterval_cnt = BTN_PROTECTION_INTERVAL;
			if((setFSWZigzagMode_cnt == SWITCH_MODE_DELAY) && !pressProcessed.longCh4)
			{
				pressProcessed.longCh4 = true;
				if(MIDICTRL_MidiMode() == RUNNING)
				{
					FSW_SetMode(FSW_ZIGZAG);
					SH100HW_SetLedBlinkCount(LED_CH4, 2);
				}
			}
			
			if(!pressProcessed.shortCh4)
			{
				pressProcessed.shortCh4 = true;
				SH100CTRL_BtnSetChannel(SH100_CHANNEL4);
				if(MIDICTRL_MidiMode() == PROGRAMMING) MIDICTRL_SetProgrammingButton(MIDI_PROG_BTN_CH4);
			}
			setFSWZigzagMode_cnt++;	
			return;
		}
		else 
		{
			setFSWZigzagMode_cnt = 0;
			pressProcessed.shortCh4 = false;
			pressProcessed.longCh4 = false;
		}
		
		//=============BTN LOOP=========================
		if(pressedButtons->btnLoop == BT_ON)
		{
			protectionInterval_cnt = BTN_PROTECTION_INTERVAL;

			if((swAuxFollowMode_cnt == SWITCH_MODE_DELAY) && !pressProcessed.longLoop)
			{
				pressProcessed.longLoop = true;
				if(MIDICTRL_MidiMode() == RUNNING)
				{
					SH100CTRL_SwAuxFollowMode();
					SH100HW_SetLedBlinkCount(LED_LOOP, 2);
				}
				else
				{
					changeMidiProgrammingMode();
				}
			}
			
			if(!pressProcessed.shortLoop)
			{
				pressProcessed.shortLoop = true;
				SH100CTRL_SwLoop();
				if(MIDICTRL_MidiMode() == PROGRAMMING) MIDICTRL_SetProgrammingButton(MIDI_PROG_BTN_LOOP);
			}
			swAuxFollowMode_cnt++;
			return;
		}
		else 
		{
			swAuxFollowMode_cnt = 0;
			swProgrammingMode_cnt = 0;
			pressProcessed.shortLoop = false;
			pressProcessed.longLoop = false;
		}
		
		//=============BTN AB===========================
		if(pressedButtons->btnAB == BT_ON)
		{
			protectionInterval_cnt = BTN_PROTECTION_INTERVAL;
			if((leaveDefSettings_cnt == SWITCH_MODE_DELAY) && !pressProcessed.longAb)
			{
				pressProcessed.longAb = true;
				discardMidiProgrammingMode();	
			}
			
			if(!pressProcessed.shortAb)
			{			
				pressProcessed.shortAb = true;		
				SH100CTRL_SwAB();
				if(MIDICTRL_MidiMode() == PROGRAMMING) MIDICTRL_SetProgrammingButton(MIDI_PROG_BTN_AB);
			}
			leaveDefSettings_cnt++;
			return;
		}
		else 
		{
			leaveDefSettings_cnt=0;
			pressProcessed.shortAb = false;
			pressProcessed.longAb = false;
		}
	}
	else
	{
		protectionInterval_cnt--;
	}
}