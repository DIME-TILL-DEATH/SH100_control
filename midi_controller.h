#ifndef MIDI_CONTROLLER_H_
#define MIDI_CONTROLLER_H_

#include "midi.h"

#define MIDI_PROG_BTN_COUNT 6

#define MIDI_PROG_BTN_CH1 0
#define MIDI_PROG_BTN_CH2 1
#define MIDI_PROG_BTN_CH3 2
#define MIDI_PROG_BTN_CH4 3
#define MIDI_PROG_BTN_LOOP 4
#define MIDI_PROG_BTN_AB 5
#define MIDI_PROG_BTN_UNDEFINED 255

typedef enum
{
	RUNNING,
	PROGRAMMING,
}MIDICTRL_Mode_t;

typedef struct
{
	MIDI_Command_t channel1;
	MIDI_Command_t channel2;
	MIDI_Command_t channel3;
	MIDI_Command_t channel4;
	MIDI_Command_t loopOn;
	MIDI_Command_t outAB;
}MIDICTRL_CommandBlock_t;

void MIDICTRL_Init();
void MIDICTRL_HandleCommand(const MIDI_Command_t* command);

void MIDICTRL_EnterProgrammingMode();
MIDICTRL_Mode_t MIDICTRL_MidiMode();

void MIDICTRL_SetProgrammingButton(uint8_t progBtnId);

void MIDICTRL_SetMidiChannel(uint8_t midiChNum);
void MIDICTRL_OmniModeEn(bool isEnabled);
void MIDICTRL_MuteCommEn(bool isEnabled);
void MIDICTRL_StoreUserCommands();
void MIDICTRL_DiscardCommands();

void MIDICTRL_SendSwChComm(uint8_t chNum);
void MIDICTRL_SendLoopEnComm();
void MIDICTRL_SendSwABComm();

#endif /* MIDI_CONTROLLER_H_ */