#pragma once

#include "global.h"
#include "box.h"
#include "collide.h"

struct ObjectInfo
{
	short nmeshes; 
	short meshIndex; 
	int boneIndex; 
	short* frameBase; 
	function<void(short itemNumber)> initialise;
	function<void(short itemNumber)> control;
	function<void(ITEM_INFO* item, int x, int y, int z, int* height)> floor;
	function<void(ITEM_INFO* item, int x, int y, int z, int* height)> ceiling;
	function<void(ITEM_INFO* item)> drawRoutine;
	function<void(ITEM_INFO* item)> drawRoutineExtra;
	function<void(short item_num, ITEM_INFO* laraitem, COLL_INFO* coll)> collision;
	ZoneTypeEnum zoneType;
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
	bool hitEffect;
	bool undead;
	bool saveMesh;
	bool unknown;
	bool friendly;
	bool castShadows;
	bool isPickup;
	bool isPuzzleHole;
	unsigned int explodableMeshbits;
	int meshSwapSlot;
};

extern ObjectInfo Objects[ID_NUMBER_OBJECTS];
extern STATIC_INFO StaticObjects[NUM_STATICS];

void InitialiseGameFlags();
void BaddyObjects();
void ObjectObjects();
void TrapObjects();
void InitialiseSpecialEffects();
void InitialiseHair();
void InitialiseObjects();