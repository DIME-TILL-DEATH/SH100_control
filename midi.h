#ifndef MIDI_H_
#define MIDI_H_

#include <stdint.h>
#include <stdbool.h>

typedef enum
{
	MIDI_COMM_UNDEFINED = 0x0,
	
	// Status messages with 2 bytes
	MIDI_COMM_NOTE_OFF = 0x8,
	MIDI_COMM_NOTE_ON =	0x9,
	MIDI_COMM_POLY_PRESSURE = 0xA,
	MIDI_COMM_CONTROL_CHANGE = 0xB,
	MIDI_COMM_PITCH_BEND = 0xE,
	// second nymble parse!
	MIDI_COMM_SONG_POSITION = 0xF2,

	// Status messages with 1 byte
	MIDI_COMM_PROGRAM_CHANGE = 0xC,
	MIDI_COMM_CHANNEL_PRESSURE = 0xD,
	// second nymble parse!
	MIDI_COMM_TIME_CODE = 0xF1,
	MIDI_COMM_SONG_SELECT = 0xF3,

	// System real-time messages(no data bytes)
	// second nymble parse!
	MIDI_COMM_TIMING_CLOCK = 0xF8,
	MIDI_COMM_START	= 0xFA,
	MIDI_COMM_CONTINUE = 0xFB,
	MIDI_COMM_STO = 0xFC,
	MIDI_COMM_ACTIVE_SENSING = 0xFE,
	MIDI_COMM_SYSTEM_RESET = 0xFF,

	// System exclusive
	// second nymble parse!
	MIDI_COMM_START_SYS_EX = 0xF0,
	MIDI_COMM_STOP_SYS_EX = 0xF7
}MIDI_Status_t;


typedef enum
{
	MIDI_TYPE_UNDEFINED = 0,
	MIDI_TYPE_ONE_BYTE = 1,
	MIDI_TYPE_TWO_BYTE,
	MIDI_TYPE_REAL_TIME,
	MIDI_TYPE_START_SYS_EX,
	MIDI_TYPE_STOP_SYS_EX	
}MIDI_StatusType_t;

typedef struct  
{
	MIDI_Status_t status;
	uint8_t channel_type;
	uint8_t data1;
	uint8_t data2;
}MIDI_Command_t;

bool MIDI_IsStatusWord(uint8_t word);
bool MIDI_IsSysRealTime(uint8_t word);

MIDI_StatusType_t MIDI_GetStatusType(MIDI_Status_t status);

#endif /* MIDI_H_ */