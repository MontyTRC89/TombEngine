#include "framework.h"
#include "ObjectsUtils.h"
#include "collide.h"
#include "objects.h"
#include "pickup.h"
#include "level.h"
#include "tr5_smashobject.h"
#include "tr5_pushableblock.h"

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
		obj->saveFlags = true;
		obj->saveAnim = true;
		obj->saveMesh = true;
		Bones[obj->boneIndex + (0 * 4)] |= ROT_Y;
		Bones[obj->boneIndex + (1 * 4)] |= ROT_X;
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

void InitPickupItem(ObjectInfo* obj, function<void(short itemNumber)> func, int objectNumber, bool useDrawAnimItem)
{
	obj = &Objects[objectNumber];
	if (obj->loaded)
	{
		obj->collision = PickupCollision;
		obj->control = func;

		if (objectNumber == ID_FLARE_ITEM)
		{
			obj->pivotLength = 256;
			obj->hitPoints = 256; // time
		}

		obj->saveFlags = true;
		obj->savePosition = true;
		if (useDrawAnimItem)
			obj->usingDrawAnimatingItem = true;
		else
			obj->usingDrawAnimatingItem = false;
	}
}

void InitProjectile(ObjectInfo* obj, function<void(short itemNumber)> func, int objectNumber, bool noLoad)
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
		obj->saveFlags = true;
		obj->savePosition = true;
		obj->saveAnim = true;
	}
}
