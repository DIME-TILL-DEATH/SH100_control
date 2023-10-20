#ifndef MIDI_CONTROLLER_H_
#define MIDI_CONTROLLER_H_

#include "midi.h"

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
void MIDICTRL_HandleCommand(MIDI_Command_t command);
void MIDICTRL_StoreUserCommands();

#endif /* MIDI_CONTROLLER_H_ */