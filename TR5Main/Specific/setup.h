#pragma once

#include "..\Global\global.h"

extern OBJECT_INFO Objects[ID_NUMBER_OBJECTS];
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