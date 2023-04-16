#pragma once
#include "Objects/objectslist.h"
#include "Math/Math.h"
#include "Renderer/Renderer11Enums.h"
#include "Specific/level.h"

enum class ZoneType;
struct CollisionInfo;
struct ItemInfo;

constexpr auto DEFAULT_RADIUS = 10;

// Custom LOT definition for Creature. Used in InitialiseSlot() in lot.cpp.
enum class LotType
{
	Skeleton,
	Basic,
	Water,
	WaterAndLand,
	Human,
	HumanPlusJump,
	HumanPlusJumpAndMonkey,
	Flyer,
	Blockable, // For large creatures such as trex and shiva.
	Spider,    // Only 2 block vault allowed.
	Ape		   // Only 2 block vault allowed.
};

enum JointRotationFlags
{
	ROT_X = (1 << 2),
	ROT_Y = (1 << 3),
	ROT_Z = (1 << 4)
};

enum class HitEffect
{
    None,
    Blood,
    Smoke,
    Richochet,
	Special,
    Max
};

enum ShatterType
{
	SHT_NONE,
	SHT_FRAGMENT,
	SHT_EXPLODE
};

struct ObjectInfo
{
	bool loaded = false; // IsLoaded

	int nmeshes; // BoneCount
	int meshIndex; // Base index in g_Level.Meshes.
	int boneIndex; // Base index in g_Level.Bones.
	int animIndex; // Base index in g_Level.Anims.
	int frameBase; // Base index in g_Level.Frames.

	LotType LotType;
	HitEffect hitEffect;
	ShadowMode shadowType;

	int meshSwapSlot;
	int pivotLength;
	int radius;
	int biteOffset;

	int HitPoints;
	bool intelligent;	// IsIntelligent
	bool waterCreature; // IsWaterCreature
	bool undead;		// IsUndead
	bool nonLot;		// IsNonLot
	bool isPickup;		// IsPickup
	bool isPuzzleHole;	// IsReceptacle
	bool usingDrawAnimatingItem;

	DWORD explodableMeshbits;

	std::function<void(short itemNumber)>										   initialise;
	std::function<void(short itemNumber)>										   control;
	std::function<void(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)> collision;

	std::function<void(ItemInfo& target, ItemInfo& source, std::optional<GameVector> pos, int damage, bool isExplosive, int jointIndex)> HitRoutine;
	std::function<void(ItemInfo* item)>																									 drawRoutine;

	std::function<std::optional<int>(int itemNumber, int x, int y, int z)> floor;
	std::function<std::optional<int>(int itemNumber, int x, int y, int z)> ceiling;
	std::function<int(short itemNumber)>								   floorBorder;
	std::function<int(short itemNumber)>								   ceilingBorder;

	/// <summary>
	/// ROT_X/Y/Z allows bones to be rotated with CreatureJoint().
	/// </summary>
	/// <param name="boneNumber:">Mesh number - 1.</param>
	/// <param name="flags:">JointRotationFlags enum.</param>
	void SetBoneRotationFlags(int boneNumber, int flags)
	{
		g_Level.Bones[boneIndex + (boneNumber * 4)] |= flags;
	}

	/// <summary>
	/// Set up hit effect for object based on its value.
	/// </summary>
	/// <param name="isAlive:">Use if object is alive but not intelligent to set up blood effects.</param>
	void SetupHitEffect(bool isSolid = false, bool isAlive = false)
	{
		// Avoid some objects such as ID_SAS_DYING having None.
		if (isAlive)
		{
			hitEffect = HitEffect::Blood;
			return;
		}

		if (intelligent)
		{
			if (isSolid && HitPoints > 0)
			{
				hitEffect = HitEffect::Richochet;
			}
			else if ((undead && HitPoints > 0) || HitPoints == NOT_TARGETABLE)
			{
				hitEffect = HitEffect::Smoke;
			}
			else if (!undead && HitPoints > 0)
			{
				hitEffect = HitEffect::Blood;
			}
		}
		else if (isSolid && HitPoints <= 0)
		{
			hitEffect = HitEffect::Richochet;
		}
		else
		{
			hitEffect = HitEffect::None;
		}
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
void InitialiseObjects();
