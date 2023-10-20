#include "midi_parser.h"

#include "uart.h"

MIDI_Status_t currentStatus = MIDI_COMM_UNDEFINED;
MIDI_StatusType_t currentStatusType = MIDI_TYPE_UNDEFINED;

uint8_t dataWord1, dataWord2;
bool isSecondDataWord = false;

enum
{
	RESET=0,
	WAIT_WORD,
	DISPATCH_STATUS,
	REC_ONE_BYTE,
	REC_TWO_BYTE,
	REC_SYS_EX,
	HADLE_SYS_EX
}state = RESET;

void handleRealTimeStatus(MIDI_Status_t status)
{
	
};

uint8_t recievedWord;
void MIDI_ParserTask()
{
	switch(state)
	{
		case RESET:
		{
			state = WAIT_WORD;
			break;
		}
		
		case WAIT_WORD:
		{
			if(isRxBufferNotEmpty())
			{
				recievedWord = usart_pop_word();
				if(MIDI_IsSysRealTime(recievedWord))
				{ 
					handleRealTimeStatus((MIDI_Status_t)recievedWord);
					return;
				}
				
				if(MIDI_IsStatusWord(recievedWord))
				{
					currentStatus = (MIDI_Status_t)recievedWord;	
					state = DISPATCH_STATUS;
				}
				else
				{
					switch(currentStatusType)
					{
						case MIDI_TYPE_TWO_BYTE: state = REC_TWO_BYTE; break;
						case MIDI_TYPE_ONE_BYTE: state = REC_ONE_BYTE; break;
						case MIDI_TYPE_START_SYS_EX: state = REC_SYS_EX; break;
						case MIDI_TYPE_STOP_SYS_EX: state = HADLE_SYS_EX; break;
						default: state = WAIT_WORD;
					}
				}
			}
			break;
		}
		
		case DISPATCH_STATUS:
		{
			currentStatusType = MIDI_GetStatusType(currentStatus);
			isSecondDataWord = false;
			state = WAIT_WORD;
			break;
		}				
		
		case REC_TWO_BYTE:
		{
			if(isSecondDataWord)
			{
				dataWord2 = recievedWord;
				isSecondDataWord = false;
				// handle message
			}
			else
			{
				dataWord1 = recievedWord;
				isSecondDataWord = true;
			}
			state = WAIT_WORD;
			break;
		}
		
		case REC_ONE_BYTE:
		{
			dataWord1 = recievedWord;
			// handle message
			state = WAIT_WORD;
			break;
		}
		
		case REC_SYS_EX:
		{
			state = WAIT_WORD;
			break;
		}
		
		case HADLE_SYS_EX:
		{
			currentStatus = MIDI_COMM_UNDEFINED;
			state = WAIT_WORD;
			break;	
		}
	}
}