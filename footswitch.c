#include <avr/eeprom.h>
#include <asf.h>

#include "sh100_memory.h"
#include "sh100_controller.h"
#include "footswitch.h"

FSW_SwitchMode_t switchMode;
SH100HW_Controls_t ctrlsPrevState;
uint8_t protectionInterval_cnt;

void FSW_Init()
{
	switchMode = eeprom_read_byte((uint8_t*)MEMORY_FSW_MODE_OFFSET);
	
	if(switchMode > 2) switchMode = 0;
	
	ctrlsPrevState = SH100HW_GetControlsState();
	protectionInterval_cnt = 0;
}

void FSW_SetMode(FSW_SwitchMode_t newSwitchMode)
{
	switchMode = newSwitchMode;
	eeprom_write_byte((uint8_t*)MEMORY_FSW_MODE_OFFSET, switchMode);
}

#define FSW_PROTECTION_INTERVAL 20
uint8_t zzCh12 = SH100_CHANNEL1;
uint8_t zzCh34 = SH100_CHANNEL3;
void FSW_MainTask(const SH100HW_Controls_t* activatedCtrls)
{
	if(protectionInterval_cnt == 0)
	{		
		if (activatedCtrls->FS1_presence == BT_ON)
		{
			switch(switchMode)
			{		
				case FSW_RELAY:
				{
					uint8_t channelNum = (!(activatedCtrls->FS1_sleeve) << 1) | (!(activatedCtrls->FS1_tip));
			
					if(SH100CTRL_GetAmpState().channelNum != channelNum)
					{
						SH100CTRL_SetChannel(channelNum); // send midi comm only once
					}		
					break;	
				}
				case FSW_RING:
				{
					uint8_t currentChannel = SH100CTRL_GetAmpState().channelNum;
					if(ctrlsPrevState.FS1_tip != activatedCtrls->FS1_tip)
					{
						protectionInterval_cnt = FSW_PROTECTION_INTERVAL;
											
						if(currentChannel > SH100_CHANNEL1) SH100CTRL_SetChannel(currentChannel-1);
						else SH100CTRL_SetChannel(SH100_CHANNEL4);
					}
			
					if(ctrlsPrevState.FS1_sleeve != activatedCtrls->FS1_sleeve)
					{
						protectionInterval_cnt = FSW_PROTECTION_INTERVAL;
						
						if(currentChannel < SH100_CHANNEL4) SH100CTRL_SetChannel(currentChannel+1);
						else SH100CTRL_SetChannel(SH100_CHANNEL1);
					}
					break;
				}
				case FSW_ZIGZAG:
				{
					uint8_t currentChannel = SH100CTRL_GetAmpState().channelNum;
					if(ctrlsPrevState.FS1_tip != activatedCtrls->FS1_tip)
					{
						protectionInterval_cnt = FSW_PROTECTION_INTERVAL;
						if(currentChannel < SH100_CHANNEL2)
						{
							zzCh12 = (currentChannel == SH100_CHANNEL1) ? SH100_CHANNEL2 : SH100_CHANNEL1;
						}					
						SH100CTRL_SetChannel(zzCh12);
					}
				
					if(ctrlsPrevState.FS1_sleeve != activatedCtrls->FS1_sleeve)
					{
						protectionInterval_cnt = FSW_PROTECTION_INTERVAL;
						if(currentChannel > SH100_CHANNEL3)
						{
							zzCh34 = (currentChannel == SH100_CHANNEL3) ? SH100_CHANNEL4 : SH100_CHANNEL3;
						}
						SH100CTRL_SetChannel(zzCh34);
					}
					break;
				}
			}
		}

		if (activatedCtrls->FS2_presence == BT_ON)
		{
			if(ctrlsPrevState.FS2_tip != activatedCtrls->FS2_tip)
			{
				protectionInterval_cnt = FSW_PROTECTION_INTERVAL;
				SH100CTRL_SwLoop();
			}
		
			if (ctrlsPrevState.FS2_sleeve != activatedCtrls->FS2_sleeve)
			{
				protectionInterval_cnt = FSW_PROTECTION_INTERVAL;
				SH100CTRL_SwAB();
			}
		}
	
		ctrlsPrevState = *activatedCtrls;
	}
	else
	{
		protectionInterval_cnt--;
	}
}

bool FSW_BlockFrontControls()
{
	return ((switchMode == FSW_RELAY) && (ctrlsPrevState.FS1_presence == BT_ON));
}