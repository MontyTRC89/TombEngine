#pragma once
#include "Game/items.h"
#include "Specific/EulerAngle.h"

constexpr auto NUM_RATS = 32;

struct RatData
{
	byte On;
	PHD_3DPOS Pose;
	EulerAngle Orientation;
	short RoomNumber;

	short Velocity;
	short VerticalVelocity;

	byte Flags;
};

extern int NextRat;
extern RatData Rats[NUM_RATS];

void ClearRats();
short GetNextRat();
void InitialiseLittleRats(short itemNumber);
void LittleRatsControl(short itemNumber);
void UpdateRats();
