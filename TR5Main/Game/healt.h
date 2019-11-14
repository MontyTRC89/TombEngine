#pragma once

#include "..\Global\global.h"

//#define InitialisePickUpDisplay ((void (__cdecl*)()) 0x0043A0E0)
#define FlashIt ((__int32 (__cdecl*)()) 0x00439C10)
#define UpdateHealtBar ((void (__cdecl*)(__int32)) 0x00439E50)
#define UpdateAirBar ((void (__cdecl*)(__int32)) 0x00439FC0)

void __cdecl DrawHealtBar(__int32 percentual);
void __cdecl DrawAirBar(__int32 percentual);
void __cdecl AddDisplayPickup(__int16 objectNumber);
__int32 __cdecl DrawAllPickups();
void __cdecl InitialisePickUpDisplay();

extern __int16 PickupX;
extern __int16 PickupY;
extern __int16 CurrentPickup;
extern DISPLAY_PICKUP Pickups[NUM_PICKUPS];
extern __int16 PickupVel;

void Inject_Healt();

