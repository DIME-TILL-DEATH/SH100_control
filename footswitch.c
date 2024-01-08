#include <avr/eeprom.h>
#include <asf.h>

#include "sh100_memory.h"
#include "sh100_controller.h"
#include "footswitch.h"

#include "midi_controller.h"

FSW_SwitchMode_t switchMode;
SH100HW_Controls_t ctrlsPrevState;
uint8_t protectionInterval_cnt;

void FSW_Init()
{
	switchMode = eeprom_read_byte((uint8_t*)MEMORY_FSW_MODE_OFFSET);
	
	if(switchMode > 2) switchMode = 0;
	
	ctrlsPrevState = SH100HW_GetControlsState(false);
	protectionInterval_cnt = 0;
}

void FSW_SetMode(FSW_SwitchMode_t newSwitchMode)
{
	switchMode = newSwitchMode;
	eeprom_write_byte((uint8_t*)MEMORY_FSW_MODE_OFFSET, switchMode);
}

#define FSW_PROTECTION_INTERVAL 50
uint8_t zzCh12 = SH100_CHANNEL1;
uint8_t zzCh34 = SH100_CHANNEL3;
void FSW_MainTask(const SH100HW_Controls_t* activatedCtrls)
{
	//if(MIDICTRL_MidiMode() == PROGRAMMING) return;
	
	if(protectionInterval_cnt == 0)
	{		
		if (activatedCtrls->FS1_presence == FSW_PRESENT)
		{
			switch(switchMode)
			{	
				case FSW_DISCRET:
				{
					uint8_t desiredChannel = SH100CTRL_GetAmpState().channelNum;
					if(ctrlsPrevState.FS1_tip != activatedCtrls->FS1_tip) desiredChannel = 0;
					else if(ctrlsPrevState.FS1_sleeve != activatedCtrls->FS1_sleeve) desiredChannel = 1;
					else return;
					
					if(SH100CTRL_GetAmpState().channelNum != desiredChannel)
					{
						SH100CTRL_FsSetChannel(desiredChannel); // send midi comm only once
					}
					else
					{
						SH100CTRL_SwLoop();
						if(MIDICTRL_MidiMode() == PROGRAMMING) MIDICTRL_SetProgrammingButton(MIDI_PROG_BTN_LOOP);
					}
					break;
				}	
				case FSW_RELAY:
				{
					if((ctrlsPrevState.FS1_tip == activatedCtrls->FS1_tip) && (ctrlsPrevState.FS1_sleeve == activatedCtrls->FS1_sleeve)) return; //There is no changes
							
					uint8_t channelNum = (!(activatedCtrls->FS1_sleeve) << 1) | (!(activatedCtrls->FS1_tip));
			
					if(SH100CTRL_GetAmpState().channelNum != channelNum)
					{
						SH100CTRL_FsSetChannel(channelNum); // send midi comm only once
					}		
					break;	
				}
				case FSW_RING:
				{
					uint8_t currentChannel = SH100CTRL_GetAmpState().channelNum;
					if(ctrlsPrevState.FS1_tip != activatedCtrls->FS1_tip)
					{
						protectionInterval_cnt = FSW_PROTECTION_INTERVAL;
											
						if(currentChannel > SH100_CHANNEL1) SH100CTRL_FsSetChannel(currentChannel-1);
						else SH100CTRL_FsSetChannel(SH100_CHANNEL4);
					}
			
					if(ctrlsPrevState.FS1_sleeve != activatedCtrls->FS1_sleeve)
					{
						protectionInterval_cnt = FSW_PROTECTION_INTERVAL;
						
						if(currentChannel < SH100_CHANNEL4) SH100CTRL_FsSetChannel(currentChannel+1);
						else SH100CTRL_FsSetChannel(SH100_CHANNEL1);
					}
					break;
				}
				case FSW_ZIGZAG:
				{
					uint8_t currentChannel = SH100CTRL_GetAmpState().channelNum;
					if(ctrlsPrevState.FS1_tip != activatedCtrls->FS1_tip)
					{
						protectionInterval_cnt = FSW_PROTECTION_INTERVAL;
						if(currentChannel < SH100_CHANNEL3)
						{
							zzCh12 = (currentChannel == SH100_CHANNEL1) ? SH100_CHANNEL2 : SH100_CHANNEL1;
						}					
						SH100CTRL_FsSetChannel(zzCh12);
					}
				
					if(ctrlsPrevState.FS1_sleeve != activatedCtrls->FS1_sleeve)
					{
						protectionInterval_cnt = FSW_PROTECTION_INTERVAL;
						if(currentChannel > SH100_CHANNEL2)
						{
							zzCh34 = (currentChannel == SH100_CHANNEL3) ? SH100_CHANNEL4 : SH100_CHANNEL3;
						}
						SH100CTRL_FsSetChannel(zzCh34);
					}
					break;
				}
			}
		}

		if (activatedCtrls->FS2_presence == FSW_PRESENT)
		{
			switch(switchMode)
			{	
				case FSW_DISCRET:
				{
					uint8_t desiredChannel = SH100CTRL_GetAmpState().channelNum;
					if(ctrlsPrevState.FS2_tip != activatedCtrls->FS2_tip) desiredChannel = 2;
					else if(ctrlsPrevState.FS2_sleeve != activatedCtrls->FS2_sleeve) desiredChannel = 3;
					else return;
					
					if(SH100CTRL_GetAmpState().channelNum != desiredChannel)
					{
						SH100CTRL_FsSetChannel(desiredChannel); // send midi comm only once
					}
					else
					{
						SH100CTRL_SwLoop();
						if(MIDICTRL_MidiMode() == PROGRAMMING) MIDICTRL_SetProgrammingButton(MIDI_PROG_BTN_LOOP);
					}
					break;
				}	
				default:
				{
					if(ctrlsPrevState.FS2_sleeve != activatedCtrls->FS2_sleeve)
					{
						protectionInterval_cnt = FSW_PROTECTION_INTERVAL;
						SH100CTRL_SwLoop();
						if(MIDICTRL_MidiMode() == PROGRAMMING) MIDICTRL_SetProgrammingButton(MIDI_PROG_BTN_LOOP);
					}
		
					if (ctrlsPrevState.FS2_tip != activatedCtrls->FS2_tip)
					{
						protectionInterval_cnt = FSW_PROTECTION_INTERVAL;
						SH100CTRL_SwAB();
						if(MIDICTRL_MidiMode() == PROGRAMMING) 
						if(MIDICTRL_MidiMode() == PROGRAMMING) MIDICTRL_SetProgrammingButton(MIDI_PROG_BTN_AB);
					}
				}
			}
		}			
	}
	else
	{
		protectionInterval_cnt--;
	}	
	
	ctrlsPrevState = *activatedCtrls;	
}

bool FSW_BlockFrontControls()
{
	return ((switchMode == FSW_RELAY) && (ctrlsPrevState.FS1_presence == FSW_PRESENT));
}