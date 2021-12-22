#pragma once
#include "Game/items.h"

#define MAX_COLLECTED_PICKUPS 32

enum GAME_OBJECT_ID : short;

struct DISPLAY_PICKUP
{
	short life;
	short objectNumber;
};

void DrawHealthBarOverlay(int value);
void DrawHealthBar(float value);
void UpdateHealthBar(ITEM_INFO* item, int flash);
void DrawAirBar(float value);
void UpdateAirBar(ITEM_INFO* item, int flash);
void DrawSprintBar(float value);
void UpdateSprintBar();
void AddDisplayPickup(GAME_OBJECT_ID objectNumber);
void DrawAllPickups();
void InitialisePickupDisplay();
int FlashIt();

extern short PickupX;
extern short PickupY;
extern short CurrentPickup;
extern DISPLAY_PICKUP Pickups[MAX_COLLECTED_PICKUPS];
extern short PickupVel;
extern int OldHitPoints;
extern int HealthBarTimer;
extern float HealthBar;
extern float MutateAmount;
extern int FlashState;
extern int PoisonFlag;

extern bool EnableSmoothHealthBar;
