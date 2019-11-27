#pragma once

#include "..\Global\global.h"

#define InitialiseRope ((void (__cdecl*)(short)) 0x0046F060)
#define RopeControl ((void (__cdecl*)(short)) 0x0046DD40)
#define RopeCollision ((void (__cdecl*)(short,ITEM_INFO*,COLL_INFO*)) 0x0046DAE0)
#define CalculateRopePoints ((void (__cdecl*)(ROPE_STRUCT*)) 0x0046EC70)