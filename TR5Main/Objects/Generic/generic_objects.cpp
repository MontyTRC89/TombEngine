#include "framework.h"
#include "generic_objects.h"

/// objects
#include "generic_trapdoor.h"
#include "generic_bridge.h"

/// necessary import
#include "setup.h"

static void StartObject()
{
	auto obj = &Objects[ID_TRAPDOOR1];
	if (obj->loaded)
	{
		obj->initialise = InitialiseTrapDoor;
		obj->collision = TrapDoorCollision;
		obj->control = TrapDoorControl;
		obj->saveAnim = true;
		obj->saveFlags = true;
	}

	obj = &Objects[ID_TRAPDOOR2];
	if (obj->loaded)
	{
		obj->initialise = InitialiseTrapDoor;
		obj->collision = TrapDoorCollision;
		obj->control = TrapDoorControl;
		obj->saveAnim = true;
		obj->saveFlags = true;
	}

	obj = &Objects[ID_TRAPDOOR3];
	if (obj->loaded)
	{
		obj->initialise = InitialiseTrapDoor;
		obj->collision = TrapDoorCollision;
		obj->control = TrapDoorControl;
		obj->saveAnim = true;
		obj->saveFlags = true;
	}

	obj = &Objects[ID_FLOOR_TRAPDOOR1];
	if (obj->loaded)
	{
		obj->initialise = InitialiseTrapDoor;
		obj->collision = FloorTrapDoorCollision;
		obj->control = TrapDoorControl;
		obj->saveAnim = true;
		obj->saveFlags = true;
	}

	obj = &Objects[ID_FLOOR_TRAPDOOR2];
	if (obj->loaded)
	{
		obj->initialise = InitialiseTrapDoor;
		obj->collision = FloorTrapDoorCollision;
		obj->control = TrapDoorControl;
		obj->saveAnim = true;
		obj->saveFlags = true;
	}

	obj = &Objects[ID_CEILING_TRAPDOOR1];
	if (obj->loaded)
	{
		obj->initialise = InitialiseTrapDoor;
		obj->collision = CeilingTrapDoorCollision;
		obj->control = TrapDoorControl;
		obj->saveAnim = true;
		obj->saveFlags = true;
	}

	obj = &Objects[ID_CEILING_TRAPDOOR2];
	if (obj->loaded)
	{
		obj->initialise = InitialiseTrapDoor;
		obj->collision = CeilingTrapDoorCollision;
		obj->control = TrapDoorControl;
		obj->saveAnim = true;
		obj->saveFlags = true;
	}

	for (int objNumber = ID_BRIDGE1; objNumber <= ID_BRIDGE8; ++objNumber)
	{
		obj = &Objects[objNumber];
		if (obj->loaded)
		{
			obj->initialise = InitialiseBridge;
			obj->floor = BridgeFloor;
			obj->ceiling = BridgeCeiling;
		}
	}
}

void InitialiseGenericObjects()
{
	StartObject();
}
