#include <avr/eeprom.h>
#include <asf.h>

#include <string.h>

#include "midi_parser.h"
#include "midi_controller.h"

#include "sh100_memory.h"
#include "sh100_controller.h"
#include "sh100_hardware.h"

#define MIDI_CC_MUTE 7
#define MIDI_CC_CHANNEL1 21
#define MIDI_CC_CHANNEL2 22
#define MIDI_CC_CHANNEL3 23
#define MIDI_CC_CHANNEL4 24
#define MIDI_CC_LOOP 25
#define MIDI_CC_AB 26

#include "footswitch.h"

typedef struct 
{
	uint8_t channelNum : 4;
	bool loopOn : 1;
	bool stateAB : 1;
}MIDICTRL_Preset_t;

MIDICTRL_Preset_t storedPresets[128];
MIDICTRL_Preset_t progPresets[128];

MIDICTRL_Preset_t currentPresetState;

bool isIndicatingError;
bool isIndicatingAccept;

MIDICTRL_Mode_t mode = RUNNING;

bool omniModeEnabled = false;
bool muteCommandEnabled = false;

uint8_t midiChannelNum = 0;

MIDICTRL_CommandBlock_t fixedCommands =
{
	.mute =
	{
		.status = MIDI_COMM_CONTROL_CHANGE,
		.midiChannel = 0,
		.data1 = MIDI_CC_MUTE,
		.data2 = 0
	},
	
	.channel1 =
	{
		.status = MIDI_COMM_CONTROL_CHANGE,
		.midiChannel = 0,
		.data1 = MIDI_CC_CHANNEL1,
		.data2 = 0x7F
	},	
	
	.channel2 =
	{
		.status = MIDI_COMM_CONTROL_CHANGE,
		.midiChannel = 0,
		.data1 = MIDI_CC_CHANNEL2,
		.data2 = 0x7F
	},
	
	.channel3 =
	{
		.status = MIDI_COMM_CONTROL_CHANGE,
		.midiChannel = 0,
		.data1 = MIDI_CC_CHANNEL3,
		.data2 = 0x7F
	},
	
	.channel4 =
	{
		.status = MIDI_COMM_CONTROL_CHANGE,
		.midiChannel = 0,
		.data1 = MIDI_CC_CHANNEL4,
		.data2 = 0x7F
	},
	
	.loopOn =
	{
		.status = MIDI_COMM_CONTROL_CHANGE,
		.midiChannel = 0,
		.data1 = MIDI_CC_LOOP,
		.data2 = 0
	},
	
	.outAB =
	{
		.status = MIDI_COMM_CONTROL_CHANGE,
		.midiChannel = 0,
		.data1 = MIDI_CC_AB,
		.data2 = 0
	}
};
void indicateMidiError();
void indicateMidiAccept();

bool midiValToBool(uint8_t data)
{
	return (data > 63) ? 1 : 0;
}

void fillDefaultPresets()
{
	for(int i=0; i<128; i++)
	{
		storedPresets[i].channelNum = i%4;
		storedPresets[i].loopOn = (i/4)%2;
		storedPresets[i].stateAB = (i/8)%2;
	}
}

void loadCommSetFromMemory()
{
	uint16_t readedMagicWord = eeprom_read_word((uint16_t*)MEMORY_MW_MIDI_COMMANDS_OFFSET);
		
	if(readedMagicWord == MEMORY_MAGIC_WORD)
	{		
		// memory is not empty. Load EEPROM values
		uint8_t readedData[128*sizeof(MIDICTRL_Preset_t)];
		eeprom_read_block(&readedData, (void*)MEMORY_USER_PRESETS_OFFSET, 128*sizeof(MIDICTRL_Preset_t));
		
		memcpy(storedPresets, readedData, 128*sizeof(MIDICTRL_Preset_t));
	}
	else
	{
		// memory empty. Load default values
		fillDefaultPresets();
		
	}
}

void setMidiLeds()
{
	if(mode == PROGRAMMING)
	{
		if (isIndicatingError)
		{
			for(uint8_t i=0; i<LED_COUNT; i++)
			{
				SH100HW_SetNewLedState(i, LED_ON); 
			}
		}
		else
		{
			SH100HW_LedState_t chosenCtrlLedState = isIndicatingAccept ? LED_ON : LED_FAST_BLINKING;
			
			for (int i=0; i<4; i++)
			{
				if(currentPresetState.channelNum == i)
				{
					SH100HW_SetNewLedState(i, chosenCtrlLedState);
				}
				else
				{
					SH100HW_SetNewLedState(i, LED_OFF);
				}
			}
			
			SH100HW_SetNewLedState(LED_LOOP, currentPresetState.loopOn ? chosenCtrlLedState : LED_OFF);
			
			if(currentPresetState.stateAB)
			{
				SH100HW_SetNewLedState(LED_A, LED_OFF);
				SH100HW_SetNewLedState(LED_B, chosenCtrlLedState);
			}
			else
			{
				SH100HW_SetNewLedState(LED_A, chosenCtrlLedState);
				SH100HW_SetNewLedState(LED_B, LED_OFF);
			}
		}
	}
}

void MIDICTRL_Init()
{	
	isIndicatingError = false;
	isIndicatingAccept = false;
	
	loadCommSetFromMemory();
	mode = RUNNING;
}

void MIDICTRL_FactoryReset()
{
	fillDefaultPresets();
	eeprom_write_word((uint16_t*)MEMORY_MW_MIDI_COMMANDS_OFFSET, 0xFFFF);
}

void MIDICTRL_EnterProgrammingMode()
{	
	mode = PROGRAMMING;
	MIDI_SetRetranslateState(false);
	
	memcpy(progPresets, storedPresets, 128*sizeof(MIDICTRL_Preset_t));
	
	currentPresetState.channelNum = 0;
	currentPresetState.loopOn = false;
	currentPresetState.stateAB = false;
	
	SH100HW_SetNewLedState(LED_PWR_GRN, LED_SLOW_BLINKING);
	SH100HW_SetNewLedState(LED_PWR_RED, LED_SLOW_BLINKING);
	
	SH100HW_SetNewLedState(LED_B, LED_OFF);
	
	setMidiLeds();
}

MIDICTRL_Mode_t MIDICTRL_MidiMode()
{
	return mode;
}

void MIDICTRL_SetProgrammingButton(uint8_t progBtnId)
{
	switch(progBtnId)
	{
		case MIDI_PROG_BTN_CH1: currentPresetState.channelNum = 0; break;
		case MIDI_PROG_BTN_CH2: currentPresetState.channelNum = 1; break;
		case MIDI_PROG_BTN_CH3: currentPresetState.channelNum = 2; break;
		case MIDI_PROG_BTN_CH4: currentPresetState.channelNum = 3; break;
		case MIDI_PROG_BTN_LOOP: currentPresetState.loopOn = !currentPresetState.loopOn; break;
		case MIDI_PROG_BTN_AB: currentPresetState.stateAB = !currentPresetState.stateAB; break;
	}
	setMidiLeds();
}

void MIDICTRL_SetMidiChannel(uint8_t midiChNum)
{
	midiChannelNum = midiChNum;
}

void MIDICTRL_OmniModeEn(bool isEnabled)
{
	omniModeEnabled = isEnabled;
}

void MIDICTRL_MuteCommEn(bool isEnabled)
{
	muteCommandEnabled = isEnabled;
}

void MIDICTRL_SendSwChComm(uint8_t chNum)
{
	if(mode == RUNNING)
	{
		switch(chNum)
		{
			case SH100_CHANNEL1: MIDI_SendCommand(fixedCommands.channel1, midiChannelNum); break;
			case SH100_CHANNEL2: MIDI_SendCommand(fixedCommands.channel2, midiChannelNum); break;
			case SH100_CHANNEL3: MIDI_SendCommand(fixedCommands.channel3, midiChannelNum); break;
			case SH100_CHANNEL4: MIDI_SendCommand(fixedCommands.channel4, midiChannelNum); break;
			default: break;
		}
	}
}

void MIDICTRL_SendLoopEnComm(bool isEn)
{
	if(mode == RUNNING)
	{	
		MIDI_Command_t loopComm = fixedCommands.loopOn;
		loopComm.data2 = isEn ? 0x7F : 0x00;
		
		MIDI_SendCommand(loopComm, midiChannelNum);
	}
}

void MIDICTRL_SendSwABComm(bool isEn)
{
	if(mode == RUNNING)
	{	
		MIDI_Command_t abComm = fixedCommands.outAB;
		abComm.data2 = isEn ? 0x7F : 0x00;
		
		MIDI_SendCommand(abComm, midiChannelNum);
	}
}

void MIDICTRL_HandleCommand(const MIDI_Command_t* command)
{
	//if(FSW_BlockFrontControls()) return;
	if(!omniModeEnabled)
	{
		if(midiChannelNum != command->midiChannel) return;
	}
	
	switch(mode)
	{
		case RUNNING:
		{
			if(command->status == MIDI_COMM_CONTROL_CHANGE)
			{
				switch(command->data1)
				{
					/*case MIDI_CC_CHANNEL1: SH100CTRL_SetChannelExclusive(0); break;
					case MIDI_CC_CHANNEL2: SH100CTRL_SetChannelExclusive(1); break;
					case MIDI_CC_CHANNEL3: SH100CTRL_SetChannelExclusive(2); break;
					case MIDI_CC_CHANNEL4: SH100CTRL_SetChannelExclusive(3); break;*/
					case MIDI_CC_CHANNEL1: SH100CTRL_BtnSetChannel(0); break;
					case MIDI_CC_CHANNEL2: SH100CTRL_BtnSetChannel(1); break;
					case MIDI_CC_CHANNEL3: SH100CTRL_BtnSetChannel(2); break;
					case MIDI_CC_CHANNEL4: SH100CTRL_BtnSetChannel(3); break;
					case MIDI_CC_LOOP: SH100CTRL_SetLoop(midiValToBool(command->data2)); break;
					case MIDI_CC_AB: SH100CTRL_SetAB(midiValToBool(command->data2)); break;
					case MIDI_CC_MUTE: 
					{
						if(muteCommandEnabled) SH100CTRL_SetMuteAmp(midiValToBool(command->data2)); 
						break;
					}
				}
			}
			
			if(command->status == MIDI_COMM_PROGRAM_CHANGE)
			{
				MIDICTRL_Preset_t preset = storedPresets[command->data1];
				
				SH100CTRL_SetChannel(preset.channelNum);
				SH100CTRL_SetLoop(preset.loopOn);
				SH100CTRL_SetAB(preset.stateAB);
			}
		}
		
		case PROGRAMMING:
		{
			if(command->status == MIDI_COMM_PROGRAM_CHANGE)
			{
				progPresets[command->data1] = currentPresetState;
				indicateMidiAccept();
			}
			/*else
			{
				indicateMidiError();
			}	*/		
			break;
		}
	}	
}

void MIDICTRL_StoreUserCommands()
{
	if(mode == PROGRAMMING)
	{
		memcpy(storedPresets, progPresets, 128*sizeof(MIDICTRL_Preset_t));
		
		eeprom_write_word((uint16_t*)MEMORY_MW_MIDI_COMMANDS_OFFSET, MEMORY_MAGIC_WORD);
		eeprom_write_block(storedPresets, (void*)MEMORY_USER_PRESETS_OFFSET, 128*sizeof(MIDICTRL_Preset_t));
		
		SH100HW_SetPreviousLedState(LED_B);
		MIDI_SetRetranslateState(true);
		
		mode = RUNNING;
	}
}

void MIDICTRL_DiscardCommands()
{
	if(mode == PROGRAMMING)
	{		
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
	
	isIndicatingError = true;
	setMidiLeds();
	
	SH100HW_SetNewLedState(LED_PWR_GRN, LED_OFF);
	SH100HW_SetNewLedState(LED_PWR_RED, LED_ON);
}

void indicateMidiAccept()
{
	TCNT1 = 0xFFFF - MIDI_ERR_TIMER_PERIOD;
	TIMSK1 |= 0x01; // OVF INT enable, count pulse = 100us
	TCCR1B |= 0x05; // psc = 1024, timer on
	
	isIndicatingError = false;
	isIndicatingAccept = true;
	setMidiLeds();
	
	SH100HW_SetNewLedState(LED_PWR_GRN, LED_ON);
	SH100HW_SetNewLedState(LED_PWR_RED, LED_OFF);
}

ISR(TIMER1_OVF_vect)
{
	TIMSK1 |= 0x00; // OVF INT disable
	TCCR1B |= 0x00; // psc = 0, timer off
	
	isIndicatingAccept = false;
	isIndicatingError = false;
	setMidiLeds();
	
	SH100HW_SetPreviousLedState(LED_PWR_GRN);
	SH100HW_SetPreviousLedState(LED_PWR_RED);
}