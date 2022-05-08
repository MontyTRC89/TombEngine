#include "framework.h"
#include "Objects/Utils/object_helper.h"
#include "Game/collision/collide_item.h"
#include "Objects/Generic/Object/objects.h"
#include "Game/pickup/pickup.h"
#include "Objects/Generic/puzzles_keys.h"
#include "Specific/level.h"

#include "Objects/TR5/Object/tr5_pushableblock.h"

using std::function;

void InitSmashObject(ObjectInfo* obj, int objectNumber)
{
	obj = &Objects[objectNumber];
	if (obj->loaded)
	{
		obj->initialise = InitialiseSmashObject;
		obj->collision = ObjectCollision;
		obj->control = SmashObjectControl;
		obj->saveFlags = true;
		obj->saveAnim = true;
		obj->saveMesh = true;
	}
}

void InitKeyHole(ObjectInfo* obj, int objectNumber)
{
	obj = &Objects[objectNumber];
	if (obj->loaded)
	{
		obj->collision = KeyHoleCollision;
		obj->hitEffect = HIT_RICOCHET;
		obj->saveFlags = true;
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
		obj->saveFlags = true;
		obj->saveAnim = true;
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
		obj->saveFlags = true;
		obj->saveAnim = true;
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
		obj->saveFlags = true;
		obj->saveAnim = true;
		obj->saveMesh = true;
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
		obj->savePosition = true;
		obj->saveFlags = true;
		obj->isPickup = true;
	}
}

void InitPickupItem(ObjectInfo* obj, function<InitFunction> func, int objectNumber, bool useDrawAnimItem)
{
	obj = &Objects[objectNumber];
	if (obj->loaded)
	{
		obj->collision = PickupCollision;
		obj->control = func;

		if (objectNumber == ID_FLARE_ITEM)
		{
			obj->pivotLength = 256;
			obj->HitPoints = 256; // time
		}

		obj->saveFlags = true;
		obj->savePosition = true;
		if (useDrawAnimItem)
			obj->usingDrawAnimatingItem = true;
		else
			obj->usingDrawAnimatingItem = false;
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
		obj->savePosition = true;
		obj->saveFlags = true;
		obj->saveAnim = true;
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
		obj->saveFlags = true;
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
		obj->saveFlags = true;
		obj->savePosition = true;
		obj->saveAnim = true;
	}
}
