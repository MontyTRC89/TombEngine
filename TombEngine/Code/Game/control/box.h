#pragma once
#include "Specific/level.h"
#include "Math/Math.h"

struct CreatureBiteInfo;
struct CreatureInfo;
struct ItemInfo;
struct LOTInfo;

enum class JumpDistance
{
	Block1,
	Block2
};

enum TARGET_TYPE
{
	NO_TARGET,
	PRIME_TARGET,
	SECONDARY_TARGET
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

#define CreatureEffectFunction short(int x, int y, int z, short speed, short yRot, short roomNumber)

constexpr auto BOX_BLOCKED = (1 << 14); // unpassable for other enemies, always set for movable blocks & closed doors
constexpr auto BOX_LAST = (1 << 15); // unpassable by large enemies (T-Rex, Centaur, etc), always set behind doors

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

void GetCreatureMood(ItemInfo* item, AI_INFO* AI, bool isViolent);
void CreatureMood(ItemInfo* item, AI_INFO* AI, bool isViolent);
void FindAITargetObject(CreatureInfo* creature, int objectNumber);
void FindAITargetObject(CreatureInfo* creature, int objectNumber, int ocb, bool checkSameZone = true);
void GetAITarget(CreatureInfo* creature);
int CreatureVault(short itemNumber, short angle, int vault, int shift);
bool MoveCreature3DPos(Pose* fromPose, Pose* toPose, int velocity, short angleDif, int angleAdd);
void CreatureYRot2(Pose* fromPose, short angle, short angleAdd);
bool SameZone(CreatureInfo* creature, ItemInfo* target);
short AIGuard(CreatureInfo* creature);
void AlertNearbyGuards(ItemInfo* item);
void AlertAllGuards(short itemNumber);
void CreatureKill(ItemInfo* item, int entityKillAnim, int laraExtraKillAnim, int entityKillState, int laraKillState);
short CreatureEffect2(ItemInfo* item, const CreatureBiteInfo& bite, short velocity, short angle, std::function<CreatureEffectFunction> func);
short CreatureEffect(ItemInfo* item, const CreatureBiteInfo& bite, std::function<CreatureEffectFunction> func);
void CreatureUnderwater(ItemInfo* item, int depth);
void CreatureFloat(short itemNumber);
void CreatureJoint(ItemInfo* item, short joint, short required, short maxAngle = ANGLE(70.0f));
void CreatureTilt(ItemInfo* item, short angle);
short CreatureTurn(ItemInfo* item, short maxTurn);
void CreatureDie(short itemNumber, bool explode);
bool BadFloor(int x, int y, int z, int boxHeight, int nextHeight, short roomNumber, LOTInfo* LOT);
int CreatureCreature(short itemNumber);
bool ValidBox(ItemInfo* item, short zoneNumber, short boxNumber);
bool EscapeBox(ItemInfo* item, ItemInfo* enemy, int boxNumber);
void TargetBox(LOTInfo* LOT, int boxNumber);
bool UpdateLOT(LOTInfo* LOT, int expansion);
bool SearchLOT(LOTInfo* LOT, int expansion);
bool CreatureActive(short itemNumber);
void InitializeCreature(short itemNumber);
bool StalkBox(ItemInfo* item, ItemInfo* enemy, int boxNumber);
void CreatureAIInfo(ItemInfo* item, AI_INFO* AI);
TARGET_TYPE CalculateTarget(Vector3i* target, ItemInfo* item, LOTInfo* LOT);
bool CreatureAnimation(short itemNumber, short headingAngle, short tiltAngle);
void CreatureHealth(ItemInfo* item);
void AdjustStopperFlag(ItemInfo* item, int direction);
void InitializeItemBoxData();

bool CanCreatureJump(ItemInfo& item, JumpDistance jumpDistType);

void DrawBox(int boxIndex, Vector3 color);
void DrawNearbyPathfinding(int boxIndex);
