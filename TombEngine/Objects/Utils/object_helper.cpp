#include "framework.h"

#include "Game/collision/collide_item.h"
#include "Game/Lara/lara_flare.h"
#include "Game/pickup/pickup.h"
#include "Objects/Utils/object_helper.h"
#include "Objects/Generic/Object/objects.h"
#include "Objects/Generic/puzzles_keys.h"
#include "Objects/TR5/Object/tr5_pushableblock.h"
#include "Specific/level.h"

using std::function;

void InitSmashObject(ObjectInfo* obj, int objectNumber)
{
	obj = &Objects[objectNumber];
	if (obj->loaded)
	{
		obj->initialise = InitialiseSmashObject;
		obj->collision = ObjectCollision;
		obj->control = SmashObjectControl;
	}
}

void InitKeyHole(ObjectInfo* obj, int objectNumber)
{
	obj = &Objects[objectNumber];
	if (obj->loaded)
	{
		obj->collision = KeyHoleCollision;
		obj->hitEffect = HIT_RICOCHET;
	}
}

void InitPuzzleHole(ObjectInfo* obj, int objectNumber)
{
	obj = &Objects[objectNumber];
	if (obj->loaded)
	{
		obj->collision = PuzzleHoleCollision;
		obj->control = AnimatingControl;
		obj->hitEffect = HIT_RICOCHET;
		obj->isPuzzleHole = true;
	}
}

void InitPuzzleDone(ObjectInfo* obj, int objectNumber)
{
	obj = &Objects[objectNumber];
	if (obj->loaded)
	{
		obj->collision = PuzzleDoneCollision;
		obj->control = AnimatingControl;
		obj->hitEffect = HIT_RICOCHET;
	}
}

void InitAnimating(ObjectInfo* obj, int objectNumber)
{
	obj = &Objects[objectNumber];
	if (obj->loaded)
	{
		obj->initialise = InitialiseAnimating;
		obj->control = AnimatingControl;
		obj->collision = ObjectCollision;
		obj->hitEffect = HIT_RICOCHET;
		//Bones[obj->boneIndex + (0 * 4)] |= ROT_Y;
		//Bones[obj->boneIndex + (1 * 4)] |= ROT_X;
	}
}

void InitPickup(ObjectInfo* obj, int objectNumber)
{
	obj = &Objects[objectNumber];
	if (obj->loaded)
	{
		obj->initialise = InitialisePickup;
		obj->collision = PickupCollision;
		obj->control = PickupControl;
		obj->isPickup = true;
	}
}

void InitFlare(ObjectInfo* obj, int objectNumber)
{
	obj = &Objects[objectNumber];
	if (obj->loaded)
	{
		obj->collision = PickupCollision;
		obj->control = FlareControl;
		obj->pivotLength = 256;
		obj->HitPoints = 256; // Time
		obj->usingDrawAnimatingItem = false;
		obj->isPickup = true;
	}
}

void InitProjectile(ObjectInfo* obj, function<InitFunction> func, int objectNumber, bool noLoad)
{
	obj = &Objects[objectNumber];
	if (obj->loaded || noLoad)
	{
		obj->initialise = nullptr;
		obj->collision = nullptr;
		obj->control = func;
	}
}

void InitSearchObject(ObjectInfo* obj, int objectNumber)
{
	obj = &Objects[objectNumber];
	if (obj->loaded)
	{
		obj->initialise = InitialiseSearchObject;
		obj->collision = SearchObjectCollision;
		obj->control = SearchObjectControl;
	}
}

void InitPushableObject(ObjectInfo* obj, int objectNumber)
{
	obj = &Objects[objectNumber];
	if (obj->loaded)
	{
		obj->initialise = InitialisePushableBlock;
		obj->control = PushableBlockControl;
		obj->collision = PushableBlockCollision;
		obj->floor = PushableBlockFloor;
		obj->ceiling = PushableBlockCeiling;
		obj->floorBorder = PushableBlockFloorBorder;
		obj->ceilingBorder = PushableBlockCeilingBorder;
		obj->hitEffect = HIT_RICOCHET;
	}
}
