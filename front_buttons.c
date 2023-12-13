#include <asf.h>

#include "midi_parser.h"
#include "midi_controller.h"

#include "sh100_hardware.h"
#include "sh100_controller.h"

#include "front_buttons.h"
#include "footswitch.h"

#define BTN_PROTECTION_INTERVAL 20
#define SWITCH_MODE_DELAY 200/BTN_PROTECTION_INTERVAL

uint8_t protectionInterval_cnt = BTN_PROTECTION_INTERVAL;
uint8_t swProgrammingMode_cnt = 0;
uint8_t leaveDefSettings_cnt = 0;

uint8_t setFSWRingMode_cnt = 0;
uint8_t setFSWZigzagMode_cnt = 0;
uint8_t setFSWRelayMode_cnt = 0;

bool loopSwitched = false;
bool abSwitched = false;

void FBTNS_MainTask(const SH100HW_Controls_t* pressedButtons)
{
	if(protectionInterval_cnt == 0)
	{
		//if(FSW_BlockFrontControls()) return;
		
		//=============BTN CH1==========================
		if(pressedButtons->btnCh1 == BT_ON)
		{
			protectionInterval_cnt = BTN_PROTECTION_INTERVAL;
			if(swProgrammingMode_cnt == SWITCH_MODE_DELAY)
			{
				//enter prog mode, or save settings
				if(MIDICTRL_MidiMode() == RUNNING)
				{
					MIDICTRL_EnterProgrammingMode();
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
						case RUNNING: SH100CTRL_BtnSetChannel(SH100_CHANNEL1); break;
						case PROGRAMMING: MIDICTRL_SetProgrammingButton(MIDI_PROG_BTN_CH1); break;
					}
				}
				
			}
			swProgrammingMode_cnt++;
			return;
		}
		else swProgrammingMode_cnt=0;
		
		//=============BTN CH2==========================
		if(pressedButtons->btnCh2 == BT_ON)
		{
			protectionInterval_cnt = BTN_PROTECTION_INTERVAL;
			if(setFSWRelayMode_cnt == SWITCH_MODE_DELAY)
			{
				if(MIDICTRL_MidiMode() == RUNNING)
				{
					FSW_SetMode(FSW_RELAY);
					SH100HW_SetLedBlinkCount(LED_CH2, 2);
				}				
			}
			else
			{
				if(setFSWRelayMode_cnt == 0)
				{
					switch(MIDICTRL_MidiMode())
					{
						case RUNNING: SH100CTRL_BtnSetChannel(SH100_CHANNEL2); break;
						case PROGRAMMING: MIDICTRL_SetProgrammingButton(MIDI_PROG_BTN_CH2); break;
					}
				}
			}
			setFSWRelayMode_cnt++;
			return;
		}
		else setFSWRelayMode_cnt = 0;
		
		//=============BTN CH3==========================
		if(pressedButtons->btnCh3 == BT_ON)
		{
			protectionInterval_cnt = BTN_PROTECTION_INTERVAL;
			if(setFSWRingMode_cnt == SWITCH_MODE_DELAY)
			{
				if(MIDICTRL_MidiMode() == RUNNING)
				{
					FSW_SetMode(FSW_RING);
					SH100HW_SetLedBlinkCount(LED_CH3, 2);
				}
			}
			else
			{
				if(setFSWRingMode_cnt == 0)
				{
					switch(MIDICTRL_MidiMode())
					{
						case RUNNING: SH100CTRL_BtnSetChannel(SH100_CHANNEL3); break;
						case PROGRAMMING: MIDICTRL_SetProgrammingButton(MIDI_PROG_BTN_CH3); break;
					}
				}				
			}
			setFSWRingMode_cnt++;
			return;			
		}
		else setFSWRingMode_cnt = 0;
		
		//=============BTN CH4==========================
		if(pressedButtons->btnCh4 == BT_ON)
		{
			protectionInterval_cnt = BTN_PROTECTION_INTERVAL;
			if(setFSWZigzagMode_cnt == SWITCH_MODE_DELAY)
			{
				if(MIDICTRL_MidiMode() == RUNNING)
				{
					FSW_SetMode(FSW_ZIGZAG);
					SH100HW_SetLedBlinkCount(LED_CH4, 2);
				}
			}
			else
			{
				if(setFSWZigzagMode_cnt == 0)
				{
					switch(MIDICTRL_MidiMode())
					{
						case RUNNING: SH100CTRL_BtnSetChannel(SH100_CHANNEL4); break;
						case PROGRAMMING: MIDICTRL_SetProgrammingButton(MIDI_PROG_BTN_CH4); break;
					}
				}	
			}
			setFSWZigzagMode_cnt++;	
			return;
		}
		else setFSWZigzagMode_cnt = 0;
		
		//=============BTN LOOP=========================
		if(pressedButtons->btnLoop == BT_ON)
		{
			protectionInterval_cnt = BTN_PROTECTION_INTERVAL;

			switch(MIDICTRL_MidiMode())
			{
				case RUNNING:
				{
					if(!loopSwitched) SH100CTRL_SwLoop();
					loopSwitched = true;
					break;
				}
				case PROGRAMMING: MIDICTRL_SetProgrammingButton(MIDI_PROG_BTN_LOOP); break;
			}
			return;
		}
		else loopSwitched = false;
		
		//=============BTN AB===========================
		if(pressedButtons->btnAB == BT_ON)
		{
			protectionInterval_cnt = BTN_PROTECTION_INTERVAL;
			if(leaveDefSettings_cnt == SWITCH_MODE_DELAY)
			{
				if(MIDICTRL_MidiMode() == PROGRAMMING)
				{
					MIDICTRL_DiscardCommands();
					SH100CTRL_SetAmpLeds();
					SH100CTRL_UnmuteAmp();
				}
			}
			else
			{				
				switch(MIDICTRL_MidiMode())
				{
					case RUNNING:
					{
						if(!abSwitched) SH100CTRL_SwAB();
						abSwitched = true;
						break;
					}
					case PROGRAMMING: MIDICTRL_SetProgrammingButton(MIDI_PROG_BTN_AB); break;
				}
			}
			leaveDefSettings_cnt++;
			return;
		}
		else 
		{
			leaveDefSettings_cnt=0;
			abSwitched = false;
		}
	}
	else
	{
		protectionInterval_cnt--;
	}
}