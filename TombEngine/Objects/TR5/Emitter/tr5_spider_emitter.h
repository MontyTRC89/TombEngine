#pragma once
#include "Game/items.h"

constexpr auto NUM_SPIDERS = 64;

struct SpiderData
{
	byte On;
	Pose Pose;
	short RoomNumber;

	short Velocity;
	short VerticalVelocity;

	byte Flags;
};

extern int NextSpider;
extern SpiderData Spiders[NUM_SPIDERS];

short GetNextSpider();
void ClearSpiders();
void InitializeSpiders(short itemNumber);
void SpidersEmitterControl(short itemNumber);
void UpdateSpiders();