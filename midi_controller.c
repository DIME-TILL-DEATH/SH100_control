#include <avr/eeprom.h>
#include <asf.h>

#include "midi_parser.h"
#include "midi_controller.h"

#include "sh100_memory.h"
#include "sh100_controller.h"
#include "sh100_hardware.h"

#include "footswitch.h"

enum 
{
	DEFAULT = 0,
	USER
}commandSet;

typedef enum
{
	PROG_CLEAR,
	PROG_ACTING,
	PROG_PROGRAMMED
}MIDICTRL_ProgBtnState_t;

MIDICTRL_ProgBtnState_t midiProgBtnState[MIDI_PROG_BTN_COUNT];

uint8_t currentProgBtn;
uint8_t currentErrBtnId;

MIDICTRL_Mode_t mode;

bool omniModeEnabled = false;
bool muteCommandEnabled = false;

uint8_t channelNum = 0;

const MIDI_Command_t muteCommand =
{
	.status = MIDI_COMM_CONTROL_CHANGE,
	.channel_type = 0,
	.data1 = 7,
	.data2 = 0
};

MIDICTRL_CommandBlock_t userCommands;
MIDICTRL_CommandBlock_t defaultCommands =
{
	.channel1 =
	{
		.status = MIDI_COMM_PROGRAM_CHANGE,
		.channel_type = 0,
		.data1 = 0,
		.data2 = 0
	},	
	
	.channel2 =
	{
		.status = MIDI_COMM_PROGRAM_CHANGE,
		.channel_type = 0,
		.data1 = 1,
		.data2 = 0
	},
	
	.channel3 =
	{
		.status = MIDI_COMM_PROGRAM_CHANGE,
		.channel_type = 0,
		.data1 = 2,
		.data2 = 0
	},
	
	.channel4 =
	{
		.status = MIDI_COMM_PROGRAM_CHANGE,
		.channel_type = 0,
		.data1 = 3,
		.data2 = 0
	},
	
	.loopOn =
	{
		.status = MIDI_COMM_CONTROL_CHANGE,
		.channel_type = 0,
		.data1 = 15,
		.data2 = 0
	},
	
	.outAB =
	{
		.status = MIDI_COMM_CONTROL_CHANGE,
		.channel_type = 0,
		.data1 = 14,
		.data2 = 0
	}
};

void indicateMidiError();

bool isEqualCommands(const MIDI_Command_t* commandRecieved, const MIDI_Command_t* commandSaved)
{
	if(commandRecieved->status != commandSaved->status) return false;
	if(commandRecieved->data1 != commandSaved->data1) return false;
			
	return true;
}

void loadCommSetFromMemory()
{
	uint16_t readedMagicWord = eeprom_read_word((uint16_t*)0x02);
		
	if(readedMagicWord == MEMORY_MAGIC_WORD)
	{
		// memory is not empty. Load EEPROM values
		uint8_t readedData[sizeof(MIDICTRL_CommandBlock_t)];
		eeprom_read_block(&readedData, (void*)MEMORY_USER_COMMANDS_OFFSET, sizeof(MIDICTRL_CommandBlock_t));
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
}

void setMidiLeds()
{
	if(mode == PROGRAMMING)
	{
		for(uint8_t i=0; i<MIDI_PROG_BTN_COUNT; i++)
		{
			switch(midiProgBtnState[i])
			{
				case PROG_CLEAR: SH100HW_SetNewLedState(i, LED_OFF); break;
				case PROG_ACTING: SH100HW_SetNewLedState(i, LED_FAST_BLINKING); break;
				case PROG_PROGRAMMED: SH100HW_SetNewLedState(i, LED_ON); break;
			}
		}
		
		if(currentErrBtnId != MIDI_PROG_BTN_UNDEFINED)
		{
			SH100HW_SetNewLedState(currentErrBtnId, LED_ON);
		}
	}
}

void MIDICTRL_Init()
{	
	currentErrBtnId = MIDI_PROG_BTN_UNDEFINED;
	
	loadCommSetFromMemory();
	mode = RUNNING;
}

void MIDICTRL_EnterProgrammingMode()
{	
	mode = PROGRAMMING;
	MIDI_SetRetranslateState(false);
	
	for(uint8_t i=0; i<MIDI_PROG_BTN_COUNT;i++)
		midiProgBtnState[i] = PROG_CLEAR;
		
	SH100HW_SetNewLedState(LED_PWR_GRN, LED_SLOW_BLINKING);
	SH100HW_SetNewLedState(LED_PWR_RED, LED_SLOW_BLINKING);
	
	SH100HW_SetNewLedState(LED_B, LED_OFF);
	
	MIDICTRL_SetProgrammingButton(MIDI_PROG_BTN_CH1);	
	setMidiLeds();
}

MIDICTRL_Mode_t MIDICTRL_MidiMode()
{
	return mode;
}

void MIDICTRL_SetProgrammingButton(uint8_t progBtnId)
{
	if(midiProgBtnState[currentProgBtn] == PROG_ACTING) 
	{
		midiProgBtnState[currentProgBtn] = PROG_CLEAR;
	}
	
	currentProgBtn = progBtnId;
	midiProgBtnState[currentProgBtn] = PROG_ACTING;
	setMidiLeds();
}

void MIDICTRL_SetMidiChannel(uint8_t midiChNum)
{
	channelNum = midiChNum;
}

void MIDICTRL_OmniModeEn(bool isEnabled)
{
	omniModeEnabled = isEnabled;
}

void MIDICTRL_MuteCommEn(bool isEnabled)
{
	muteCommandEnabled = isEnabled;
}

void programmBtn(MIDI_Command_t* targetComm, const MIDI_Command_t* srcComm, uint8_t targetBtnId)
{
	if(midiProgBtnState[targetBtnId] == PROG_ACTING)
	{
		targetComm->status = srcComm->status;
		targetComm->data1 = srcComm->data1;
		midiProgBtnState[targetBtnId] = PROG_PROGRAMMED;
		setMidiLeds();
	}
}

void MIDICTRL_SendSwChComm(uint8_t chNum)
{
	if(mode == RUNNING)
	{
		MIDICTRL_CommandBlock_t* currentCommandBlock;
		if(commandSet == USER) currentCommandBlock = &userCommands;
		else currentCommandBlock = &defaultCommands;
		
		switch(chNum)
		{
			case SH100_CHANNEL1: MIDI_SendCommand(currentCommandBlock->channel1, channelNum); break;
			case SH100_CHANNEL2: MIDI_SendCommand(currentCommandBlock->channel2, channelNum); break;
			case SH100_CHANNEL3: MIDI_SendCommand(currentCommandBlock->channel3, channelNum); break;
			case SH100_CHANNEL4: MIDI_SendCommand(currentCommandBlock->channel4, channelNum); break;
			default: break;
		}
	}
}

void MIDICTRL_SendLoopEnComm()
{
	if(mode == RUNNING)
	{
		MIDICTRL_CommandBlock_t* currentCommandBlock;
		if(commandSet == USER) currentCommandBlock = &userCommands;
		else currentCommandBlock = &defaultCommands;
		
		MIDI_SendCommand(currentCommandBlock->loopOn, channelNum);
	}
}

void MIDICTRL_SendSwABComm()
{
	if(mode == RUNNING)
	{
		MIDICTRL_CommandBlock_t* currentCommandBlock;
		if(commandSet == USER) currentCommandBlock = &userCommands;
		else currentCommandBlock = &defaultCommands;
		
		MIDI_SendCommand(currentCommandBlock->outAB, channelNum);
	}
}

void MIDICTRL_HandleCommand(const MIDI_Command_t* command)
{
	if(FSW_BlockFrontControls()) return;
	
	switch(mode)
	{
		case RUNNING:
		{
			if(!omniModeEnabled)
			{
				if(channelNum != command->channel_type) return;
			}
			
			if(muteCommandEnabled)
			{
				if(isEqualCommands(command, &muteCommand)) 
				{
					if((command->data2>63) ? 1 : 0) SH100CTRL_MuteAmp();
					else SH100CTRL_UnmuteAmp();
				}
			}
			
			MIDICTRL_CommandBlock_t* currentCommandBlock;
			if(commandSet == USER) currentCommandBlock = &userCommands;
			else currentCommandBlock = &defaultCommands;
			
			// priority ch1, ch2, ch3, ch4, loop, AB. After handling, return. Only one switch by one command
			if(isEqualCommands(command, &(currentCommandBlock->channel1))) 
			{
				SH100CTRL_SetChannel(0); 
				return;
			}
			if(isEqualCommands(command, &(currentCommandBlock->channel2))) 
			{
				SH100CTRL_SetChannel(1); 
				return;
			}
			if(isEqualCommands(command, &(currentCommandBlock->channel3))) 
			{
				SH100CTRL_SetChannel(2); 
				return;
			}
			if(isEqualCommands(command, &(currentCommandBlock->channel4))) 
			{
				SH100CTRL_SetChannel(3); 
				return;
			}		
			if(isEqualCommands(command, &(currentCommandBlock->loopOn))) 
			{
				SH100CTRL_SetLoop((command->data2>63) ? 1 : 0); 
				return;
			}
			if(isEqualCommands(command, &(currentCommandBlock->outAB))) 
			{
				SH100CTRL_SetAB((command->data2>63) ? 1 : 0); 
				return;
			}
			break;
		}
		
		case PROGRAMMING:
		{
			if(command->status == MIDI_COMM_PROGRAM_CHANGE)
			{
				switch(currentProgBtn)
				{
					case MIDI_PROG_BTN_CH1: programmBtn(&(userCommands.channel1), command, MIDI_PROG_BTN_CH1); break;
					case MIDI_PROG_BTN_CH2: programmBtn(&(userCommands.channel2), command, MIDI_PROG_BTN_CH2); break;
					case MIDI_PROG_BTN_CH3: programmBtn(&(userCommands.channel3), command, MIDI_PROG_BTN_CH3); break;
					case MIDI_PROG_BTN_CH4: programmBtn(&(userCommands.channel4), command, MIDI_PROG_BTN_CH4); break;
					default: indicateMidiError(); break; 
				}
			}
			else if(command->status == MIDI_COMM_CONTROL_CHANGE)
			{
				if(command->data1 == muteCommand.data1)
				{
					indicateMidiError();
				}
				else
				{
					switch(currentProgBtn)
					{
						case MIDI_PROG_BTN_AB: programmBtn(&(userCommands.outAB), command, MIDI_PROG_BTN_AB); break;
						case MIDI_PROG_BTN_LOOP: programmBtn(&(userCommands.loopOn), command, MIDI_PROG_BTN_LOOP); break;
						default: indicateMidiError(); break;
					}
				}
			}
			else
			{
				indicateMidiError();
			}			
			break;
		}
	}	
}

void MIDICTRL_StoreUserCommands()
{
	if(mode == PROGRAMMING)
	{
		commandSet = USER;
		
		eeprom_write_word((uint16_t*)0x02, MEMORY_MAGIC_WORD);
		eeprom_write_byte((void*)MEMORY_COMMAND_BLOCK_TYPE_OFFSET, commandSet);
		eeprom_write_block(&userCommands, (void*)MEMORY_USER_COMMANDS_OFFSET, sizeof(MIDICTRL_CommandBlock_t));
		
		SH100HW_SetPreviousLedState(LED_B);
		MIDI_SetRetranslateState(true);
		
		mode = RUNNING;
	}
}

void MIDICTRL_DiscardCommands()
{
	if(mode == PROGRAMMING)
	{
		commandSet = DEFAULT;
		eeprom_write_byte((void*)MEMORY_COMMAND_BLOCK_TYPE_OFFSET, commandSet);		
		
		SH100HW_SetPreviousLedState(LED_B);
		MIDI_SetRetranslateState(true);
		
		mode = RUNNING;
	}
}

//===================ERROR indication=================
#define MIDI_ERR_TIMER_PERIOD 9765
void indicateMidiError()
{
	TCNT1 = 0xFFFF - MIDI_ERR_TIMER_PERIOD;
	TIMSK1 |= 0x01; // OVF INT enable, count pulse = 100us
	TCCR1B |= 0x05; // psc = 1024, timer on
	currentErrBtnId = currentProgBtn;
	setMidiLeds();
	
	SH100HW_SetNewLedState(LED_PWR_GRN, LED_OFF);
	SH100HW_SetNewLedState(LED_PWR_RED, LED_ON);
}

ISR(TIMER1_OVF_vect)
{
	TIMSK1 |= 0x00; // OVF INT disable
	TCCR1B |= 0x00; // psc = 0, timer off
	currentErrBtnId = MIDI_PROG_BTN_UNDEFINED;
	setMidiLeds();
	
	SH100HW_SetPreviousLedState(LED_PWR_GRN);
	SH100HW_SetPreviousLedState(LED_PWR_RED);
}