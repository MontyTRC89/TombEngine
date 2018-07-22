#pragma once

#include "..\Global\global.h"

#define SmashObject ((__int32 (__cdecl*)(__int16)) 0x00465200)

void __cdecl SmashItem(__int16 itemNum);

void Inject_LaraFire();