#pragma once

#include "..\Global\global.h"

struct ObjectInfo {
	short nmeshes; 
	short meshIndex; 
	int boneIndex; 
	short* frameBase; 
	void(*initialise)(short itemNumber); 
	void(*control)(short itemNumber); 
	void(*floor)(ITEM_INFO* item, int x, int y, int z, int* height); 
	void(*ceiling)(ITEM_INFO* item, int x, int y, int z, int* height); 
	void(*drawRoutine)(ITEM_INFO* item);
	void(*drawRoutineExtra)(ITEM_INFO* item);
	void(*collision)(short item_num, ITEM_INFO* laraitem, COLL_INFO* coll); 
	short zoneType; 
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
	unsigned int explodableMeshbits;
	int meshSwapSlot;
};

extern ObjectInfo Objects[ID_NUMBER_OBJECTS];
extern STATIC_INFO StaticObjects[NUM_STATICS];

void BaddyObjects();
void ObjectObjects();
void TrapObjects();
void InitialiseHair();
void InitialiseSpecialEffects();
void InitialiseGameFlags();
void CustomObjects();
void InitialiseObjects();

#define INIT_PICKUP(obid) \
obj = &Objects[obid]; \
if (obj->loaded) \
{ \
	obj->initialise = InitialisePickup; \
	obj->collision = PickupCollision; \
	obj->control = PickupControl; \
}

#define INIT_KEYHOLE(obid) \
obj = &Objects[obid]; \
if (obj->loaded) \
{ \
	obj->collision = KeyHoleCollision; \
	obj->saveFlags = true; \
}

#define INIT_PUZZLEHOLE(obid) \
obj = &Objects[obid]; \
if (obj->loaded) \
{ \
	obj->collision = PuzzleHoleCollision; \
	obj->control = AnimatingControl; \
	obj->saveFlags = true; \
	obj->saveAnim = true; \
}

#define INIT_PUZZLEDONE(obid) \
obj = &Objects[obid]; \
if (obj->loaded) \
{ \
	obj->collision = PuzzleDoneCollision; \
	obj->control = AnimatingControl; \
	obj->saveFlags = true; \
	obj->saveAnim = true; \
}

#define INIT_ANIMATING(obid) \
obj = &Objects[obid]; \
if (obj->loaded) \
{ \
	obj->initialise = InitialiseAnimating; \
	obj->control = AnimatingControl; \
	obj->collision = ObjectCollision; \
	obj->saveFlags = true; \
	obj->saveAnim = true; \
	obj->saveMesh = true; \
	Bones[obj->boneIndex] |= ROT_Y; \
	Bones[obj->boneIndex + 4] |= ROT_X; \
}