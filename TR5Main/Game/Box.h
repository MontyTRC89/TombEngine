#pragma once

#include "..\Global\global.h"

//#define CreatureActive ((__int32 (__cdecl*)(__int16)) 0x00408630)
//#define CreatureUnderwater ((void (__cdecl*)(ITEM_INFO*, __int32)) 0x0040B400)
//#define CreatureFloat ((__int32 (__cdecl*)(__int16)) 0x0040B2C0)
//#define CreatureDie ((void (__cdecl*)(__int16, __int32)) 0x0040A090)
//#define CreatureAIInfo ((void (__cdecl*)(ITEM_INFO*, AI_INFO*)) 0x004086C0)
//#define CreatureTurn ((__int16 (__cdecl*)(ITEM_INFO*, __int16)) 0x0040AE90)
//#define CreatureMood ((void (__cdecl*)(ITEM_INFO*, AI_INFO*, __int32)) 0x00409370)
//#define GetCreatureMood ((void (__cdecl*)(ITEM_INFO*, AI_INFO*, __int32)) 0x004090A0)
//#define CreatureEffect ((__int16 (__cdecl*)(ITEM_INFO*, BITE_INFO*, __int16(*)(__int32, __int32, __int32, __int16, __int16, __int16))) 0x0040B4D0)
//#define CreatureEffect2 ((__int16 (__cdecl*)(ITEM_INFO*, BITE_INFO*, __int16, __int16, __int16(*)(__int32, __int32, __int32, __int16, __int16, __int16))) 0x0040B550)
//#define CreatureTilt ((void (__cdecl*)(ITEM_INFO*, __int16)) 0x0040B1B0)
//#define CreatureJoint ((void (__cdecl*)(ITEM_INFO*, __int16, __int16)) 0x0040B240)
#define CreatureAnimation ((void (__cdecl*)(__int16, __int16, __int16)) 0x0040A1D0)
#define CreatureCollision ((void (__cdecl*)(__int16, ITEM_INFO*, COLL_INFO*)) 0x004124E0)
//#define InitialiseCreature ((void (__cdecl*)(__int16)) 0x00408550)
//#define CreatureKill ((void (__cdecl*)(ITEM_INFO*, __int32, __int32, __int32)) 0x0040B820)
#define LOS ((__int32 (__cdecl*)(GAME_VECTOR*, GAME_VECTOR*)) 0x00417CF0) 
//#define AlertAllGuards ((void (__cdecl*)(__int16)) 0x0040BA70)
//#define AlertNearbyGuards ((void (__cdecl*)(ITEM_INFO*)) 0x0040BB10)
//#define AIGuard ((__int16(__cdecl*)(CREATURE_INFO*)) 0x0040BBE0)
//#define GetAITarget ((void(__cdecl*)(CREATURE_INFO*)) 0x0040BCC0)
//#define MoveCreature3DPos ((__int32(__cdecl*)(PHD_3DPOS*, PHD_3DPOS*, __int32, __int16, __int16)) 0x0040C460)

void __cdecl GetCreatureMood(ITEM_INFO* item, AI_INFO* info, __int32 violent);
void __cdecl CreatureMood(ITEM_INFO* item, AI_INFO* info, __int32 violent);
void __cdecl FindAITargetObject(CREATURE_INFO* creature, __int16 objectNumber);
void __cdecl GetAITarget(CREATURE_INFO* creature);
__int32 __cdecl CreatureVault(__int16 itemNum, __int16 angle, __int32 vault, __int32 shift);
void __cdecl DropBaddyPickups(ITEM_INFO* item);
__int32 __cdecl MoveCreature3DPos(PHD_3DPOS* srcpos, PHD_3DPOS* destpos, __int32 velocity, __int16 angdif, __int32 angadd);
void __cdecl CreatureYRot(PHD_3DPOS* srcpos, __int16 angle, __int16 angadd);
__int16 __cdecl SameZone(CREATURE_INFO* creature, ITEM_INFO* targetItem);
void __cdecl FindAITargetObject(CREATURE_INFO* creature, __int16 objNum);
__int16 __cdecl AIGuard(CREATURE_INFO* creature);
void __cdecl AlertNearbyGuards(ITEM_INFO* item);
void __cdecl AlertAllGuards(__int16 itemNumber);
void __cdecl CreatureKill(ITEM_INFO* item, __int32 killAnim, __int32 killState, __int16 laraAnim);
__int16 __cdecl CreatureEffect2(ITEM_INFO* item, BITE_INFO* bite, __int16 damage, __int16 angle, __int16 (*generate)(__int32 x, __int32 y, __int32 z, __int16 speed, __int16 yrot, __int16 roomNumber));
__int16 __cdecl CreatureEffect(ITEM_INFO* item, BITE_INFO* bite, __int16(*generate)(__int32 x, __int32 y, __int32 z, __int16 speed, __int16 yrot, __int16 roomNumber));
void __cdecl CreatureUnderwater(ITEM_INFO* item, __int32 depth);
void __cdecl CreatureFloat(__int16 itemNumber);
void __cdecl CreatureJoint(ITEM_INFO* item, __int16 joint, __int16 required);
void __cdecl CreatureTilt(ITEM_INFO* item, __int16 angle);
__int16 __cdecl CreatureTurn(ITEM_INFO* item, __int16 maximumTurn);
void __cdecl CreatureDie(__int16 itemNumber, __int32 explode);
__int32 __cdecl BadFloor(__int32 x, __int32 y, __int32 z, __int32 boxHeight, __int32 nextHeight, __int32 roomNumber, LOT_INFO* LOT);
__int32 __cdecl CreatureCreature(__int16 itemNumber);
__int32 __cdecl ValidBox(ITEM_INFO* item, __int16 zoneNumber, __int16 boxNumber);
__int32 __cdecl EscapeBox(ITEM_INFO* item, ITEM_INFO* enemy, __int16 boxNumber);
void __cdecl TargetBox(LOT_INFO* LOT, __int16 boxNumber);
__int32 __cdecl UpdateLOT(LOT_INFO* LOT, __int32 expansion);
__int32 __cdecl SearchLOT(LOT_INFO* LOT, __int32 expansion);
__int32 __cdecl CreatureActive(__int16 itemNumber);
void __cdecl InitialiseCreature(__int16 itemNumber);
__int32 __cdecl StalkBox(ITEM_INFO* item, ITEM_INFO* enemy, __int16 boxNumber);
void __cdecl CreatureAIInfo(ITEM_INFO* item, AI_INFO* info);

void Inject_Box();

