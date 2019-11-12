#pragma once

#include "..\Global\global.h"

void __cdecl BaddyObjects(); //#define BaddyObjects ((void (__cdecl*)()) 0x004737C0)
void __cdecl ObjectObjects(); //#define ObjectObjects ((void (__cdecl*)()) 0x00476360)
void __cdecl TrapObjects(); //#define TrapObjects ((void (__cdecl*)()) 0x00475D40)
#define InitialiseHairs ((void (__cdecl*)()) 0x00438BE0)
#define InitialiseSpecialEffects ((void (__cdecl*)()) 0x0043D8B0)

void __cdecl CustomObjects();
void __cdecl InitialiseObjects();

void Inject_Setup();

#define INIT_PICKUP(obj_id) \
obj = &Objects[obj_id]; \
if (obj->loaded) \
{ \
	obj->initialise = InitialisePickup; \
	obj->collision = PickupCollision; \
	obj->control = PickupControl; \
}

#define INIT_KEYHOLE(obj_id) \
obj = &Objects[obj_id]; \
if (obj->loaded) \
{ \
	obj->collision = KeyHoleCollision; \
}

#define INIT_PUZZLEHOLE(obj_id) \
obj = &Objects[obj_id]; \
if (obj->loaded) \
{ \
	obj->collision = PuzzleHoleCollision; \
	obj->control = AnimatingControl; \
}

#define INIT_PUZZLEDONE(obj_id) \
obj = &Objects[obj_id]; \
if (obj->loaded) \
{ \
	obj->collision = PuzzleDoneCollision; \
	obj->control = AnimatingControl; \
}

// normally INIT_ANIMATING have:
//Bones[obj->boneIndex] |= ROT_Y;
//Bones[obj->boneIndex + 4] |= ROT_X;

#define INIT_ANIMATING(obj_id) \
obj = &Objects[obj_id]; \
if (obj->loaded) \
{ \
	obj->initialise = InitialiseAnimating; \
	obj->control = AnimatingControl; \
	obj->collision = ObjectCollision; \
}