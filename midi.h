#ifndef MIDI_H
#define MIDI_h

#include <stdint.h>
#include <stdbool.h>

// Status messages with 2 bytes
#define MIDI_COMM_NOTE_OFF	0x80
#define MIDI_COMM_NOTE_ON	0x90
#define MIDI_COMM_POLY_PRESSURE	0xA0
#define MIDI_COMM_CONTROL_CHANGE	0xB0
#define MIDI_COMM_PITCH_BEND	0xE0
#define MIDI_COMM_SONG_POSITION	0xF2

// Status messages with 1 byte
#define MIDI_COMM_PROGRAM_CHANGE	0xC0
#define MIDI_COMM_CHANNEL_PRESSURE	0xD0
#define MIDI_COMM_TIME_CODE			0xF1
#define MIDI_COMM_SONG_SELECT		0xF3

// System real-time messages(no data bytes)
#define MIDI_COMM_TIMING_CLOCK	0xF8
#define MIDI_COMM_START		0xFA
#define MIDI_COMM_CONTINUE	0xFB
#define MIDI_COMM_STOP		0xFC
#define MIDI_COMM_ACTIVE_SENSING	0xFE
#define MIDI_COMM_SYSTEM_RESET		0xFF

// System exclusive
#define MIDI_COMM_START_SYS_EX	0xF0
#define MIDI_COMM_STOP_SYS_EX	0xF7


typedef enum
{
	MIDI_ONE_BYTE = 1,
	MIDI_TWO,
	MIDI_REAL_TIME	
}MIDI_CommandType;

bool MIDI_IsStatusWord(uint8_t word);
bool MIDI_IsSysRealTime(uint8_t word);

#endif /* MIDI_H */