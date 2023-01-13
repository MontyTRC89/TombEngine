#include "framework.h"

#include "Game/collision/collide_item.h"
#include "Game/Lara/lara_flare.h"
#include "Game/pickup/pickup.h"
#include "Objects/Utils/object_helper.h"
#include "Objects/Generic/Object/objects.h"
#include "Objects/Generic/puzzles_keys.h"
#include "Objects/TR5/Object/tr5_pushableblock.h"
#include "Specific/level.h"


void AssignObjectMeshSwap(ObjectInfo* obj, int requiredMeshSwap, const std::string& baseName, const std::string& requiredName)
{
	if (Objects[requiredMeshSwap].loaded)
		obj->meshSwapSlot = requiredMeshSwap;
	else
		TENLog("The slot: " + requiredName + " is not loaded, " + baseName + " will have problem to swapmesh (could cause problem with entity not working correctly).", LogLevel::Warning);
}

void CheckIfSlotExists(int requiredObj, const std::string& baseName, const std::string& requiredName)
{
	if (!Objects[requiredObj].loaded)
		TENLog("The slot: " + requiredName + " is not loaded, " + baseName + " will have problem to work correctly (could cause crash).", LogLevel::Warning);
}

bool AssignObjectAnimations(ObjectInfo* obj, int requiredObj, const std::string& baseName, const std::string& requiredName)
{
	auto& anim = g_Level.Anims[obj->animIndex];
	// check if object have at last one animation with more than 1 frame.
	if (anim.frameEnd - anim.frameBase > 1)
		return true;

	// if not then try to refer to a specified slot.
	if (Objects[requiredObj].loaded) // if slot is loaded then use it !
	{
		obj->animIndex = Objects[requiredObj].animIndex;
		obj->frameBase = Objects[requiredObj].frameBase;
		return true;
	}
	else
	{
		TENLog("The slot: " + requiredName + " is not loaded, " + baseName + " will not have any animation.", LogLevel::Warning);
	}

	return false;
}

void InitSmashObject(ObjectInfo* obj, int objectNumber)
{
	obj = &Objects[objectNumber];
	if (obj->loaded)
	{
		obj->initialise = InitialiseSmashObject;
		obj->collision = ObjectCollision;
		obj->control = SmashObjectControl;
		obj->SetupHitEffect(true);
	}
}

void InitKeyHole(ObjectInfo* obj, int objectNumber)
{
	obj = &Objects[objectNumber];
	if (obj->loaded)
	{
		obj->collision = KeyHoleCollision;
		obj->SetupHitEffect(true);
	}
}

void InitPuzzleHole(ObjectInfo* obj, int objectNumber)
{
	obj = &Objects[objectNumber];
	if (obj->loaded)
	{
		obj->collision = PuzzleHoleCollision;
		obj->control = AnimatingControl;
		obj->isPuzzleHole = true;
		obj->SetupHitEffect(true);
	}
}

void InitPuzzleDone(ObjectInfo* obj, int objectNumber)
{
	obj = &Objects[objectNumber];
	if (obj->loaded)
	{
		obj->collision = PuzzleDoneCollision;
		obj->control = AnimatingControl;
		obj->SetupHitEffect(true);
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
		obj->SetupHitEffect(true);
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
		obj->SetupHitEffect(true);
	}
}

void InitPickup(ObjectInfo* obj, int objectNumber, std::function<ControlFunction> func)
{
	obj = &Objects[objectNumber];
	if (obj->loaded)
	{
		obj->initialise = InitialisePickup;

		obj->collision = PickupCollision;
		obj->control = (func != nullptr) ? func : PickupControl;
		obj->isPickup = true;
		obj->SetupHitEffect(true);
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
		obj->HitPoints = 256; // Time.
		obj->usingDrawAnimatingItem = false;
		obj->isPickup = true;
	}
}

void InitProjectile(ObjectInfo* obj, std::function<InitFunction> func, int objectNumber, bool noLoad)
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
		obj->SetupHitEffect(true);
	}
}
