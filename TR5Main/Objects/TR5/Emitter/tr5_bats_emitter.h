#pragma once
#include "Game/items.h"

constexpr auto NUM_BATS = 64;

struct BAT_STRUCT
{
	PHD_3DPOS pos; // size=20, offset=0
	short roomNumber; // size=0, offset=20
	short speed; // size=0, offset=22
	short counter; // size=0, offset=24
	short laraTarget; // size=0, offset=26
	byte xTarget; // size=0, offset=28
	byte zTarget; // size=0, offset=29
	byte on; // size=0, offset=30
	byte flags; // size=0, offset=31
};

extern int NextBat;
extern BAT_STRUCT Bats[NUM_BATS];


short GetNextBat();
void InitialiseLittleBats(short itemNumber);
void LittleBatsControl(short itemNumber);
void TriggerLittleBat(ITEM_INFO* item);
void UpdateBats();