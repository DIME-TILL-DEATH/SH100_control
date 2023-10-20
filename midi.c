#include "midi.h"

bool MIDI_IsStatusWord(uint8_t word)
{
	return (word & 0x80);
}

bool MIDI_IsSysRealTime(uint8_t word)
{
	return word > 0xF7;
}

MIDI_StatusType_t MIDI_GetStatusType(MIDI_Status_t status)
{
	switch(status)
	{
		// Status messages with 2 bytes
		case MIDI_COMM_NOTE_OFF: return MIDI_TYPE_TWO_BYTE;
		case MIDI_COMM_NOTE_ON: return MIDI_TYPE_TWO_BYTE;
		case MIDI_COMM_POLY_PRESSURE: return MIDI_TYPE_TWO_BYTE;
		case MIDI_COMM_CONTROL_CHANGE: return MIDI_TYPE_TWO_BYTE;
		case MIDI_COMM_PITCH_BEND: return MIDI_TYPE_TWO_BYTE;
		case MIDI_COMM_SONG_POSITION: return MIDI_TYPE_TWO_BYTE;

		// Status messages with 1 byte
		case MIDI_COMM_PROGRAM_CHANGE: return MIDI_TYPE_ONE_BYTE;
		case MIDI_COMM_CHANNEL_PRESSURE: return MIDI_TYPE_ONE_BYTE;
		case MIDI_COMM_TIME_CODE: return MIDI_TYPE_ONE_BYTE;
		case MIDI_COMM_SONG_SELECT: return MIDI_TYPE_ONE_BYTE;

		// System real-time messages(no data bytes)
		case MIDI_COMM_TIMING_CLOCK: return MIDI_TYPE_REAL_TIME;
		case MIDI_COMM_START: return MIDI_TYPE_REAL_TIME;
		case MIDI_COMM_CONTINUE: return MIDI_TYPE_REAL_TIME;
		case MIDI_COMM_STO: return MIDI_TYPE_REAL_TIME;
		case MIDI_COMM_ACTIVE_SENSING: return MIDI_TYPE_REAL_TIME;
		case MIDI_COMM_SYSTEM_RESET: return MIDI_TYPE_REAL_TIME;

		// System exclusive
		case MIDI_COMM_START_SYS_EX: return MIDI_TYPE_START_SYS_EX;
		case MIDI_COMM_STOP_SYS_EX: return MIDI_TYPE_STOP_SYS_EX;
		
		default: return MIDI_TYPE_UNDEFINED;
	}
}