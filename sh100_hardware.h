#ifndef SH100_HARDWARE_H_
#define SH100_HARDWARE_H_

#include <stdint.h>
#include <stdbool.h>

#define LED_CH1 0
#define LED_CH2 1
#define LED_CH3 2
#define LED_CH4 3
#define LED_LOOP 4
#define LED_A 5
#define LED_B 6
#define LED_PWR_GRN 7
#define LED_PWR_RED 8

typedef enum
{
	BT_OFF = 1,
	BT_ON = 0
}SH100HW_ButtonState_t;

typedef enum
{
	LED_OFF = 0,
	LED_ON,
	LED_FAST_BLINKING,
	LED_SLOW_BLINKING
}SH100HW_LedState_t;

typedef enum
{
	OUT_NONE=0,
	OUT_16OHM=1,
	OUT_8OHM=2,
	OUT_BOTH=3
}SH100HW_OutputJacks_t;

typedef struct  
{
	uint8_t midiChNum;
	bool midiOmni;
	bool midiMuteComm;
	
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
}SH100HW_Controls_t;

typedef enum
{
	ADC_V_POSITIVE = 0x0,
	ADC_V_SIGNAL = 0x6,
	ADC_V_NEGATIVE = 0x7
}ADC_Channels_t;

void SH100HW_Init();
SH100HW_Controls_t SH100HW_GetControlsState();

void SH100HW_SetCh(uint8_t chNum);
void SH100HW_LoopEn(bool isEnabled);
void SH100HW_SetAB(bool isBEn);

void SH100HW_SetNewLedState(uint8_t ledId, SH100HW_LedState_t newState);
void SH100HW_SetPreviousLedState(uint8_t ledId);
void SH100HW_SetLedBlinkCount(uint8_t ledId, uint8_t blinkCount);

SH100HW_OutputJacks_t SH100HW_GetOutputJacks();

void SH100HW_StartADConvertion(ADC_Channels_t channel);

typedef enum
{
	OUTPUT_MUTE = 0,
	OUTPUT_ENABLED	
}SH100HW_OutputState_t;
void SH100HW_SetPAState(SH100HW_OutputState_t state);

typedef enum
{
	OUTPUT_8OHM = 0,
	OUTPUT_16OHM = 1
}SH100HW_PAMode_t;
void SH100HW_SetOutputMode(SH100HW_PAMode_t mode);
void SH100HW_SetPAFailure(bool isFail);
bool SH100HW_GetPAFailure();

void SH100HW_MainTask();

#endif /* SH100_HARDWARE_H_ */