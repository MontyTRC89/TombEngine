#pragma once
#include "Game/items.h"

constexpr auto NUM_SPIDERS = 64;

struct SpiderData
{
	byte On;
	PHD_3DPOS Pose;
	short RoomNumber;

	short Velocity;
	short VerticalVelocity;

	byte Flags;
};

extern int NextSpider;
extern SpiderData Spiders[NUM_SPIDERS];

short GetNextSpider();
void ClearSpiders();
void InitialiseSpiders(short itemNumber);
void SpidersEmitterControl(short itemNumber);
void UpdateSpiders();