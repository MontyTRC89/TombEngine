#include "framework.h"
#include "Objects/Utils/object_helper.h"

#include "Game/collision/collide_item.h"
#include "Game/Lara/lara_flare.h"
#include "Game/pickup/pickup.h"
#include "Objects/Generic/Object/objects.h"
#include "Objects/Generic/puzzles_keys.h"
#include "Objects/TR5/Object/tr5_pushableblock.h"
#include "Specific/level.h"

void AssignObjectMeshSwap(ObjectInfo& object, int requiredMeshSwap, const std::string& baseName, const std::string& requiredName)
{
	if (Objects[requiredMeshSwap].loaded)
		object.meshSwapSlot = requiredMeshSwap;
	else
		TENLog("Slot " + requiredName + " not loaded. Meshswap issues with " + baseName + " may result in incorrect behaviour.", LogLevel::Warning);
}

bool AssignObjectAnimations(ObjectInfo& object, int requiredObject, const std::string& baseName, const std::string& requiredName)
{
	// Check if the object has at least 1 animation with more than 1 frame.
	const auto& anim = g_Level.Anims[object.animIndex];
	if ((anim.frameEnd - anim.frameBase) > 1)
		return true;

	// Use slot if loaded.
	if (Objects[requiredObject].loaded)
	{
		object.animIndex = Objects[requiredObject].animIndex;
		object.frameBase = Objects[requiredObject].frameBase;
		return true;
	}
	else
	{
		TENLog("Slot " + requiredName + " not loaded. " + baseName + " will have no animations.", LogLevel::Warning);
	}

	return false;
}

void CheckIfSlotExists(int requiredObject, const std::string& baseName, const std::string& requiredName)
{
	if (!Objects[requiredObject].loaded)
		TENLog("Slot " + requiredName + " not loaded. " + baseName + " may work incorrectly or crash.", LogLevel::Warning);
}

void InitSmashObject(ObjectInfo* object, int objectNumber)
{
	object = &Objects[objectNumber];
	if (object->loaded)
	{
		object->initialise = InitialiseSmashObject;
		object->collision = ObjectCollision;
		object->control = SmashObjectControl;
		object->SetupHitEffect(true);
	}
}

void InitKeyHole(ObjectInfo* object, int objectNumber)
{
	object = &Objects[objectNumber];
	if (object->loaded)
	{
		object->collision = KeyHoleCollision;
		object->SetupHitEffect(true);
	}
}

void InitPuzzleHole(ObjectInfo* object, int objectNumber)
{
	object = &Objects[objectNumber];
	if (object->loaded)
	{
		object->collision = PuzzleHoleCollision;
		object->control = AnimatingControl;
		object->isPuzzleHole = true;
		object->SetupHitEffect(true);
	}
}

void InitPuzzleDone(ObjectInfo* object, int objectNumber)
{
	object = &Objects[objectNumber];
	if (object->loaded)
	{
		object->collision = PuzzleDoneCollision;
		object->control = AnimatingControl;
		object->SetupHitEffect(true);
	}
}

void InitAnimating(ObjectInfo* object, int objectNumber)
{
	object = &Objects[objectNumber];
	if (object->loaded)
	{
		object->initialise = InitialiseAnimating;
		object->control = AnimatingControl;
		object->collision = ObjectCollision;
		object->SetupHitEffect(true);
	}
}

void InitPickup(ObjectInfo* object, int objectNumber)
{
	object = &Objects[objectNumber];
	if (object->loaded)
	{
		object->initialise = InitialisePickup;
		object->collision = PickupCollision;
		object->control = PickupControl;
		object->isPickup = true;
		object->SetupHitEffect(true);
	}
}

void InitPickup(ObjectInfo* object, int objectNumber, std::function<ControlFunction> func)
{
	object = &Objects[objectNumber];
	if (object->loaded)
	{
		object->initialise = InitialisePickup;

		object->collision = PickupCollision;
		object->control = (func != nullptr) ? func : PickupControl;
		object->isPickup = true;
		object->SetupHitEffect(true);
	}
}

void InitFlare(ObjectInfo* object, int objectNumber)
{
	object = &Objects[objectNumber];
	if (object->loaded)
	{
		object->collision = PickupCollision;
		object->control = FlareControl;
		object->pivotLength = 256;
		object->HitPoints = 256; // Time.
		object->usingDrawAnimatingItem = false;
		object->isPickup = true;
	}
}

void InitProjectile(ObjectInfo* object, std::function<InitFunction> func, int objectNumber, bool noLoad)
{
	object = &Objects[objectNumber];
	if (object->loaded || noLoad)
	{
		object->initialise = nullptr;
		object->collision = nullptr;
		object->control = func;
	}
}

void InitSearchObject(ObjectInfo* object, int objectNumber)
{
	object = &Objects[objectNumber];
	if (object->loaded)
	{
		object->initialise = InitialiseSearchObject;
		object->collision = SearchObjectCollision;
		object->control = SearchObjectControl;
	}
}

void InitPushableObject(ObjectInfo* object, int objectNumber)
{
	object = &Objects[objectNumber];
	if (object->loaded)
	{
		object->initialise = InitialisePushableBlock;
		object->control = PushableBlockControl;
		object->collision = PushableBlockCollision;
		object->floor = PushableBlockFloor;
		object->ceiling = PushableBlockCeiling;
		object->floorBorder = PushableBlockFloorBorder;
		object->ceilingBorder = PushableBlockCeilingBorder;
		object->SetupHitEffect(true);
	}
}
