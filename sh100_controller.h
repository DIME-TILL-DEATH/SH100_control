#ifndef SH100_CONTROLLER_H_
#define SH100_CONTROLLER_H_

#include <stdint.h>
#include <stdbool.h>

typedef struct 
{
	uint8_t channelNum;
	
	bool loopOn;
	bool swAB;
}SH100_State_t;

void SH100CTRL_Init();

void SH100CTRL_SwChannel1();
void SH100CTRL_SwChannel2();
void SH100CTRL_SwChannel3();
void SH100CTRL_SwChannel4();

void SH100CTRL_SwLoop();
void SH100CTRL_SwAB();

void SH100CTRL_MuteAmp();

void SH100CTRL_SetAmpState(SH100_State_t state);
void SH100CTRL_StoreAmpState();

#endif /* SH100_CONTROLLER_H_ */