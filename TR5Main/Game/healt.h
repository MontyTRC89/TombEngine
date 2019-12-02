#pragma once

#include "..\Global\global.h"

//#define InitialisePickupDisplay ((void (__cdecl*)()) 0x0043A0E0)
#define FlashIt ((int (__cdecl*)()) 0x00439C10)
#define UpdateHealtBar ((void (__cdecl*)(int)) 0x00439E50)
#define UpdateAirBar ((void (__cdecl*)(int)) 0x00439FC0)

void __cdecl DrawHealtBar(int percentual);
void __cdecl DrawAirBar(int percentual);
void __cdecl AddDisplayPickup(short objectNumber);
int __cdecl DrawAllPickups();
void __cdecl InitialisePickupDisplay();

extern short PickupX;
extern short PickupY;
extern short CurrentPickup;
extern DISPLAY_PICKUP Pickups[NUM_PICKUPS];
extern short PickupVel;

void Inject_Healt();

