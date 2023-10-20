//#include <avr/pgmspace.h>  // use only if SRAM is over. Can put commandBlocks in FLASH
#include <avr/eeprom.h>

#include "sh100_memory.h"

#include "midi_controller.h"
#include "sh100_controller.h"

enum 
{
	RUNNING,
	PROGRAMMING
}mode;

enum 
{
	DEFAULT = 0,
	USER
}commandSet;

bool omniModeEnabled = false;
bool muteCommandEnabled = false;

uint8_t channelNum = 0;

const MIDI_Command_t muteCommand =
{
	.status = MIDI_COMM_CONTROL_CHANGE,
	.channel = 0,
	.data1 = 7,
	.data2 = 0
};

MIDICTRL_CommandBlock_t userCommands;
MIDICTRL_CommandBlock_t defaultCommands =
{
	.channel1 =
	{
		.status = MIDI_COMM_PROGRAM_CHANGE,
		.channel = 0,
		.data1 = 0,
		.data2 = 0
	},	
	
	.channel2 =
	{
		.status = MIDI_COMM_PROGRAM_CHANGE,
		.channel = 0,
		.data1 = 1,
		.data2 = 0
	},
	
	.channel2 =
	{
		.status = MIDI_COMM_PROGRAM_CHANGE,
		.channel = 0,
		.data1 = 2,
		.data2 = 0
	},
	
	.channel3 =
	{
		.status = MIDI_COMM_PROGRAM_CHANGE,
		.channel = 0,
		.data1 = 3,
		.data2 = 0
	},
	
	.loopOn =
	{
		.status = MIDI_COMM_CONTROL_CHANGE,
		.channel = 0,
		.data1 = 15,
		.data2 = 0
	},
	
	.outAB =
	{
		.status = MIDI_COMM_CONTROL_CHANGE,
		.channel = 0,
		.data1 = 14,
		.data2 = 0
	}
};

bool isEqualCommands(MIDI_Command_t commandRecieved, MIDI_Command_t commandSaved)
{
	if(commandRecieved.status != commandSaved.status) return false;
	if(commandRecieved.data1 != commandSaved.data2) return false;	
	return true;
}

void MIDICTRL_Init()
{
	uint16_t readedMagicWord = eeprom_read_word(0x00);
	
	if(readedMagicWord == MEMORY_MAGIC_WORD)
	{
		// memory is not empty. Load EEPROM values
		uint8_t readedData[sizeof(MIDICTRL_CommandBlock_t)];	
		eeprom_read_block(&readedData, (uint16_t*)MEMORY_USER_COMMANDS_OFFSET, sizeof(MIDICTRL_CommandBlock_t));		
		MIDICTRL_CommandBlock_t* userCommands_ptr = (MIDICTRL_CommandBlock_t*)readedData;
		userCommands = *userCommands_ptr;
		
		commandSet = eeprom_read_byte((uint8_t*)MEMORY_COMMAND_BLOCK_TYPE_OFFSET);
	}
	else
	{
		// memory empty. Load default values
		userCommands = defaultCommands;
		commandSet = DEFAULT;	
	}

	mode = RUNNING;
}

void MIDICTRL_HandleCommand(MIDI_Command_t command)
{
	switch(mode)
	{
		case RUNNING:
		{
			if(!omniModeEnabled)
			{
				if(channelNum != command.channel) return;
			}
			
			if(muteCommandEnabled)
			{
				if(isEqualCommands(command, muteCommand)) SH100CTRL_MuteAmp();
			}
			
			MIDICTRL_CommandBlock_t* currentCommandBlock;
			if(commandSet == USER) currentCommandBlock = &userCommands;
			else currentCommandBlock = &defaultCommands;
			
			// priority ch1, ch2, ch3, ch4, loop, AB. After handling, return. Only one switch by one command
			if(isEqualCommands(command, currentCommandBlock->channel1)) SH100CTRL_SwChannel1(); return;
			if(isEqualCommands(command, currentCommandBlock->channel2)) SH100CTRL_SwChannel2(); return;
			if(isEqualCommands(command, currentCommandBlock->channel3)) SH100CTRL_SwChannel3(); return;
			if(isEqualCommands(command, currentCommandBlock->channel4)) SH100CTRL_SwChannel4(); return;
			
			if(isEqualCommands(command, currentCommandBlock->loopOn)) SH100CTRL_SwLoop(); return;
			if(isEqualCommands(command, currentCommandBlock->outAB)) SH100CTRL_SwAB(); return;
			break;
		}
		
		case PROGRAMMING:
		{
			break;
		}
	}	
}

void MIDICTRL_StoreUserCommands()
{
	if(mode == PROGRAMMING)
	{
		eeprom_write_word(0x00, MEMORY_MAGIC_WORD);
		eeprom_write_byte((void*)MEMORY_COMMAND_BLOCK_TYPE_OFFSET, commandSet);
		eeprom_write_block(&userCommands, (void*)MEMORY_USER_COMMANDS_OFFSET, sizeof(MIDICTRL_CommandBlock_t));
	}
}