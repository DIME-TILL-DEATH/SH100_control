#ifndef SH100_HARDWARE_H_
#define SH100_HARDWARE_H_

#include <stdint.h>
#include <stdbool.h>

#define LED_CH1 0
#define LED_CH2 1
#define LED_CH3 2
#define LED_CH4 3
#define LED_LOOP 5
#define LED_A 6
#define LED_B 7
#define LED_PWR_GRN 7
#define LED_PWR_RED 8

typedef enum
{
	BT_OFF = 0,
	BT_ON
}SH100HW_ButtonState_t;

typedef enum
{
	LED_OFF = 0,
	LED_ON,
	LED_FAST_BLINKING,
	LED_SLOW_BLINKING,
}SH100HW_LedState_t;

typedef struct  
{
	uint8_t midiChNum;
	SH100HW_ButtonState_t midiOmni;
	SH100HW_ButtonState_t midiMuteComm;
	
	SH100HW_ButtonState_t btnCh1;
	SH100HW_ButtonState_t btnCh2;
	SH100HW_ButtonState_t btnCh3;
	SH100HW_ButtonState_t btnCh4;
	SH100HW_ButtonState_t btnAB;
	SH100HW_ButtonState_t btnLoop;
	
	SH100HW_ButtonState_t FS1_sleeve;
	SH100HW_ButtonState_t FS1_tip;
	SH100HW_ButtonState_t FS1_presence;
	
	SH100HW_ButtonState_t FS2_sleeve;
	SH100HW_ButtonState_t FS2_tip;
	SH100HW_ButtonState_t FS2_presence;
}SH100HW_Buttons_t;

void SH100HW_Init();
SH100HW_Buttons_t SH100HW_GetButtonsState();

void SH100HW_SwitchCh(uint8_t chNum);
void SH100HW_LoopEn(bool isEnabled);
void SH100HW_SwitchAB(bool isBEn);

void SH100HW_SetNewLedState(uint8_t ledId, SH100HW_LedState_t newState);
void SH100HW_SetPreviousLedState(uint8_t ledId);

void SH100HW_MainTask();

#endif /* SH100_HARDWARE_H_ */