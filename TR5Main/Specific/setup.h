#pragma once

#include "..\Global\global.h"

void BaddyObjects();
void ObjectObjects();
void TrapObjects();
void InitialiseHair();
void InitialiseSpecialEffects();

void CustomObjects();
void InitialiseObjects();

void Inject_Setup();

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
}

#define INIT_PUZZLEHOLE(obid) \
obj = &Objects[obid]; \
if (obj->loaded) \
{ \
	obj->collision = PuzzleHoleCollision; \
	obj->control = AnimatingControl; \
}

#define INIT_PUZZLEDONE(obid) \
obj = &Objects[obid]; \
if (obj->loaded) \
{ \
	obj->collision = PuzzleDoneCollision; \
	obj->control = AnimatingControl; \
}

// normally INIT_ANIMATING have:
//Bones[obj->boneIndex] |= ROT_Y;
//Bones[obj->boneIndex + 4] |= ROT_X;

#define INIT_ANIMATING(obid) \
obj = &Objects[obid]; \
if (obj->loaded) \
{ \
	obj->initialise = InitialiseAnimating; \
	obj->control = AnimatingControl; \
	obj->collision = ObjectCollision; \
}