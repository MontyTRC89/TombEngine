#pragma once
#include "box.h"
#include "collide.h"
#include "objectslist.h"

typedef enum HitEffectEnum
{
    HIT_NONE,
    HIT_BLOOD,
    HIT_SMOKE,
    HIT_FRAGMENT,
    MAX_HIT_EFFECT
};

typedef struct OBJECT_INFO
{
	short nmeshes; 
	short meshIndex; 
	int boneIndex; 
	short* frameBase; 
	std::function<void(short itemNumber)> initialise;
	std::function<void(short itemNumber)> control;
	std::function<void(ITEM_INFO* item, int x, int y, int z, int* height)> floor;
	std::function<void(ITEM_INFO* item, int x, int y, int z, int* height)> ceiling;
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

typedef struct StaticInfo
{
	short meshNumber;
	short flags;
	short xMinp;
	short xMaxp;
	short yMinp;
	short yMaxp;
	short zMinp;
	short zMaxp;
	short xMinc;
	short xMaxc;
	short yMinc;
	short yMaxc;
	short zMinc;
	short zMaxc;
};

#define MAX_STATICS 1000
constexpr auto GRAVITY = 6;
constexpr auto SWAMP_GRAVITY = 2;

extern OBJECT_INFO Objects[ID_NUMBER_OBJECTS];
extern StaticInfo StaticObjects[MAX_STATICS];

void InitialiseGameFlags();
void InitialiseSpecialEffects();
void InitialiseHair();
void InitialiseObjects();