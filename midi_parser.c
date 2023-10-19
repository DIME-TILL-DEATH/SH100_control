#include "midi_parser.h"

MIDI_Status_t currentStatus = MIDI_COMM_UNDEFINED;

enum
{
	RESET=0,
	IDLE,
	DISPATCH_STATUS,
	REC_FIRST_BYTE,
	REC_SECOND_BYTE,
	HANDLE_MESSAGE,
	HADLE_SYS_EX
}state = RESET;

void MIDI_ParserTask()
{
	switch(state)
	{
		case RESET:
		{
			state = IDLE;
			break;
		}
		
		case IDLE:
		{
			break;
		}
	}
}