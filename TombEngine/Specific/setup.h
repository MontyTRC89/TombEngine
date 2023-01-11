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

// Custom LOT definition for Creature.
// Used in InitialiseSlot() in lot.cpp.
enum class LOTType
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
	SophiaLee, // Prevents Sophia from going to lower levels again.
	Ape		   // Only 2 block vault allowed.
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
	LOTType lotType = LOTType::Basic;
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
	HitEffect hitEffect;
	bool undead;
	bool isPickup;
	bool isPuzzleHole;
	int meshSwapSlot;
	DWORD explodableMeshbits;

	/// <summary>
	/// Use ROT_X/Y/Z to allow bones to be rotated with CreatureJoint().
	/// </summary>
	/// <param name="boneID">the mesh id - 1</param>
	/// <param name="flags">can be ROT_X, ROT_Y, ROT_Z or all.</param>
	void SetBoneRotationFlags(int boneID, int flags)
	{
		g_Level.Bones[boneIndex + boneID * 4] |= flags;
	}

	/// <summary>
	/// Use this to set up a hit rffect for the slot based on its value.
	/// </summary>
	/// <param name="isAlive">Use this if the object is alive but not intelligent to set up blood effects.</param>
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
void InitialiseHair();
void InitialiseObjects();
