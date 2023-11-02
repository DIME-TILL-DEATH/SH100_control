#ifndef SH100_CONTROLLER_H_
#define SH100_CONTROLLER_H_

#include <stdint.h>
#include <stdbool.h>

#define SH100_CHANNEL1 0
#define SH100_CHANNEL2 1
#define SH100_CHANNEL3 2
#define SH100_CHANNEL4 3
#define SH100_LOOP_EN 4
#define SH100_AB_SW 5

typedef struct 
{
	uint8_t channelNum;
	
	bool loopOn[4]; // every channel
	bool swAB;
}SH100_State_t;

void SH100CTRL_Init();

void SH100CTRL_BtnSwChannel(uint8_t chNum); // with loop on/off
void SH100CTRL_SwChannel(uint8_t chNum);
void SH100CTRL_SwLoop();
void SH100CTRL_SwAB();
void SH100CTRL_SetLoop(bool en);
void SH100CTRL_SetAB(bool isB);

void SH100CTRL_MuteAmp();
void SH100CTRL_UnmuteAmp();

void SH100CTRL_SetAmpState(SH100_State_t state);
void SH100CTRL_StoreAmpState();
void SH100CTRL_SetAmpLeds();

void SH100CTRL_CheckOutputJacks();

#endif /* SH100_CONTROLLER_H_ */