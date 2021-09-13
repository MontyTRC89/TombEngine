#pragma once
#include "Specific\phd_global.h"
#include "level.h"
struct ITEM_INFO;
struct BITE_INFO;
struct CREATURE_INFO;
struct LOT_INFO;
enum TARGET_TYPE
{
	NO_TARGET,
	PRIME_TARGET,
	SECONDARY_TARGET
};



struct OBJECT_BONES
{
	short bone0;
	short bone1;
	short bone2;
	short bone3;

	OBJECT_BONES()
	{
		this->bone0 = 0;
		this->bone1 = 0;
		this->bone2 = 0;
		this->bone3 = 0;
	}

	OBJECT_BONES(short all)
	{
		this->bone0 = all;
		this->bone1 = all;
		this->bone2 = all;
		this->bone3 = all;
	}

	OBJECT_BONES(short angleY, short angleX)
	{
		this->bone0 = angleY;
		this->bone1 = angleX;
		this->bone2 = angleY;
		this->bone3 = angleX;
	}

	OBJECT_BONES(short angleY, short angleX, bool total)
	{
		this->bone0 = angleY;
		this->bone1 = angleX;
		if (total)
		{
			this->bone2 = angleY;
			this->bone3 = angleX;
		}
		else
		{
			this->bone2 = 0;
			this->bone3 = 0;
		}
	}
};

struct AI_INFO
{
	int zoneNumber;
	int enemyZone;
	int distance;
	int ahead;
	int bite;
	short angle;
	short xAngle;
	short enemyFacing;
};

struct BOX_INFO
{
	unsigned int left;
	unsigned int right;
	unsigned int top;
	unsigned int bottom;
	int height;
	int overlapIndex;
	int flags;
};

struct OVERLAP
{
	int box;
	int flags;
};

struct BITE_INFO
{
	int	x;
	int	y;
	int	z;
	int	meshNum;

	BITE_INFO()
	{
		this->x = 0;
		this->y = 0;
		this->z = 0;
		this->meshNum = 0;
	}

	BITE_INFO(int xpos, int ypos, int zpos, int meshNumber)
	{
		this->x = xpos;
		this->y = ypos;
		this->z = zpos;
		this->meshNum = meshNumber;
	}
};



struct EntityStoringInfo
{
	// position of the entity
	int x;
	int y;
	int z;
	// waterLevel is mostly -NO_HEIGHT but if the position are in water then it's 0
	// waterDepth is the depth starting for the water room ceiling (0) and increase 1 by 1
	// to store from GetWaterDepth() and GetWaterHeight()
	int waterLevel;
	int waterDepth;
	// to store roomNumber from GetFloor()
	short roomNumber;
	// store the boxNumber from LOT or from room->floor[].box
	short boxNumber;

	EntityStoringInfo()
	{
		this->x = 0;
		this->y = 0;
		this->z = 0;
		this->waterLevel = 0;
		this->waterDepth = 0;
		this->roomNumber = 0;
		this->boxNumber = 0;
	}

	EntityStoringInfo(int xpos, int ypos, int zpos)
	{
		this->x = xpos;
		this->y = ypos;
		this->z = zpos;
		this->waterLevel = 0;
		this->waterDepth = 0;
		this->roomNumber = 0;
		this->boxNumber = 0;
	}
};

#define CreatureEffectFunction short(int x, int y, int z, short speed, short yRot, short roomNumber)

constexpr auto UNIT_SHADOW = 256;
constexpr auto NO_SHADOW = 0;
constexpr auto DEFAULT_RADIUS = 10;
constexpr auto ROT_X = 0x0004;
constexpr auto ROT_Y = 0x0008;
constexpr auto ROT_Z = 0x0010;
constexpr auto BOX_BLOCKED = (1 << 14); // unpassable for other enemies, always set for movable blocks & closed doors
constexpr auto BOX_LAST = (1 << 15); // unpassable by large enemies (T-Rex, Centaur, etc), always set behind doors
constexpr auto TIMID = 0;
constexpr auto VIOLENT = 1;
constexpr auto ONESHOT = 0x100;
constexpr auto END_BIT = 0x8000;
constexpr auto VALUE_BITS = 0x3FF;
constexpr auto CODE_BITS = 0x3E00;
constexpr auto REVERSE = 0x4000;
constexpr auto SWONESHOT = 0x40;
constexpr auto ATONESHOT = 0x80;
constexpr auto BLOCKABLE = 0x8000;
constexpr auto BLOCKED = 0x4000;
constexpr auto OVERLAP_INDEX = 0x3FFF;
constexpr auto SEARCH_NUMBER = 0x7FFF;
constexpr auto BLOCKED_SEARCH = 0x8000;
constexpr auto NO_BOX = -1;
constexpr auto NO_ZONE = -1;
constexpr auto BOX_JUMP = 0x800;
constexpr auto BOX_MONKEY = 0x2000;
constexpr auto BOX_END_BIT = 0x8000;
constexpr auto EXPAND_LEFT = 0x1;
constexpr auto EXPAND_RIGHT = 0x2;
constexpr auto EXPAND_TOP = 0x4;
constexpr auto EXPAND_BOTTOM = 0x8;
constexpr auto NO_FLYING = 0;
constexpr auto FLY_ZONE = 0x2000;
constexpr auto CLIP_LEFT = 0x1;
constexpr auto CLIP_RIGHT = 0x2;
constexpr auto CLIP_TOP = 0x4;
constexpr auto CLIP_BOTTOM = 0x8;
constexpr auto SECONDARY_CLIP = 0x10;
constexpr auto ALL_CLIP = (CLIP_LEFT | CLIP_RIGHT | CLIP_TOP | CLIP_BOTTOM);

void GetCreatureMood(ITEM_INFO* item, AI_INFO* info, int violent);
void CreatureMood(ITEM_INFO* item, AI_INFO* info, int violent);
void FindAITargetObject(CREATURE_INFO* creature, short objectNumber);
void GetAITarget(CREATURE_INFO* creature);
int CreatureVault(short itemNum, short angle, int vault, int shift);
void DropBaddyPickups(ITEM_INFO* item);
int MoveCreature3DPos(PHD_3DPOS* srcpos, PHD_3DPOS* destpos, int velocity, short angdif, int angadd);
void CreatureYRot2(PHD_3DPOS* srcpos, short angle, short angadd);
bool SameZone(CREATURE_INFO* creature, ITEM_INFO* target);
void FindAITargetObject(CREATURE_INFO* creature, short objNum);
short AIGuard(CREATURE_INFO* creature);
void AlertNearbyGuards(ITEM_INFO* item);
void AlertAllGuards(short itemNumber);
void CreatureKill(ITEM_INFO* item, int killAnim, int killState, short laraAnim);
short CreatureEffect2(ITEM_INFO* item, BITE_INFO* bite, short damage, short angle, std::function<CreatureEffectFunction> func);
short CreatureEffect(ITEM_INFO* item, BITE_INFO* bite, std::function<CreatureEffectFunction> func);
void CreatureUnderwater(ITEM_INFO* item, int depth);
void CreatureFloat(short itemNumber);
void CreatureJoint(ITEM_INFO* item, short joint, short required);
void CreatureTilt(ITEM_INFO* item, short angle);
short CreatureTurn(ITEM_INFO* item, short maximumTurn);
void CreatureDie(short itemNumber, int explode);
int BadFloor(int x, int y, int z, int boxHeight, int nextHeight, short roomNumber, LOT_INFO* LOT);
int CreatureCreature(short itemNumber);
int ValidBox(ITEM_INFO* item, short zoneNumber, short boxNumber);
int EscapeBox(ITEM_INFO* item, ITEM_INFO* enemy, int boxNumber);
void TargetBox(LOT_INFO* LOT, int boxNumber);
int UpdateLOT(LOT_INFO* LOT, int expansion);
int SearchLOT(LOT_INFO* LOT, int expansion);
int CreatureActive(short itemNumber);
void InitialiseCreature(short itemNumber);
int StalkBox(ITEM_INFO* item, ITEM_INFO* enemy, int boxNumber);
void CreatureAIInfo(ITEM_INFO* item, AI_INFO* info);
TARGET_TYPE CalculateTarget(PHD_VECTOR* target, ITEM_INFO* item, LOT_INFO* LOT);
int CreatureAnimation(short itemNumber, short angle, short tilt);
void AdjustStopperFlag(ITEM_INFO* item, int dir, int set);
//(room->floor[ +  * room->xSize])
FLOOR_INFO* XZ_GET_SECTOR(ROOM_INFO* r, int x, int z); 

