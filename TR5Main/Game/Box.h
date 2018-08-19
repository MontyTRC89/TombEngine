#pragma once

#include "..\Global\global.h"

#define DropBaddyPickups ((void (__cdecl*)(ITEM_INFO*)) 0x0040C5A0)
#define CreatureActive ((__int32 (__cdecl*)(__int16)) 0x00408630)
#define CreatureUnderwater ((void (__cdecl*)(ITEM_INFO*, __int32)) 0x0040B400)
#define CreatureFloat ((__int32 (__cdecl*)(__int16)) 0x0040B2C0)
#define CreatureAIInfo ((void (__cdecl*)(ITEM_INFO*, AI_INFO*)) 0x004086C0)
#define CreatureTurn ((__int16 (__cdecl*)(ITEM_INFO*, __int16)) 0x0040AE90)
#define CreatureMood ((void (__cdecl*)(ITEM_INFO*, AI_INFO*, __int32)) 0x00409370)
#define GetCreatureMood ((void (__cdecl*)(ITEM_INFO*, AI_INFO*, __int32)) 0x004090A0)
#define CreatureEffect ((__int16 (__cdecl*)(ITEM_INFO*, BITE_INFO*, __int16(*)(__int32, __int32, __int32, __int16, __int16, __int16))) 0x0040B4D0)
#define CreatureTilt ((void (__cdecl*)(ITEM_INFO*, __int16)) 0x0040B1B0)
#define CreatureJoint ((void (__cdecl*)(ITEM_INFO*, __int16, __int16)) 0x0040B240)
#define CreatureAnimation ((void (__cdecl*)(__int16, __int16, __int16)) 0x0040A1D0)
#define CreatureCollision ((void (__cdecl*)(__int16, ITEM_INFO*, COLL_INFO*)) 0x004124E0)
#define InitialiseCreature ((void (__cdecl*)(__int16)) 0x00408550)
#define CreatureKill ((void (__cdecl*)(ITEM_INFO*, __int32, __int32, __int32)) 0x0040B820)
#define LOS ((__int32 (__cdecl*)(GAME_VECTOR*, GAME_VECTOR*)) 0x00417CF0) 
#define AlertAllGuards ((void (__cdecl*)(__int16)) 0x0040BA70)
#define AIGuard ((__int16(__cdecl*)(CREATURE_INFO*)) 0x0040BBE0)
#define GetAITarget ((void(__cdecl*)(CREATURE_INFO*)) 0x0040BCC0)

void Inject_Box();

