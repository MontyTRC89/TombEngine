#pragma once
#include "Objects/objectslist.h"
#include "Specific/phd_global.h"
#include "Renderer/Renderer11Enums.h"

struct ItemInfo;
struct CollisionInfo;
enum ZoneType : char;

constexpr auto DEFAULT_RADIUS = 10;
constexpr auto ROT_X = 0x0004;
constexpr auto ROT_Y = 0x0008;
constexpr auto ROT_Z = 0x0010;

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
	ZoneType zoneType;
	int animIndex; 
	short HitPoints; 
	short pivotLength; 
	short radius; 
	ShadowMode shadowType;
	short biteOffset; 
	bool loaded;
	bool intelligent;
	bool nonLot;
	bool savePosition;
	bool saveHitpoints;
	bool saveFlags;
	bool saveAnim;
	bool semiTransparent;
	bool waterCreature;
	bool usingDrawAnimatingItem;
	HitEffectEnum hitEffect;
	bool undead;
	bool saveMesh;
	bool friendly;
	bool isPickup;
	bool isPuzzleHole;
	int meshSwapSlot;
	DWORD explodableMeshbits;
};

struct STATIC_INFO
{
	int meshNumber;
	int flags;
	BOUNDING_BOX visibilityBox;
	BOUNDING_BOX collisionBox;
	int shatterType;
	int shatterSound;
};

#define MAX_STATICS 1000
constexpr auto SF_NO_COLLISION = 0x01;
constexpr auto SF_SHATTERABLE = 0x02;
constexpr auto GRAVITY = 6;
constexpr auto SWAMP_GRAVITY = GRAVITY / 3;

extern ObjectInfo Objects[ID_NUMBER_OBJECTS];
extern STATIC_INFO StaticObjects[MAX_STATICS];

void InitialiseGameFlags();
void InitialiseSpecialEffects();
void InitialiseHair();
void InitialiseObjects();
