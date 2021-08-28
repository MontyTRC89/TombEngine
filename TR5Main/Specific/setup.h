#pragma once

#include "objectslist.h"
#include "phd_global.h"
struct ITEM_INFO;
struct COLL_INFO;
enum ZoneType : char;
enum HitEffectEnum
{
    HIT_NONE,
    HIT_BLOOD,
    HIT_SMOKE,
    HIT_RICOCHET,
	HIT_SPECIAL,
    MAX_HIT_EFFECT
};

struct OBJECT_INFO
{
	short nmeshes; 
	short meshIndex; 
	int boneIndex; 
	int frameBase;
	std::function<void(short itemNumber)> initialise;
	std::function<void(short itemNumber)> control;
	std::function<std::optional<int>(short itemNumber, int x, int y, int z)> floor;
	std::function<std::optional<int>(short itemNumber, int x, int y, int z)> ceiling;
	std::function<int(short itemNumber)> floorBorder;
	std::function<int(short itemNumber)> ceilingBorder;
	std::function<void(ITEM_INFO* item)> drawRoutine;
	std::function<void(ITEM_INFO* item)> drawRoutineExtra;
	std::function<void(short item_num, ITEM_INFO* laraitem, COLL_INFO* coll)> collision;
	ZoneType zoneType;
	short animIndex; 
	short hitPoints; 
	short pivotLength; 
	short radius; 
	short shadowSize; 
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
	bool castShadows;
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
	int shatterDamage;
	int shatterSound;
};

#define MAX_STATICS 1000
constexpr auto SF_NO_COLLISION = 0x01;
constexpr auto SF_SHATTERABLE = 0x02;
constexpr auto GRAVITY = 6;
constexpr auto SWAMP_GRAVITY = 2;
constexpr auto SHT_NONE = 0;
constexpr auto SHT_EXPLODE = 1;
constexpr auto SHT_FRAGMENT = 2;

extern OBJECT_INFO Objects[ID_NUMBER_OBJECTS];
extern STATIC_INFO StaticObjects[MAX_STATICS];

void InitialiseGameFlags();
void InitialiseSpecialEffects();
void InitialiseHair();
void InitialiseObjects();