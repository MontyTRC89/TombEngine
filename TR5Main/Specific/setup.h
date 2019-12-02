#pragma once

#include "..\Global\global.h"

void BaddyObjects(); //#define BaddyObjects ((void (__cdecl*)()) 0x004737C0)
void ObjectObjects(); //#define ObjectObjects ((void (__cdecl*)()) 0x00476360)
void TrapObjects(); //#define TrapObjects ((void (__cdecl*)()) 0x00475D40)
#define InitialiseHairs ((void (__cdecl*)()) 0x00438BE0)
#define InitialiseSpecialEffects ((void (__cdecl*)()) 0x0043D8B0)

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