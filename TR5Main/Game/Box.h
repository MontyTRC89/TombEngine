#pragma once

#include "..\Global\global.h"

//#define CreatureActive ((int (__cdecl*)(short)) 0x00408630)
//#define _CreatureUnderwater ((void (__cdecl*)(ITEM_INFO*, int)) 0x0040B400)
//#define _CreatureFloat ((int (__cdecl*)(short)) 0x0040B2C0)
//#define CreatureDie ((void (__cdecl*)(short, int)) 0x0040A090)
//#define _CreatureAIInfo ((void (__cdecl*)(ITEM_INFO*, AI_INFO*)) 0x004086C0)
//#define _CreatureMood ((void (__cdecl*)(ITEM_INFO*, AI_INFO*, int)) 0x00409370)
//#define _GetCreatureMood ((void (__cdecl*)(ITEM_INFO*, AI_INFO*, int)) 0x004090A0)
//#define CreatureEffect ((short (__cdecl*)(ITEM_INFO*, BITE_INFO*, short(*)(int, int, int, short, short, short))) 0x0040B4D0)
//#define CreatureEffect2 ((short (__cdecl*)(ITEM_INFO*, BITE_INFO*, short, short, short(*)(int, int, int, short, short, short))) 0x0040B550)
//#define CreatureTilt ((void (__cdecl*)(ITEM_INFO*, short)) 0x0040B1B0)
//#define CreatureJoint ((void (__cdecl*)(ITEM_INFO*, short, short)) 0x0040B240)
//#define _CreatureAnimation ((void (__cdecl*)(short, short, short)) 0x0040A1D0)
//#define _InitialiseCreature ((void (__cdecl*)(short)) 0x00408550)
//#define CreatureKill ((void (__cdecl*)(ITEM_INFO*, int, int, int)) 0x0040B820)
//#define AlertAllGuards ((void (__cdecl*)(short)) 0x0040BA70)
//#define AlertNearbyGuards ((void (__cdecl*)(ITEM_INFO*)) 0x0040BB10)
//#define MoveCreature3DPos ((int(__cdecl*)(PHD_3DPOS*, PHD_3DPOS*, int, short, short)) 0x0040C460)
//#define _CalculateTarget ((int(__cdecl*)(PHD_VECTOR*, ITEM_INFO*, LOT_INFO*)) 0x004098B0)
//#define ValidBox ((int(__cdecl*)(ITEM_INFO*, short, short)) 0x00408FD0)
//#define StalkBox ((int(__cdecl*)(ITEM_INFO*, ITEM_INFO*, short)) 0x00409770)
//#define TargetBox ((int(__cdecl*)(LOT_INFO*, short)) 0x00408E20)
//#define EscapeBox ((int(__cdecl*)(ITEM_INFO*, ITEM_INFO*, short)) 0x00408EF0)
//#define BadFloor ((int(__cdecl*)(int, int, int, int, int, short, LOT_INFO*)) 0x00409FB0)
//#define CreatureCreature ((int(__cdecl*)(short)) 0x00409E20)
//#define DropBaddyPickups ((int(__cdecl*)(ITEM_INFO*)) 0x0040C5A0)
//#define CreatureCollision ((void (__cdecl*)(short, ITEM_INFO*, COLL_INFO*)) 0x004124E0)
#define CreatureTurn ((short (__cdecl*)(ITEM_INFO*, short)) 0x0040AE90)
#define LOS ((int (__cdecl*)(GAME_VECTOR*, GAME_VECTOR*)) 0x00417CF0)
#define _AIGuard ((short(__cdecl*)(CREATURE_INFO*)) 0x0040BBE0)
#define _GetAITarget ((void(__cdecl*)(CREATURE_INFO*)) 0x0040BCC0)
#define _FindAIObject ((int(__cdecl*)(CREATURE_INFO*, short)) 0x0040C070)
#define _SameZoneAIObject ((int(__cdecl*)(CREATURE_INFO*, short)) 0x0040C460)

void GetCreatureMood(ITEM_INFO* item, AI_INFO* info, int violent);
void CreatureMood(ITEM_INFO* item, AI_INFO* info, int violent);
void FindAITargetObject(CREATURE_INFO* creature, short objectNumber);
void GetAITarget(CREATURE_INFO* creature);
int CreatureVault(short itemNum, short angle, int vault, int shift);
void DropBaddyPickups(ITEM_INFO* item);
int MoveCreature3DPos(PHD_3DPOS* srcpos, PHD_3DPOS* destpos, int velocity, short angdif, int angadd);
void CreatureYRot2(PHD_3DPOS* srcpos, short angle, short angadd);
short SameZone(CREATURE_INFO* creature, ITEM_INFO* targetItem);
void FindAITargetObject(CREATURE_INFO* creature, short objNum);
short AIGuard(CREATURE_INFO* creature);
void AlertNearbyGuards(ITEM_INFO* item);
void AlertAllGuards(short itemNumber);
void CreatureKill(ITEM_INFO* item, int killAnim, int killState, short laraAnim);
short CreatureEffect2(ITEM_INFO* item, BITE_INFO* bite, short damage, short angle, short (*generate)(int x, int y, int z, short speed, short yrot, short roomNumber));
short CreatureEffect(ITEM_INFO* item, BITE_INFO* bite, short(*generate)(int x, int y, int z, short speed, short yrot, short roomNumber));
void CreatureUnderwater(ITEM_INFO* item, int depth);
void CreatureFloat(short itemNumber);
void CreatureJoint(ITEM_INFO* item, short joint, short required);
void CreatureTilt(ITEM_INFO* item, short angle);
//short _CreatureTurn(ITEM_INFO* item, short maximumTurn);
void CreatureDie(short itemNumber, int explode);
int BadFloor(int x, int y, int z, int boxHeight, int nextHeight, short roomNumber, LOT_INFO* LOT);
int CreatureCreature(short itemNumber);
int ValidBox(ITEM_INFO* item, short zoneNumber, short boxNumber);
int EscapeBox(ITEM_INFO* item, ITEM_INFO* enemy, short boxNumber);
void TargetBox(LOT_INFO* LOT, short boxNumber);
int UpdateLOT(LOT_INFO* LOT, int expansion);
int SearchLOT(LOT_INFO* LOT, int expansion);
int CreatureActive(short itemNumber);
void InitialiseCreature(short itemNumber);
int StalkBox(ITEM_INFO* item, ITEM_INFO* enemy, short boxNumber);
void CreatureAIInfo(ITEM_INFO* item, AI_INFO* info);
TARGET_TYPE CalculateTarget(PHD_VECTOR* target, ITEM_INFO* item, LOT_INFO* LOT);
int CreatureAnimation(short itemNumber, short angle, short tilt);
long mgLOS(GAME_VECTOR* start, GAME_VECTOR* target, long push);

void Inject_Box();

