#pragma once
#include "Game/items.h"

#define MAX_COLLECTED_PICKUPS 32

enum GAME_OBJECT_ID : short;

struct DisplayPickup
{
	int Life;
	short ObjectNumber;
};

void DrawHUD(ItemInfo* item);
void DrawHealthBarOverlay(ItemInfo* item, int value);
void DrawHealthBar(ItemInfo* item, float value);
void DrawHealthBar(ItemInfo* item, bool flash);
void DrawAirBar(float value);
void DrawAirBar(ItemInfo* item, bool flash);
void DrawSprintBar(float value);
void DrawSprintBar(ItemInfo* item);
void UpdateBars(ItemInfo* item);
void AddDisplayPickup(GAME_OBJECT_ID objectNumber);
void DrawAllPickups();
void InitialisePickupDisplay();

extern short PickupX;
extern short PickupY;
extern DisplayPickup Pickups[MAX_COLLECTED_PICKUPS];

extern bool EnableSmoothHealthBar;