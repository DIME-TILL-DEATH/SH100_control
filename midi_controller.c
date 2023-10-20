#include <avr/pgmspace.h>

#include "midi_controller.h"
#include "sh100_controller.h"

enum 
{
	RUNNING,
	PROGRAMMING
}mode;

enum 
{
	DEFAULT,
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
	// !!!!!!!!LOAD FROM EEPROM after init!!!!!!!!!!!!!
	userCommands = defaultCommands;
	commandSet = USER;
	
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