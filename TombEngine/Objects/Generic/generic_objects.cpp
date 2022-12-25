#include "framework.h"
#include "Objects/Generic/generic_objects.h"

#include "Game/pickup/pickup.h"
#include "Game/collision/collide_item.h"

// Objects
#include "Objects/Generic/Object/generic_trapdoor.h"
#include "Objects/Generic/Object/generic_bridge.h"
#include "Objects/Generic/Object/burning_torch.h"
#include "Objects/Generic/Object/polerope.h"
#include "Objects/Generic/Object/rope.h"

// Switches
#include "Objects/Generic/Switches/cog_switch.h"
#include "Objects/Generic/Switches/rail_switch.h"
#include "Objects/Generic/Switches/jump_switch.h"
#include "Objects/Generic/Switches/generic_switch.h"
#include "Objects/Generic/Switches/crowbar_switch.h"
#include "Objects/Generic/Switches/underwater_switch.h"
#include "Objects/Generic/Switches/pulley_switch.h"
#include "Objects/Generic/Switches/fullblock_switch.h"
#include "Objects/Generic/Switches/turn_switch.h"

// Doors
#include "Objects/Generic/Doors/generic_doors.h"
#include "Objects/Generic/Doors/double_doors.h"
#include "Objects/Generic/Doors/pushpull_kick_door.h"
#include "Objects/Generic/Doors/sequence_door.h"
#include "Objects/Generic/Doors/steel_door.h"
#include "Objects/Generic/Doors/underwater_door.h"

// Traps
#include "Objects/Generic/Traps/dart_emitter.h"

/// Necessary import
#include "Specific/setup.h"

using namespace TEN::Entities::Switches;
using namespace TEN::Entities::Doors;
using namespace TEN::Entities::Traps;
using namespace TEN::Entities::Generic;

static void StartObject(ObjectInfo* obj)
{
	for (int objectNum = ID_TRAPDOOR1; objectNum <= ID_CEILING_TRAPDOOR2; objectNum++)
	{
		obj = &Objects[objectNum];
		if (obj->loaded)
		{
			obj->initialise = InitialiseTrapDoor;
			obj->collision = TrapDoorCollision;
			obj->control = TrapDoorControl;
			obj->floorBorder = TrapDoorFloorBorder;
			obj->ceilingBorder = TrapDoorCeilingBorder;
			obj->floor = TrapDoorFloor;
			obj->ceiling = TrapDoorCeiling;
			obj->isSolid = true;
			obj->SetupHitEffect();
		}
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

void StartSwitches(ObjectInfo* obj)
{
	obj = &Objects[ID_COG_SWITCH];
	if (obj->loaded)
	{
		obj->collision = CogSwitchCollision;
		obj->control = CogSwitchControl;
		obj->isSolid = true;
		obj->SetupHitEffect();
	}

	obj = &Objects[ID_LEVER_SWITCH];
	if (obj->loaded)
	{
		obj->collision = RailSwitchCollision;
		obj->control = SwitchControl;
		obj->isSolid = true;
		obj->SetupHitEffect();
	}

	obj = &Objects[ID_JUMP_SWITCH];
	if (obj->loaded)
	{
		obj->collision = JumpSwitchCollision;
		obj->control = SwitchControl;
		obj->isSolid = true;
		obj->SetupHitEffect();
	}

	for (int objectNum = ID_SWITCH_TYPE1; objectNum <= ID_SWITCH_TYPE16; objectNum++)
	{
		obj = &Objects[objectNum];
		if (obj->loaded)
		{
			obj->collision = SwitchCollision;
			obj->control = SwitchControl;
			obj->isSolid = true;
			obj->SetupHitEffect();
		}
	}

	obj = &Objects[ID_CROWBAR_SWITCH];
	if (obj->loaded)
	{
		obj->collision = CrowbarSwitchCollision;
		obj->control = SwitchControl;
		obj->isSolid = true;
		obj->SetupHitEffect();
	}

	for (int objectNum = ID_UNDERWATER_SWITCH1; objectNum <= ID_UNDERWATER_SWITCH4; objectNum++)
	{
		obj = &Objects[objectNum];
		if (obj->loaded)
		{
			obj->control = SwitchControl;
			obj->collision = UnderwaterSwitchCollision;
			obj->isSolid = true;
		}
	}

	obj = &Objects[ID_PULLEY];
	if (obj->loaded)
	{
		obj->initialise = InitialisePulleySwitch;
		obj->control = SwitchControl;
		obj->collision = PulleySwitchCollision;
		obj->isSolid = true;
	}

	obj = &Objects[ID_TURN_SWITCH];
	if (obj->loaded)
	{
		obj->control = TurnSwitchControl;
		obj->collision = TurnSwitchCollision;
		obj->isSolid = true;
		obj->SetupHitEffect();
	}

	obj = &Objects[ID_SEQUENCE_SWITCH1];
	if (obj->loaded)
	{
		obj->collision = FullBlockSwitchCollision;
		obj->control = FullBlockSwitchControl;
		obj->isSolid = true;
		obj->SetupHitEffect();
	}

	obj = &Objects[ID_SEQUENCE_SWITCH2];
	if (obj->loaded)
	{
		obj->collision = FullBlockSwitchCollision;
		obj->control = FullBlockSwitchControl;
		obj->isSolid = true;
		obj->SetupHitEffect();
	}

	obj = &Objects[ID_SEQUENCE_SWITCH3];
	if (obj->loaded)
	{
		obj->collision = FullBlockSwitchCollision;
		obj->control = FullBlockSwitchControl;
		obj->isSolid = true;
		obj->SetupHitEffect();
	}
}

void StartDoors(ObjectInfo* obj)
{
	for (int objectNumber = ID_DOOR_TYPE1; objectNumber <= ID_DOOR_TYPE30; objectNumber++)
	{
		obj = &Objects[objectNumber];
		if (obj->loaded)
		{
			obj->initialise = InitialiseDoor;
			obj->control = DoorControl;
			obj->collision = DoorCollision;
			obj->isSolid = true;
			obj->SetupHitEffect();
		}
	}

	obj = &Objects[ID_LIFT_DOORS1];
	if (obj->loaded)
	{
		obj->initialise = InitialiseDoor;
		obj->control = DoorControl;
		obj->isSolid = true;
		obj->SetupHitEffect();
	}

	obj = &Objects[ID_LIFT_DOORS2];
	if (obj->loaded)
	{
		obj->initialise = InitialiseDoor;
		obj->control = DoorControl;
		obj->isSolid = true;
		obj->SetupHitEffect();
	}

	obj = &Objects[ID_SEQUENCE_DOOR1];
	if (obj->loaded)
	{
		obj->initialise = InitialiseDoor;
		obj->collision = DoorCollision;
		obj->control = SequenceDoorControl;
		obj->isSolid = true;
		obj->SetupHitEffect();
	}

	for (int i = ID_DOUBLE_DOORS1; i <= ID_DOUBLE_DOORS4; i++)
	{
		obj = &Objects[i];
		if (obj->loaded)
		{
			obj->initialise = InitialiseDoor;
			obj->collision = DoubleDoorCollision;
			obj->control = PushPullKickDoorControl;
			obj->isSolid = true;
			obj->SetupHitEffect();
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
			obj->isSolid = true;
			obj->SetupHitEffect();
		}
	}

	for (int objectNum = ID_PUSHPULL_DOOR1; objectNum <= ID_KICK_DOOR4; objectNum++)
	{
		obj = &Objects[objectNum];
		if (obj->loaded)
		{
			obj->initialise = InitialiseDoor;
			obj->collision = PushPullKickDoorCollision;
			obj->control = PushPullKickDoorControl;
			obj->isSolid = true;
			obj->SetupHitEffect();
		}
	}

	obj = &Objects[ID_STEEL_DOOR];
	if (obj->loaded)
	{
		obj->initialise = InitialiseSteelDoor;
		obj->collision = SteelDoorCollision;
		obj->isSolid = true;
		obj->SetupHitEffect();
	}
}

void StartTraps(ObjectInfo* obj)
{
	obj = &Objects[ID_DARTS];
	if (obj->loaded)
	{
		obj->collision = ObjectCollision;
		obj->control = DartControl;
		obj->shadowType = ShadowMode::All;
	}

	obj = &Objects[ID_DART_EMITTER];
	if (obj->loaded)
	{
		obj->control = DartEmitterControl;
		obj->drawRoutine = nullptr;
		obj->usingDrawAnimatingItem = false;
	}

	obj = &Objects[ID_HOMING_DART_EMITTER];
	if (obj->loaded)
	{
		obj->control = DartEmitterControl;
		obj->drawRoutine = nullptr;
		obj->usingDrawAnimatingItem = false;
	}

	obj = &Objects[ID_ROPE];
	if (obj->loaded)
	{
		obj->initialise = InitialiseRope;
		obj->control = RopeControl;
		obj->collision = RopeCollision;
		obj->drawRoutine = nullptr;
		obj->usingDrawAnimatingItem = false;
	}

	obj = &Objects[ID_POLEROPE];
	if (obj->loaded)
	{
		obj->collision = PoleCollision;
		obj->isSolid = true;
		obj->SetupHitEffect();
	}

	obj = &Objects[ID_BURNING_TORCH_ITEM];
	if (obj->loaded)
	{
		obj->control = TorchControl;
		obj->collision = PickupCollision;
		obj->usingDrawAnimatingItem = true;
		obj->isPickup = true;
	}
}

void InitialiseGenericObjects()
{
	ObjectInfo* objToInit = nullptr;
	StartTraps(objToInit);
	StartObject(objToInit);
	StartSwitches(objToInit);
	StartDoors(objToInit);
}
