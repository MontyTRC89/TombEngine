#pragma once
#include "Objects/objectslist.h"
#include "Math/Math.h"
#include "Renderer/Renderer11Enums.h"
#include "Specific/level.h"

enum class ZoneType;
struct CollisionInfo;
struct ItemInfo;

constexpr auto DEFAULT_RADIUS = 10;

enum JointRotationFlags
{
	ROT_X = (1 << 2),
	ROT_Y = (1 << 3),
	ROT_Z = (1 << 4)
};

enum HitEffectEnum
{
    HIT_NONE,
    HIT_BLOOD,
    HIT_SMOKE,
    HIT_RICOCHET,
	HIT_SPECIAL,
    MAX_HIT_EFFECT
};

enum ShatterType
{
	SHT_NONE,
	SHT_FRAGMENT,
	SHT_EXPLODE
};

struct ObjectInfo
{
	int nmeshes; 
	int meshIndex;
	int boneIndex; 
	int frameBase;
	std::function<void(short itemNumber)> initialise;
	std::function<void(short itemNumber)> control;
	std::function<std::optional<int>(short itemNumber, int x, int y, int z)> floor;
	std::function<std::optional<int>(short itemNumber, int x, int y, int z)> ceiling;
	std::function<int(short itemNumber)> floorBorder;
	std::function<int(short itemNumber)> ceilingBorder;
	std::function<void(ItemInfo* item)> drawRoutine;
	std::function<void(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)> collision;
	ZoneType ZoneType;
	int animIndex; 
	short HitPoints; 
	short pivotLength; 
	short radius; 
	ShadowMode shadowType;
	short biteOffset; 
	bool loaded;
	bool intelligent;
	bool nonLot;
	bool waterCreature;
	bool usingDrawAnimatingItem;
	HitEffectEnum hitEffect;
	bool undead;
	bool isPickup;
	bool isPuzzleHole;
	int meshSwapSlot;
	DWORD explodableMeshbits;

	// Use ROT_X/Y/Z to allow bone to be rotated with CreatureJoint().
	void SetBoneRotation(int boneID, int flags)
	{
		g_Level.Bones[boneIndex + boneID * 4] |= flags;
	}
};

struct STATIC_INFO
{
	int meshNumber;
	int flags;
	GameBoundingBox visibilityBox;
	GameBoundingBox collisionBox;
	int shatterType;
	int shatterSound;
};

constexpr auto MAX_STATICS = 1000;
constexpr auto SF_NO_COLLISION = 0x01;
constexpr auto SF_SHATTERABLE = 0x02;
constexpr auto GRAVITY = 6.0f;
constexpr auto SWAMP_GRAVITY = GRAVITY / 3.0f;

extern ObjectInfo Objects[ID_NUMBER_OBJECTS];
extern STATIC_INFO StaticObjects[MAX_STATICS];

void InitialiseGameFlags();
void InitialiseSpecialEffects();
void InitialiseHair();
void InitialiseObjects();
