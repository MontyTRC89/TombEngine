#pragma once
#include "Game/items.h"
#include "Specific/EulerAngle.h"

constexpr auto NUM_BATS = 64;

struct BatData
{
	bool On;
	PoseData Pose;
	EulerAngle Orientation;
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
void TriggerLittleBat(ITEM_INFO* item);
void UpdateBats();
