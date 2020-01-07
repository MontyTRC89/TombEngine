#pragma once

#include "..\Global\global.h"

//#define FlashIt ((int (__cdecl*)()) 0x00439C10)
//#define UpdateHealtBar ((void (__cdecl*)(int)) 0x00439E50)
//#define UpdateAirBar ((void (__cdecl*)(int)) 0x00439FC0)

void DrawHealthBarOverlay(int value);
void DrawHealthBar(int value);
void UpdateHealtBar(int flash);
void DrawAirBar(int value);
void UpdateAirBar(int flash);
void DrawDashBar(int value);
void AddDisplayPickup(short objectNumber);
int DrawAllPickups();
void InitialisePickupDisplay();
int FlashIt();

extern short PickupX;
extern short PickupY;
extern short CurrentPickup;
extern DISPLAY_PICKUP Pickups[NUM_PICKUPS];
extern short PickupVel;
extern int OldHitPoints;
extern int HealtBarTimer;
extern int FlashState;
extern int PoisonFlag;
extern int DashTimer;

void Inject_Healt();

