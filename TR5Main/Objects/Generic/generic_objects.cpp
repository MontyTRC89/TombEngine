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
		obj->floorBorder = TrapDoorFloorBorder;
		obj->ceilingBorder = TrapDoorCeilingBorder;
		obj->floor = TrapDoorFloor;
		obj->ceiling = TrapDoorCeiling;
		obj->saveAnim = true;
		obj->saveFlags = true;
	}

	obj = &Objects[ID_TRAPDOOR2];
	if (obj->loaded)
	{
		obj->initialise = InitialiseTrapDoor;
		obj->collision = TrapDoorCollision;
		obj->control = TrapDoorControl;
		obj->floorBorder = TrapDoorFloorBorder;
		obj->ceilingBorder = TrapDoorCeilingBorder;
		obj->floor = TrapDoorFloor;
		obj->ceiling = TrapDoorCeiling;
		obj->saveAnim = true;
		obj->saveFlags = true;
	}

	obj = &Objects[ID_TRAPDOOR3];
	if (obj->loaded)
	{
		obj->initialise = InitialiseTrapDoor;
		obj->collision = TrapDoorCollision;
		obj->control = TrapDoorControl;
		obj->floorBorder = TrapDoorFloorBorder;
		obj->ceilingBorder = TrapDoorCeilingBorder;
		obj->floor = TrapDoorFloor;
		obj->ceiling = TrapDoorCeiling;
		obj->saveAnim = true;
		obj->saveFlags = true;
	}

	obj = &Objects[ID_FLOOR_TRAPDOOR1];
	if (obj->loaded)
	{
		obj->initialise = InitialiseTrapDoor;
		obj->collision = FloorTrapDoorCollision;
		obj->control = TrapDoorControl;
		obj->floorBorder = TrapDoorFloorBorder;
		obj->ceilingBorder = TrapDoorCeilingBorder;
		obj->floor = TrapDoorFloor;
		obj->ceiling = TrapDoorCeiling;
		obj->saveAnim = true;
		obj->saveFlags = true;
	}

	obj = &Objects[ID_FLOOR_TRAPDOOR2];
	if (obj->loaded)
	{
		obj->initialise = InitialiseTrapDoor;
		obj->collision = FloorTrapDoorCollision;
		obj->control = TrapDoorControl;
		obj->floorBorder = TrapDoorFloorBorder;
		obj->ceilingBorder = TrapDoorCeilingBorder;
		obj->floor = TrapDoorFloor;
		obj->ceiling = TrapDoorCeiling;
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

	obj = &Objects[ID_BRIDGE_FLAT];
	if (obj->loaded)
	{
		obj->initialise = InitialiseBridge;
		obj->floor = BridgeFloor<0>;
		obj->ceiling = BridgeCeiling<0>;
		obj->floorBorder = BridgeFloorBorder<0>;
		obj->ceilingBorder = BridgeCeilingBorder<0>;
	}

	obj = &Objects[ID_BRIDGE_TILT1];
	if (obj->loaded)
	{
		obj->initialise = InitialiseBridge;
		obj->floor = BridgeFloor<1>;
		obj->ceiling = BridgeCeiling<1>;
		obj->floorBorder = BridgeFloorBorder<1>;
		obj->ceilingBorder = BridgeCeilingBorder<1>;
	}

	obj = &Objects[ID_BRIDGE_TILT2];
	if (obj->loaded)
	{
		obj->initialise = InitialiseBridge;
		obj->floor = BridgeFloor<2>;
		obj->ceiling = BridgeCeiling<2>;
		obj->floorBorder = BridgeFloorBorder<2>;
		obj->ceilingBorder = BridgeCeilingBorder<2>;
	}

	obj = &Objects[ID_BRIDGE_TILT3];
	if (obj->loaded)
	{
		obj->initialise = InitialiseBridge;
		obj->floor = BridgeFloor<3>;
		obj->ceiling = BridgeCeiling<3>;
		obj->floorBorder = BridgeFloorBorder<3>;
		obj->ceilingBorder = BridgeCeilingBorder<3>;
	}

	obj = &Objects[ID_BRIDGE_TILT4];
	if (obj->loaded)
	{
		obj->initialise = InitialiseBridge;
		obj->floor = BridgeFloor<4>;
		obj->ceiling = BridgeCeiling<4>;
		obj->floorBorder = BridgeFloorBorder<4>;
		obj->ceilingBorder = BridgeCeilingBorder<4>;
	}
}

void InitialiseGenericObjects()
{
	StartObject();
}
