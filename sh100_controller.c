#include <avr/eeprom.h>

#include "midi_controller.h"

#include "sh100_memory.h"
#include "sh100_controller.h"
#include "sh100_hardware.h"

SH100_State_t ampState;

void setChannelLeds();

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
		ampState.channelNum = 0;
		for(int i=0; i<4; i++)
		{
			ampState.loopOn[i] = false;
		}		
		ampState.swAB = false;		
	}
	
	SH100CTRL_SetAmpState(ampState);
}

void SH100CTRL_SetAmpState(SH100_State_t state)
{
	ampState = state;
	
	SH100HW_SetCh(ampState.channelNum);
	SH100HW_LoopEn(ampState.loopOn[ampState.channelNum]);
	SH100HW_SetAB(ampState.swAB);
	
	setChannelLeds();
	SH100HW_SetNewLedState(LED_LOOP, ampState.loopOn[ampState.channelNum]);
	SH100HW_SetNewLedState(LED_A, !ampState.swAB);
	SH100HW_SetNewLedState(LED_B, ampState.swAB);
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
	SH100HW_SetNewLedState(LED_A, !ampState.swAB);
	SH100HW_SetNewLedState(LED_B, ampState.swAB);
}

void SH100CTRL_StoreAmpState()
{
	eeprom_write_word(0x00, MEMORY_MAGIC_WORD);
	eeprom_write_block(&ampState, (void*)MEMORY_AMP_STATE_OFFSET, sizeof(SH100_State_t));
}

void SH100CTRL_BtnSwChannel(uint8_t chNum)
{
	if(ampState.channelNum != chNum)
	{
		SH100CTRL_SwChannel(chNum);
	}
	else
	{
		SH100CTRL_SwLoop();
	}
}

void SH100CTRL_SwChannel(uint8_t chNum)
{
	ampState.channelNum = chNum;
	
	SH100HW_SetCh(chNum);
	SH100HW_LoopEn(ampState.loopOn[ampState.channelNum]);
	setChannelLeds();
}

void SH100CTRL_SetLoop(bool en)
{
	ampState.loopOn[ampState.channelNum] = en;
	SH100HW_LoopEn(ampState.loopOn[ampState.channelNum]);
	SH100HW_SetNewLedState(LED_LOOP, ampState.loopOn[ampState.channelNum]);
}

void SH100CTRL_SetAB(bool isB)
{
	ampState.swAB = isB;
	SH100HW_SetAB(ampState.swAB);
	SH100HW_SetNewLedState(LED_A, !ampState.swAB);
	SH100HW_SetNewLedState(LED_B, ampState.swAB);
}

void SH100CTRL_SwLoop()
{
	SH100CTRL_SetLoop(!ampState.loopOn);
}

void SH100CTRL_SwAB()
{
	SH100CTRL_SetAB(!ampState.swAB);
}

void SH100CTRL_MuteAmp()
{
	SH100HW_SetPAState(OUTPUT_MUTE);
}

void SH100CTRL_UnmuteAmp()
{
	if(SH100HW_GetOutputJacks() != OUT_NONE)
	{
		SH100HW_SetPAState(OUTPUT_ENABLED);	
	}		
}

void SH100CTRL_CheckOutputJacks()
{
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
			SH100CTRL_MuteAmp();
			break;
		}
		case OUT_16OHM:
		{
			if(MIDICTRL_MidiMode() == RUNNING)
			{
				SH100CTRL_UnmuteAmp();
				SH100HW_SetOutputMode(OUTPUT_16OHM);
				SH100HW_SetNewLedState(LED_PWR_GRN, LED_ON);
			}			
			break;
		}
		case OUT_8OHM:
		{
			if(MIDICTRL_MidiMode() == RUNNING)
			{
				SH100CTRL_UnmuteAmp();
				SH100HW_SetOutputMode(OUTPUT_8OHM);
				SH100HW_SetNewLedState(LED_PWR_GRN, LED_ON);
			}

			break;
		}
		case OUT_BOTH:
		{
			if(MIDICTRL_MidiMode() == RUNNING)
			{
				SH100HW_SetNewLedState(LED_PWR_GRN, LED_SLOW_BLINKING);
			}
			//SH100CTRL_MuteAmp(); //can load be less than 8Ohm?
			break;
		}
	}
}