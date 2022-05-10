#pragma once
#include "Game/items.h"

constexpr auto NUM_BATS = 64;

struct BatData
{
	bool On;
	PHD_3DPOS Pose;
	short RoomNumber;

	short Velocity;
	short Counter;
	short LaraTarget;
	byte XTarget;
	byte ZTarget;

	byte Flags;
};

extern int NextBat;
extern BatData Bats[NUM_BATS];

short GetNextBat();
void InitialiseLittleBats(short itemNumber);
void LittleBatsControl(short itemNumber);
void TriggerLittleBat(ItemInfo* item);
void UpdateBats();
