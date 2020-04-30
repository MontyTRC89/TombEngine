#pragma once

#include "..\Global\global.h"

extern int NumberBoxes;
extern BOX_INFO* Boxes;
extern int NumberOverlaps;
extern short* Overlaps;
extern short* Zones[5][2];

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
short CreatureTurn(ITEM_INFO* item, short maximumTurn);
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
void AdjustStopperFlag(ITEM_INFO* item, int dir, int set);


