#include <avr/eeprom.h>

#include "sh100_memory.h"
#include "sh100_controller.h"

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

void SH100CTRL_SwChannel1()
{
	
}

void SH100CTRL_SwChannel2()
{
	
}

void SH100CTRL_SwChannel3()
{
	
}

void SH100CTRL_SwChannel4()
{
	
}

void SH100CTRL_SwLoop()
{
	
}

void SH100CTRL_SwAB()
{
	
}

void SH100CTRL_MuteAmp()
{
	
}

void SH100CTRL_SetAmpState(SH100_State_t state)
{
	
}

void SH100CTRL_StoreAmpState()
{
	eeprom_write_block(&ampState, (void*)MEMORY_AMP_STATE_OFFSET, sizeof(SH100_State_t));
}