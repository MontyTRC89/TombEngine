#pragma once
#include "Specific/phd_global.h"
#include "Specific/level.h"

struct ItemInfo;
struct BITE_INFO;
struct CreatureInfo;
struct LOTInfo;

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
	int verticalDistance;
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

#define CreatureEffectFunction short(int x, int y, int z, short speed, short yRot, short roomNumber)

constexpr auto BOX_BLOCKED = (1 << 14); // unpassable for other enemies, always set for movable blocks & closed doors
constexpr auto BOX_LAST = (1 << 15); // unpassable by large enemies (T-Rex, Centaur, etc), always set behind doors
constexpr auto TIMID = 0;
constexpr auto VIOLENT = 1;
constexpr auto REVERSE = 0x4000;
constexpr auto BLOCKABLE = 0x8000;
constexpr auto BLOCKED = 0x4000;
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

void GetCreatureMood(ItemInfo* item, AI_INFO* AI, int violent);
void CreatureMood(ItemInfo* item, AI_INFO* AI, int violent);
void FindAITargetObject(CreatureInfo* creature, short objectNumber);
void GetAITarget(CreatureInfo* creature);
int CreatureVault(short itemNumber, short angle, int vault, int shift);
void DropEntityPickups(ItemInfo* item);
bool MoveCreature3DPos(PHD_3DPOS* origin, PHD_3DPOS* target, int velocity, short angleDif, int angleAdd);
void CreatureYRot2(PHD_3DPOS* srcPos, short angle, short angleAdd);
bool SameZone(CreatureInfo* creature, ItemInfo* target);
void FindAITargetObject(CreatureInfo* creature, short objNum);
short AIGuard(CreatureInfo* creature);
void AlertNearbyGuards(ItemInfo* item);
void AlertAllGuards(short itemNumber);
void CreatureKill(ItemInfo* item, int killAnim, int killState, int laraKillState);
short CreatureEffect2(ItemInfo* item, BITE_INFO* bite, short damage, short angle, std::function<CreatureEffectFunction> func);
short CreatureEffect(ItemInfo* item, BITE_INFO* bite, std::function<CreatureEffectFunction> func);
void CreatureUnderwater(ItemInfo* item, int depth);
void CreatureFloat(short itemNumber);
void CreatureJoint(ItemInfo* item, short joint, short required);
void CreatureTilt(ItemInfo* item, short angle);
short CreatureTurn(ItemInfo* item, short maxTurn);
void CreatureDie(short itemNumber, bool explode);
int BadFloor(int x, int y, int z, int boxHeight, int nextHeight, short roomNumber, LOTInfo* LOT);
int CreatureCreature(short itemNumber);
int ValidBox(ItemInfo* item, short zoneNumber, short boxNumber);
int EscapeBox(ItemInfo* item, ItemInfo* enemy, int boxNumber);
void TargetBox(LOTInfo* LOT, int boxNumber);
int UpdateLOT(LOTInfo* LOT, int expansion);
int SearchLOT(LOTInfo* LOT, int expansion);
int CreatureActive(short itemNumber);
void InitialiseCreature(short itemNumber);
int StalkBox(ItemInfo* item, ItemInfo* enemy, int boxNumber);
void CreatureAIInfo(ItemInfo* item, AI_INFO* AI);
TARGET_TYPE CalculateTarget(Vector3Int* target, ItemInfo* item, LOTInfo* LOT);
int CreatureAnimation(short itemNumber, short angle, short tilt);
void CreatureSwitchRoom(short itemNumber);
void AdjustStopperFlag(ItemInfo* item, int direction, bool set);
void InitialiseItemBoxData();


void DrawBox(int boxIndex, Vector3 color);
void DrawNearbyPathfinding(int boxIndex);