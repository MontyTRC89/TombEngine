#include "framework.h"
#include "object_helper.h"
#include "collide.h"
#include "objects.h"
#include "pickup.h"
#include "puzzles_keys.h"
#include "level.h"
#include "tr5_smashobject.h"
#include "tr5_pushableblock.h"
using std::function;
void InitSmashObject(OBJECT_INFO* obj, int objectNumber)
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

void InitKeyHole(OBJECT_INFO* obj, int objectNumber)
{
	obj = &Objects[objectNumber];
	if (obj->loaded)
	{
		obj->collision = KeyHoleCollision;
		obj->saveFlags = true;
	}
}

void InitPuzzleHole(OBJECT_INFO* obj, int objectNumber)
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

void InitPuzzleDone(OBJECT_INFO* obj, int objectNumber)
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

void InitAnimating(OBJECT_INFO* obj, int objectNumber)
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
		//Bones[obj->boneIndex + (0 * 4)] |= ROT_Y;
		//Bones[obj->boneIndex + (1 * 4)] |= ROT_X;
	}
}

void InitPickup(OBJECT_INFO* obj, int objectNumber)
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

void InitPickupItem(OBJECT_INFO* obj, function<InitFunction> func, int objectNumber, bool useDrawAnimItem)
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

void InitProjectile(OBJECT_INFO* obj, function<InitFunction> func, int objectNumber, bool noLoad)
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

void InitSearchObject(OBJECT_INFO* obj, int objectNumber)
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

void InitPushableObject(OBJECT_INFO* obj, int objectNumber)
{
	obj = &Objects[objectNumber];
	if (obj->loaded)
	{
		obj->initialise = InitialisePushableBlock;
		obj->control = PushableBlockControl;
		obj->collision = PushableBlockCollision;
		obj->floor = PushableBlockFloor;
		obj->ceiling = PushableBlockCeiling;
		obj->saveFlags = true;
		obj->savePosition = true;
		obj->saveAnim = true;
	}
}
