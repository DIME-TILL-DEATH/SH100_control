#include <avr/eeprom.h>

#include "midi_controller.h"

#include "sh100_memory.h"
#include "sh100_controller.h"
#include "sh100_hardware.h"

SH100_State_t ampState;
bool ampMuted = false;

void setChannelLeds();

void SH100CTRL_setDefaultAmpState()
{
	ampState.auxFollowChannel = true;
	ampState.channelNum = 0;
	ampState.loopAll = false;
	for(int i=0; i<4; i++)
	{
		ampState.loopOn[i] = false;
		ampState.swAB[i] = false;
	}
}

void SH100CTRL_Init()
{
	uint16_t readedMagicWord = eeprom_read_word(0x00);
	
	if(readedMagicWord == MEMORY_MAGIC_WORD)
	{
		uint8_t readedData[sizeof(SH100_State_t)];
		eeprom_read_block(&readedData, (uint16_t*)MEMORY_AMP_STATE_OFFSET, sizeof(SH100_State_t));
		SH100_State_t* ampState_ptr = (SH100_State_t*)readedData;
		ampState = *ampState_ptr;
	}
	else
	{
		SH100CTRL_setDefaultAmpState();
	}
	SH100CTRL_SetAmpState(&ampState);
}

void SH100CTRL_FactoryReset()
{
	SH100CTRL_setDefaultAmpState();
	eeprom_write_word(0x00, 0xFFFF);
}

void SH100CTRL_SwAuxFollowMode()
{
	ampState.auxFollowChannel = !ampState.auxFollowChannel;
}

void SH100CTRL_SetAmpState(const SH100_State_t* state)
{
	ampState = *state;
	
	SH100CTRL_SetChannelExclusive(ampState.channelNum);
	SH100CTRL_SetLoop(ampState.auxFollowChannel ? ampState.loopOn[ampState.channelNum] : ampState.loopAll);
	SH100CTRL_SetAB(ampState.swAB[ampState.channelNum]);
	SH100CTRL_SetAmpLeds();
}

SH100_State_t SH100CTRL_GetAmpState()
{
	return ampState;
}

void setChannelLeds()
{
	SH100HW_SetNewLedState(LED_CH1, LED_OFF);
	SH100HW_SetNewLedState(LED_CH2, LED_OFF);
	SH100HW_SetNewLedState(LED_CH3, LED_OFF);
	SH100HW_SetNewLedState(LED_CH4, LED_OFF);
	
	SH100HW_SetNewLedState(ampState.channelNum, LED_ON);
}

void SH100CTRL_SetAmpLeds()
{
	setChannelLeds();
	SH100HW_SetNewLedState(LED_LOOP, ampState.loopOn[ampState.channelNum]);
	SH100HW_SetNewLedState(LED_A, !ampState.swAB[ampState.channelNum]);
	SH100HW_SetNewLedState(LED_B, ampState.swAB[ampState.channelNum]);
}

void SH100CTRL_StoreAmpState()
{
	eeprom_write_word(0x00, MEMORY_MAGIC_WORD);
	eeprom_write_block(&ampState, (void*)MEMORY_AMP_STATE_OFFSET, sizeof(SH100_State_t));
}

void SH100CTRL_FsSetChannel(uint8_t chNum)
{
	SH100CTRL_BtnSetChannel(chNum);
	//if(MIDICTRL_MidiMode() == PROGRAMMING) MIDICTRL_SetProgrammingButton(chNum);
}

void SH100CTRL_BtnSetChannel(uint8_t chNum)
{
	if(ampState.channelNum != chNum)
	{
		SH100CTRL_SetChannel(chNum);
		MIDICTRL_SendSwChComm(chNum);
	}
	else
	{
		SH100CTRL_SwLoop();
	}
}

void SH100CTRL_SetChannel(uint8_t chNum)
{
	SH100CTRL_SetChannelExclusive(chNum);
	if(ampState.auxFollowChannel)
	{
		SH100CTRL_SetLoop(ampState.loopOn[ampState.channelNum]);
		SH100CTRL_SetAB(ampState.swAB[ampState.channelNum]);
	}
}

void SH100CTRL_SetChannelExclusive(uint8_t chNum)
{
	ampState.channelNum = chNum;
	SH100HW_SetCh(chNum);
	if(MIDICTRL_MidiMode() == RUNNING) setChannelLeds();
}

void SH100CTRL_SwLoop()
{
	SH100CTRL_SetLoop(!(ampState.auxFollowChannel ? ampState.loopOn[ampState.channelNum] : ampState.loopAll));
	MIDICTRL_SendLoopEnComm(!ampState.loopOn[ampState.channelNum]);
}

void SH100CTRL_SetLoop(bool en)
{
	if(ampState.auxFollowChannel)
	{
		ampState.loopOn[ampState.channelNum] = en;
		SH100HW_LoopEn(ampState.loopOn[ampState.channelNum]);	
		if(MIDICTRL_MidiMode() == RUNNING)  SH100HW_SetNewLedState(LED_LOOP, ampState.loopOn[ampState.channelNum]);
	}
	else
	{
		ampState.loopAll = en;
		SH100HW_LoopEn(ampState.loopAll);
		if(MIDICTRL_MidiMode() == RUNNING)  SH100HW_SetNewLedState(LED_LOOP, ampState.loopAll);
	}
}

void SH100CTRL_SwAB()
{
	SH100CTRL_SetAB(!ampState.swAB[ampState.channelNum]);
	MIDICTRL_SendSwABComm(!ampState.swAB[ampState.channelNum]);
}

void SH100CTRL_SetAB(bool isB)
{
	ampState.swAB[ampState.channelNum] = isB;
	SH100HW_SetAB(ampState.swAB[ampState.channelNum]);
	if(MIDICTRL_MidiMode() == RUNNING)
	{
		SH100HW_SetNewLedState(LED_A, !(ampState.swAB[ampState.channelNum]));
		SH100HW_SetNewLedState(LED_B, ampState.swAB[ampState.channelNum]);
	}
}

void SH100CTRL_UnmuteAmp()
{
	if(!ampMuted)
	{
		SH100HW_SetDiMute(OUTPUT_ENABLED);
		if(SH100HW_GetOutputJacks() != OUT_NONE)
		{
			SH100HW_SetPAState(OUTPUT_ENABLED);
		}
		
	}
}

void SH100CTRL_SetMuteAmp(bool mute)
{
	ampMuted = mute;
	if(mute)
	{
		SH100HW_SetPAState(OUTPUT_MUTE);
		SH100HW_SetDiMute(OUTPUT_MUTE);
	}
	else SH100CTRL_UnmuteAmp();
}

void SH100CTRL_CheckOutputJacks()
{
	if(SH100HW_GetPAFailure()) return;
	
	SH100HW_OutputJacks_t outJacksState = SH100HW_GetOutputJacks();
	
	switch(outJacksState)
	{
		case OUT_NONE:
		{
			if(MIDICTRL_MidiMode() == RUNNING)
			{
				SH100HW_SetNewLedState(LED_PWR_GRN, LED_OFF);
				SH100HW_SetNewLedState(LED_PWR_RED, LED_ON);
			}
			SH100HW_SetOutputMode(OUTPUT_8OHM);
			SH100HW_SetPAState(OUTPUT_MUTE);
			break;
		}
		case OUT_16OHM:
		{
			if(MIDICTRL_MidiMode() == RUNNING)
			{
				SH100HW_SetNewLedState(LED_PWR_GRN, LED_ON);
				SH100HW_SetNewLedState(LED_PWR_RED, LED_OFF);
			}		
			SH100HW_SetOutputMode(OUTPUT_16OHM);
			if(!ampMuted) SH100HW_SetPAState(OUTPUT_ENABLED);
			break;
		}
		case OUT_8OHM:
		{
			if(MIDICTRL_MidiMode() == RUNNING)
			{
				SH100HW_SetNewLedState(LED_PWR_GRN, LED_ON);
				SH100HW_SetNewLedState(LED_PWR_RED, LED_OFF);
			}		
			SH100HW_SetOutputMode(OUTPUT_8OHM);
			if(!ampMuted) SH100HW_SetPAState(OUTPUT_ENABLED);
			break;
		}
		case OUT_BOTH:
		{
			if(MIDICTRL_MidiMode() == RUNNING)
			{
				SH100HW_SetNewLedState(LED_PWR_GRN, LED_OFF);
				SH100HW_SetNewLedState(LED_PWR_RED, LED_ON);
			}
			SH100HW_SetOutputMode(OUTPUT_8OHM);
			SH100HW_SetPAState(OUTPUT_MUTE);
			break;
		}
	}
}