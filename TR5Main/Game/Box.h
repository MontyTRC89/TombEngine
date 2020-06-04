#pragma once
#include "phd_global.h"
#include "items.h"
#include "level.h"

typedef enum MOOD_TYPE
{
	BORED_MOOD,
	ATTACK_MOOD,
	ESCAPE_MOOD,
	STALK_MOOD
};

typedef enum TARGET_TYPE
{
	NO_TARGET,
	PRIME_TARGET,
	SECONDARY_TARGET
};

typedef enum ZoneType
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

typedef struct BOX_NODE
{
	short exitBox;
	unsigned short searchNumber;
	short nextExpansion;
	short boxNumber;
};

typedef struct BOX_INFO
{
	unsigned char left;
	unsigned char right;
	unsigned char top;
	unsigned char bottom;
	short height;
	short overlapIndex;
};

typedef struct AI_INFO
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

typedef struct BITE_INFO
{
	int	x;
	int	y;
	int	z;
	int	meshNum;
};

typedef struct LOT_INFO
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
	ZoneType zone;
};

typedef struct CREATURE_INFO
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

#define CreatureEffectFunction short(int x, int y, int z, short speed, short yRot, short roomNumber)
#define XZ_GET_SECTOR(room, x, z) (room->floor[((z) >> WALL_SHIFT) + ((x) >> WALL_SHIFT) * room->xSize])

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
constexpr auto DATA_TYPE = 0x1F;
constexpr auto DATA_TILT = 0xF;    // tile type (FLOOR_TYPE enum)
constexpr auto DATA_STATIC = 0xFF; // probably add static collision
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
constexpr auto NO_BOX = 0x7FF;
constexpr auto BOX_JUMP = 0x800;
constexpr auto BOX_MONKEY = 0x2000;
constexpr auto BOX_NUMBER = 0x7FF;
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
constexpr auto SLOPE_DIF = 60;

extern int NumberBoxes;
extern BOX_INFO* Boxes;
extern int NumberOverlaps;
extern short* Overlaps;
extern short* Zones[ZONE_MAX][2];

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


