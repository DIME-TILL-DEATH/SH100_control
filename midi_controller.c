//#include <avr/pgmspace.h>  // use only if SRAM is over. Can put commandBlocks in FLASH
#include <avr/eeprom.h>
#include <asf.h>

#include "midi_parser.h"
#include "midi_controller.h"

#include "sh100_memory.h"
#include "sh100_controller.h"
#include "sh100_hardware.h"

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

void indicateMidiError();

bool isEqualCommands(MIDI_Command_t commandRecieved, MIDI_Command_t commandSaved)
{
	if(commandRecieved.status != commandSaved.status) return false;
	if(commandRecieved.data1 != commandSaved.data2) return false;	
	return true;
}

void loadCommSetFromMemory()
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
	// Timer1 init. For error indication
	TCCR1B |= 0x05; // psc = 1024
	TIMSK1 |= 0x01; // OVF INT enable, count pulse = 100us
	TCNT1 = 0;
	
	currentErrBtnId = MIDI_PROG_BTN_UNDEFINED;
	
	loadCommSetFromMemory();
	mode = RUNNING;
}

void MIDICTRL_SwitchMode(MIDICTRL_Mode_t newMode)
{	
	if(mode == RUNNING)
	{
		MIDI_SetRetranslateState(false);
		
		for(uint8_t i=0; i<MIDI_PROG_BTN_COUNT;i++) 
			midiProgBtnState[i] = PROG_CLEAR;
		
		MIDICTRL_SetProgrammingButton(MIDI_PROG_BTN_CH1);
		setMidiLeds();
		
		SH100HW_SetNewLedState(LED_PWR_GRN, LED_SLOW_BLINKING);
		SH100HW_SetNewLedState(LED_PWR_RED, LED_SLOW_BLINKING);
	}
	else
	{
		MIDI_SetRetranslateState(true);
	}
	mode = newMode;
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
				if(isEqualCommands(command, muteCommand)) 
				{
					if(command.data2>0) SH100CTRL_MuteAmp();
					else SH100CTRL_UnmuteAmp();
				}
			}
			
			MIDICTRL_CommandBlock_t* currentCommandBlock;
			if(commandSet == USER) currentCommandBlock = &userCommands;
			else currentCommandBlock = &defaultCommands;
			
			// priority ch1, ch2, ch3, ch4, loop, AB. After handling, return. Only one switch by one command
			if(isEqualCommands(command, currentCommandBlock->channel1)) SH100CTRL_SwChannel(0); return;
			if(isEqualCommands(command, currentCommandBlock->channel2)) SH100CTRL_SwChannel(1); return;
			if(isEqualCommands(command, currentCommandBlock->channel3)) SH100CTRL_SwChannel(2); return;
			if(isEqualCommands(command, currentCommandBlock->channel4)) SH100CTRL_SwChannel(3); return;
			
			if(isEqualCommands(command, currentCommandBlock->loopOn)) 
			{
				SH100CTRL_SetLoop((command.data2>0)); 
				return;
			}
			if(isEqualCommands(command, currentCommandBlock->outAB)) 
			{
				SH100CTRL_SwAB((command.data2>0)); 
				return;
			}
			break;
		}
		
		case PROGRAMMING:
		{
			if(command.status == MIDI_COMM_PROGRAM_CHANGE)
			{
				switch(currentProgBtn)
				{
					case MIDI_PROG_BTN_CH1: programmBtn(&(userCommands.channel1), &command, MIDI_PROG_BTN_CH1); break;
					case MIDI_PROG_BTN_CH2: programmBtn(&(userCommands.channel2), &command, MIDI_PROG_BTN_CH2); break;
					case MIDI_PROG_BTN_CH3: programmBtn(&(userCommands.channel3), &command, MIDI_PROG_BTN_CH3); break;
					case MIDI_PROG_BTN_CH4: programmBtn(&(userCommands.channel4), &command, MIDI_PROG_BTN_CH4); break;
					default: indicateMidiError(); break; 
				}
			}
			else if(command.status == MIDI_COMM_CONTROL_CHANGE)
			{
				if(command.data1 == muteCommand.data1)
				{
					indicateMidiError();
				}
				else
				{
					switch(currentProgBtn)
					{
						case MIDI_PROG_BTN_AB: programmBtn(&(userCommands.outAB), &command, MIDI_PROG_BTN_AB); break;
						case MIDI_PROG_BTN_LOOP: programmBtn(&(userCommands.loopOn), &command, MIDI_PROG_BTN_LOOP); break;
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
		
		eeprom_write_word(0x00, MEMORY_MAGIC_WORD);
		eeprom_write_byte((void*)MEMORY_COMMAND_BLOCK_TYPE_OFFSET, commandSet);
		eeprom_write_block(&userCommands, (void*)MEMORY_USER_COMMANDS_OFFSET, sizeof(MIDICTRL_CommandBlock_t));
		
		mode = RUNNING;
	}
}

void MIDICTRL_DiscardCommands()
{
	if(mode == PROGRAMMING)
	{
		commandSet = DEFAULT;
		loadCommSetFromMemory();
		mode = RUNNING;
	}
}

//===================ERROR indication=================
void indicateMidiError()
{
	TCNT1 = 100;
	currentErrBtnId = currentProgBtn;
	setMidiLeds();
	
	SH100HW_SetNewLedState(LED_PWR_GRN, LED_OFF);
	SH100HW_SetNewLedState(LED_PWR_RED, LED_ON);
}

ISR(TIMER1_OVF_vect)
{
	currentErrBtnId = MIDI_PROG_BTN_UNDEFINED;
	setMidiLeds();
	
	SH100HW_SetPreviousLedState(LED_PWR_GRN);
	SH100HW_SetPreviousLedState(LED_PWR_RED);
}