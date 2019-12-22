#pragma once

#include "..\Global\global.h"

#define FlashIt ((int (__cdecl*)()) 0x00439C10)
#define UpdateHealtBar ((void (__cdecl*)(int)) 0x00439E50)
#define UpdateAirBar ((void (__cdecl*)(int)) 0x00439FC0)

void DrawHealtBar(int percentual);
void DrawAirBar(int percentual);
void AddDisplayPickup(short objectNumber);
int DrawAllPickups();
void InitialisePickupDisplay();

extern short PickupX;
extern short PickupY;
extern short CurrentPickup;
extern DISPLAY_PICKUP Pickups[NUM_PICKUPS];
extern short PickupVel;

void Inject_Healt();

