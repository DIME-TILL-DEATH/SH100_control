#ifndef MIDI_PARSER_H_
#define MIDI_PARSER_H_

#include "midi.h"

void MIDI_ParserTask();
void MIDI_SendCommand(MIDI_Command_t command);

void MIDI_SetRetranslateState(bool enabled);

#endif /* MIDI_PARSER_H_ */