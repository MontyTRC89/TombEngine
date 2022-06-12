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
void UpdateHealthBar(ItemInfo* item, int flash);
void DrawAirBar(float value);
void UpdateAirBar(ItemInfo* item, int flash);
void DrawSprintBar(float value);
void UpdateSprintBar(ItemInfo* item);
void AddDisplayPickup(GAME_OBJECT_ID objectNumber);
void DrawAllPickups();
void InitialisePickupDisplay();
int FlashIt();

extern short PickupX;
extern short PickupY;
extern short CurrentPickup;
extern DisplayPickup Pickups[MAX_COLLECTED_PICKUPS];
extern short PickupVel;
extern int OldHitPoints;
extern int HealthBarTimer;
extern float HealthBar;
extern float MutateAmount;
extern int FlashState;
extern int PoisonFlag;

extern bool EnableSmoothHealthBar;
