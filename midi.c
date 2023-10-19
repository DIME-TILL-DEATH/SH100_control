#include "midi.h"

bool MIDI_IsStatusWord(uint8_t word)
{
	return (word & 0x80);
}

bool MIDI_IsSysRealTime(uint8_t word)
{
	return word > 0xF7;
}