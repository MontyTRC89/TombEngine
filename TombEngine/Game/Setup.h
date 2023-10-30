#pragma once
#include "Game/control/box.h"
#include "Math/Math.h"
#include "Objects/objectslist.h"
#include "Renderer/Renderer11Enums.h"
#include "Specific/level.h"

struct CollisionInfo;
struct ItemInfo;

constexpr auto DEFAULT_RADIUS = 10;
constexpr auto GRAVITY		  = 6.0f;
constexpr auto SWAMP_GRAVITY  = GRAVITY / 3.0f;

constexpr auto MAX_STATICS = 1000;

enum JointRotationFlags
{
	ROT_X = (1 << 2),
	ROT_Y = (1 << 3),
	ROT_Z = (1 << 4)
};

// Unused.
enum ShatterFlags
{
	NoCollision = (1 << 0),
	Shatterable = (1 << 1)
};

// Custom LOT definition for Creature. Used in InitializeSlot() in lot.cpp.
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
	Blockable,	  // For large creatures such as trex and shiva.
	Spider,		  // Only 2 block vault allowed.
	Ape,		  // Only 2 block vault allowed.
	SnowmobileGun // Only 1 block vault allowed and 4 block drop max.
};

enum class HitEffect
{
    None,
    Blood,
    Smoke,
    Richochet,
	Special
};

enum class DamageMode
{
	None,
	Any,
	Explosion
};

enum class ShatterType
{
	None,
	Fragment,
	Explode
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
	DamageMode damageType;
	ShadowMode shadowType;

	int meshSwapSlot;
	int pivotLength;
	int radius;

	int HitPoints;
	bool intelligent;	// IsIntelligent
	bool waterCreature; // IsWaterCreature
	bool nonLot;		// IsNonLot
	bool isPickup;		// IsPickup
	bool isPuzzleHole;	// IsReceptacle
	bool usingDrawAnimatingItem;

	DWORD explodableMeshbits;

	std::function<void(short itemNumber)> Initialize;
	std::function<void(short itemNumber)> control;
	std::function<void(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)> collision;

	std::function<void(ItemInfo& target, ItemInfo& source, std::optional<GameVector> pos, int damage, bool isExplosive, int jointIndex)> HitRoutine;
	std::function<void(ItemInfo* item)> drawRoutine;

	std::function<std::optional<int>(int itemNumber, int x, int y, int z)> floor;
	std::function<std::optional<int>(int itemNumber, int x, int y, int z)> ceiling;
	std::function<int(short itemNumber)> floorBorder;
	std::function<int(short itemNumber)> ceilingBorder;

	// NOTE: ROT_X/Y/Z allows bones to be rotated with CreatureJoint().
	void SetBoneRotationFlags(int boneNumber, int flags)
	{
		g_Level.Bones[boneIndex + (boneNumber * 4)] |= flags;
	}

	// NOTE: Use if object is alive but not intelligent to set up blood effects.
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
			else if ((damageType != DamageMode::Any && HitPoints > 0) || HitPoints == NOT_TARGETABLE)
			{
				hitEffect = HitEffect::Smoke;
			}
			else if (damageType == DamageMode::Any && HitPoints > 0)
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

class ObjectHandler
{
	private:
		ObjectInfo Objects[ID_NUMBER_OBJECTS];

		ObjectInfo& GetFirstAvailableObject()
		{
			for (int i = 0; i < ID_NUMBER_OBJECTS; i++)
			{
				if (Objects[i].loaded)
					return Objects[i];
			}

			return Objects[0];
		}

	public:
		void Initialize() 
		{ 
			std::memset(Objects, 0, sizeof(ObjectInfo) * GAME_OBJECT_ID::ID_NUMBER_OBJECTS);
		}

		bool CheckID(int index, bool isSilent = false)
		{
			if (index == GAME_OBJECT_ID::ID_NO_OBJECT || index >= GAME_OBJECT_ID::ID_NUMBER_OBJECTS)
			{
				if (!isSilent)
				{
					TENLog("Attempted to access unavailable slot ID (" + std::to_string(index) + "). " +
						"Check if last accessed item exists in level.", LogLevel::Warning, LogConfig::Debug);
				}

				return false;
			}

			return true;
		}

		ObjectInfo& operator[](int index) 
		{
			if (CheckID(index))
				return Objects[index];
		
			return GetFirstAvailableObject();
		}
};

struct StaticInfo
{
	int meshNumber;
	int flags;
	GameBoundingBox visibilityBox;
	GameBoundingBox collisionBox;
	ShatterType shatterType;
	int shatterSound;
};

extern ObjectHandler Objects;
extern StaticInfo StaticObjects[MAX_STATICS];

void InitializeGameFlags();
void InitializeSpecialEffects();
void InitializeObjects();
