#pragma once

#define InitialisePickUpDisplay ((void (__cdecl*)()) 0x0043A0E0)
#define FlashIt ((__int32 (__cdecl*)()) 0x00439C10)
#define UpdateHealtBar ((void (__cdecl*)(__int32)) 0x00439E50)
#define UpdateAirBar ((void (__cdecl*)(__int32)) 0x00439FC0)

void __cdecl DrawHealtBar(__int32 percentual);
void __cdecl DrawAirBar(__int32 percentual);

void Inject_Healt();

