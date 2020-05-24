#pragma once

#include "global.h"

#define MAX_COLLECTED_PICKUPS 32

void DrawHealthBarOverlay(int value);
void DrawHealthBar(float value);
void UpdateHealtBar(int flash);
void DrawAirBar(float value);
void UpdateAirBar(int flash);
void DrawDashBar(int value);
void AddDisplayPickup(short objectNumber);
int DrawAllPickups();
void InitialisePickupDisplay();
int FlashIt();

extern short PickupX;
extern short PickupY;
extern short CurrentPickup;
extern DISPLAY_PICKUP Pickups[MAX_COLLECTED_PICKUPS];
extern short PickupVel;
extern int OldHitPoints;
extern int HealtBarTimer;
extern int FlashState;
extern int PoisonFlag;
extern int DashTimer;
