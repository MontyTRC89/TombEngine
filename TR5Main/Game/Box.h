#pragma once
#include "global.h"

enum ZoneTypeEnum
{
	ZONE_NULL = -1,  // default zone
	ZONE_SKELLY = 0,
	ZONE_BASIC,
	ZONE_FLYER,
	ZONE_HUMAN_CLASSIC,
	ZONE_WATER,
	ZONE_MAX,
	/// custom zone (using zone above for LOT.zone):
	ZONE_HUMAN_JUMP_AND_MONKEY,
	ZONE_HUMAN_JUMP,
	ZONE_SPIDER,
	ZONE_BLOCKABLE, // for trex, shiva, etc..
	ZONE_SOPHIALEE, // dont want sophia to go down again !
	ZONE_APE,       // only 2 click climb
};

struct BOX_NODE
{
	short exitBox;
	unsigned short searchNumber;
	short nextExpansion;
	short boxNumber;
};

struct BOX_INFO
{
	unsigned char left;
	unsigned char right;
	unsigned char top;
	unsigned char bottom;
	short height;
	short overlapIndex;
};

struct AI_INFO
{
	short zoneNumber;
	short enemyZone;
	int distance;
	int ahead;
	int bite;
	short angle;
	short xAngle;
	short enemyFacing;
};

struct BITE_INFO
{
	int	x;
	int	y;
	int	z;
	int	meshNum;
};

struct LOT_INFO
{
	BOX_NODE* node;
	short head;
	short tail;
	unsigned short searchNumber;
	unsigned short blockMask;
	short step;
	short drop;
	short zoneCount;
	short targetBox;
	short requiredBox;
	short fly;
	bool canJump;
	bool canMonkey;
	bool isAmphibious;
	bool isJumping;
	bool isMonkeying;
	PHD_VECTOR target;
	ZoneTypeEnum zone;
};

struct CREATURE_INFO
{
	short jointRotation[4];
	short maximumTurn;
	short flags;
	bool alerted;
	bool headLeft;
	bool headRight;
	bool reachedGoal;
	bool hurtByLara;
	bool patrol2;
	bool jumpAhead;
	bool monkeyAhead;
	MOOD_TYPE mood;
	ITEM_INFO* enemy;
	ITEM_INFO aiTarget;
	short pad;
	short itemNum;
	PHD_VECTOR target;
	LOT_INFO LOT;
};

extern int NumberBoxes;
extern BOX_INFO* Boxes;
extern int NumberOverlaps;
extern short* Overlaps;
extern short* Zones[ZONE_MAX][2];

#define CreatureEffectFunction short(int x, int y, int z, short speed, short yRot, short roomNumber)

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
short CreatureEffect2(ITEM_INFO* item, BITE_INFO* bite, short damage, short angle, function<CreatureEffectFunction> func);
short CreatureEffect(ITEM_INFO* item, BITE_INFO* bite, function<CreatureEffectFunction> func);
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


