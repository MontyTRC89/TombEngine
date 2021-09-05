#include "framework.h"
#include "generic_objects.h"

// objects
#include "generic_trapdoor.h"
#include "generic_bridge.h"

// switches
#include "cog_switch.h"
#include "rail_switch.h"
#include "jump_switch.h"
#include "generic_switch.h"
#include "crowbar_switch.h"
#include "underwater_switch.h"
#include "pulley_switch.h"
#include "fullblock_switch.h"

// Doors
#include "generic_doors.h"
#include "double_doors.h"
#include "pushpull_kick_door.h"
#include "sequence_door.h"
#include "steel_door.h"
#include "underwater_door.h"

/// necessary import
#include "setup.h"

using namespace TEN::Entities::Switches;
using namespace TEN::Entities::Doors;

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
		obj->floorBorder = TrapDoorFloorBorder;
		obj->ceilingBorder = TrapDoorCeilingBorder;
		obj->floor = TrapDoorFloor;
		obj->ceiling = TrapDoorCeiling;
		obj->saveAnim = true;
		obj->saveFlags = true;
	}

	obj = &Objects[ID_CEILING_TRAPDOOR2];
	if (obj->loaded)
	{
		obj->initialise = InitialiseTrapDoor;
		obj->collision = CeilingTrapDoorCollision;
		obj->control = TrapDoorControl;
		obj->floorBorder = TrapDoorFloorBorder;
		obj->ceilingBorder = TrapDoorCeilingBorder;
		obj->floor = TrapDoorFloor;
		obj->ceiling = TrapDoorCeiling;
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

void StartSwitches()
{
	OBJECT_INFO* obj;

	obj = &Objects[ID_COG_SWITCH];
	if (obj->loaded)
	{
		obj->collision = CogSwitchCollision;
		obj->control = CogSwitchControl;
		obj->hitEffect = HIT_RICOCHET;
		obj->saveFlags = true;
		obj->saveAnim = true;
	}

	obj = &Objects[ID_LEVER_SWITCH];
	if (obj->loaded)
	{
		obj->collision = RailSwitchCollision;
		obj->control = SwitchControl;
		obj->saveFlags = true;
		obj->saveAnim = true;
	}

	obj = &Objects[ID_JUMP_SWITCH];
	if (obj->loaded)
	{
		obj->collision = JumpSwitchCollision;
		obj->control = SwitchControl;
		obj->saveFlags = true;
		obj->saveAnim = true;
	}

	for (int objNum = ID_SWITCH_TYPE1; objNum <= ID_SWITCH_TYPE16; objNum++)
	{
		obj = &Objects[objNum];
		if (obj->loaded)
		{
			obj->collision = SwitchCollision;
			obj->control = SwitchControl;
			obj->saveFlags = true;
			obj->saveAnim = true;
			obj->saveMesh = true;
		}
	}

	obj = &Objects[ID_CROWBAR_SWITCH];
	if (obj->loaded)
	{
		obj->collision = CrowbarSwitchCollision;
		obj->control = SwitchControl;
		obj->saveFlags = true;
		obj->saveAnim = true;
	}

	for (int objNum = ID_UNDERWATER_SWITCH1; objNum <= ID_UNDERWATER_SWITCH4; objNum++)
	{
		obj = &Objects[objNum];
		if (obj->loaded)
		{
			obj->control = SwitchControl;
			obj->collision = objNum < ID_UNDERWATER_SWITCH3 ? UnderwaterSwitchCollision : CeilingUnderwaterSwitchCollision;
			obj->saveFlags = true;
			obj->saveAnim = true;
		}
	}

	obj = &Objects[ID_PULLEY];
	if (obj->loaded)
	{
		obj->initialise = InitialisePulleySwitch;
		obj->control = SwitchControl;
		obj->collision = PulleySwitchCollision;
		obj->saveFlags = true;
		obj->saveAnim = true;
	}

	obj = &Objects[ID_SEQUENCE_SWITCH1];
	if (obj->loaded)
	{
		obj->collision = FullBlockSwitchCollision;
		obj->control = FullBlockSwitchControl;
		obj->saveFlags = true;
		obj->saveAnim = true;
	}

	obj = &Objects[ID_SEQUENCE_SWITCH2];
	if (obj->loaded)
	{
		obj->collision = FullBlockSwitchCollision;
		obj->control = FullBlockSwitchControl;
		obj->saveFlags = true;
		obj->saveAnim = true;
	}

	obj = &Objects[ID_SEQUENCE_SWITCH3];
	if (obj->loaded)
	{
		obj->collision = FullBlockSwitchCollision;
		obj->control = FullBlockSwitchControl;
		obj->saveFlags = true;
		obj->saveAnim = true;
	}
}

void StartDoors()
{
	OBJECT_INFO* obj;

	for (int objNum = ID_DOOR_TYPE1; objNum <= ID_CLOSED_DOOR6; objNum++)
	{
		obj = &Objects[objNum];
		if (obj->loaded)
		{
			obj->initialise = InitialiseDoor;
			obj->control = DoorControl;
			obj->collision = DoorCollision;
			obj->hitEffect = HIT_RICOCHET;
			obj->saveAnim = true;
			obj->saveFlags = true;
			obj->saveMesh = true;
		}
	}

	obj = &Objects[ID_LIFT_DOORS1];
	if (obj->loaded)
	{
		obj->initialise = InitialiseDoor;
		obj->control = DoorControl;
		obj->hitEffect = HIT_RICOCHET;
		obj->saveFlags = true;
	}

	obj = &Objects[ID_LIFT_DOORS2];
	if (obj->loaded)
	{
		obj->initialise = InitialiseDoor;
		obj->control = DoorControl;
		obj->hitEffect = HIT_RICOCHET;
		obj->saveFlags = true;
	}

	obj = &Objects[ID_SEQUENCE_DOOR1];
	if (obj->loaded)
	{
		obj->initialise = InitialiseDoor;
		obj->collision = DoorCollision;
		obj->control = SequenceDoorControl;
		obj->hitEffect = HIT_RICOCHET;
		obj->saveAnim = true;
		obj->saveFlags = true;
	}

	for (int i = ID_DOUBLE_DOORS1; i <= ID_DOUBLE_DOORS4; i++)
	{
		obj = &Objects[i];
		if (obj->loaded)
		{
			obj->initialise = InitialiseDoor;
			obj->collision = DoubleDoorCollision;
			obj->control = PushPullKickDoorControl;
			obj->hitEffect = HIT_RICOCHET;
			obj->saveAnim = true;
			obj->saveFlags = true;
		}
	}

	for (int i = ID_UNDERWATER_DOOR1; i <= ID_UNDERWATER_DOOR4; i++)
	{
		obj = &Objects[i];
		if (obj->loaded)
		{
			obj->initialise = InitialiseDoor;
			obj->collision = UnderwaterDoorCollision;
			obj->control = PushPullKickDoorControl;
			obj->hitEffect = HIT_RICOCHET;
			obj->saveAnim = true;
			obj->saveFlags = true;
		}
	}

	for (int objNum = ID_PUSHPULL_DOOR1; objNum <= ID_KICK_DOOR4; objNum++)
	{
		obj = &Objects[objNum];
		if (obj->loaded)
		{
			obj->initialise = InitialiseDoor;
			obj->collision = PushPullKickDoorCollision;
			obj->control = PushPullKickDoorControl;
			obj->hitEffect = HIT_RICOCHET;
			obj->saveAnim = true;
			obj->saveFlags = true;
		}
	}

	obj = &Objects[ID_STEEL_DOOR];
	if (obj->loaded)
	{
		obj->initialise = InitialiseSteelDoor;
		obj->collision = SteelDoorCollision;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->saveMesh = true;
		obj->savePosition = true;
	}
}

void InitialiseGenericObjects()
{
	StartObject();
	StartSwitches();
	StartDoors();
}
