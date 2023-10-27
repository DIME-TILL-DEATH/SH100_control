#include <avr/eeprom.h>

#include "sh100_memory.h"
#include "sh100_controller.h"
#include "sh100_hardware.h"

SH100_State_t ampState;

void SH100CTRL_Init()
{
	uint16_t readedMagicWord = eeprom_read_word(0x00);
	
	if(readedMagicWord == MEMORY_MAGIC_WORD)
	{
		uint8_t readedData[sizeof(SH100_State_t)];
		eeprom_read_block(&readedData, (uint16_t*)MEMORY_USER_COMMANDS_OFFSET, sizeof(SH100_State_t));
		SH100_State_t* ampState_ptr = (SH100_State_t*)readedData;
		ampState = *ampState_ptr;
	}
	else
	{
		ampState.channelNum = 0;
		ampState.loopOn = false;
		ampState.swAB = false;		
	}
	
	SH100CTRL_SetAmpState(ampState);
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
	SH100HW_SetNewLedState(LED_LOOP, ampState.loopOn);
	SH100HW_SetNewLedState(LED_A, !ampState.swAB);
	SH100HW_SetNewLedState(LED_B, ampState.swAB);
}

void SH100CTRL_SwChannel(uint8_t chNum)
{
	ampState.channelNum = chNum;
	
	SH100HW_SwitchCh(chNum);
	setChannelLeds();
}

void SH100CTRL_SwLoop()
{
	ampState.loopOn = !ampState.loopOn;
	SH100HW_LoopEn(ampState.loopOn);
	SH100HW_SetNewLedState(LED_LOOP, ampState.loopOn);
}

void SH100CTRL_SwAB()
{
	ampState.swAB = !ampState.swAB;
	SH100HW_SwitchAB(ampState.swAB);
	SH100HW_SetNewLedState(LED_A, !ampState.swAB);
	SH100HW_SetNewLedState(LED_B, ampState.swAB);
}

void SH100CTRL_MuteAmp()
{
	
}

void SH100CTRL_SetAmpState(SH100_State_t state)
{
	ampState = state;
	
	SH100HW_SwitchCh(ampState.channelNum);
	SH100HW_LoopEn(ampState.loopOn);
	SH100HW_SwitchAB(ampState.swAB);
	
	setChannelLeds();
	SH100HW_SetNewLedState(LED_LOOP, ampState.loopOn);
	SH100HW_SetNewLedState(LED_A, !ampState.swAB);
	SH100HW_SetNewLedState(LED_B, ampState.swAB);
}

void SH100CTRL_StoreAmpState()
{
	eeprom_write_block(&ampState, (void*)MEMORY_AMP_STATE_OFFSET, sizeof(SH100_State_t));
}