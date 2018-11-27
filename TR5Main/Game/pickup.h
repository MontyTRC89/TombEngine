#pragma once

#include "..\Global\global.h"

#define RegeneratePickups ((void (__cdecl*)()) 0x00467AF0)
#define PuzzleHoleCollision ((void (__cdecl*)(__int16, ITEM_INFO*, COLL_INFO*)) 0x00468C70)
#define PickupCollision ((void (__cdecl*)(__int16, ITEM_INFO*, COLL_INFO*)) 0x00467C00)
#define InitialisePickup ((void (__cdecl*)(__int16)) 0x0043E260)
#define PickupControl ((void (__cdecl*)(__int16)) 0x004679D0)

__int32 __cdecl DrawAllPickups();
void __cdecl PickedUpObject(__int16 objectNumber);

void Inject_Pickup();

