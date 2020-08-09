#pragma once
#include <items.h>

#define NUM_LITTLE_BETTLES 256

struct BEETLE_INFO
{
	PHD_3DPOS pos;
	short roomNumber;
	short speed;
	short fallspeed;
	byte on;
	byte flags;
};

extern BEETLE_INFO* LittleBeetles;
extern int NextLittleBeetle;

void InitialiseLittleBeetle(short itemNumber);
void LittleBeetleControl(short itemNumber);
short GetNextLittleBeetle();
void ClearLittleBeetles();
void UpdateLittleBeetles();
