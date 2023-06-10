#include "framework.h"
#include "Objects/Generic/generic_objects.h"
#include "Objects/Generic/Object/objects.h"
#include "Objects/Utils/object_helper.h"

// Necessary import
#include "Game/collision/collide_item.h"
#include "Game/effects/effects.h"
#include "Game/pickup/pickup.h"
#include "Game/Setup.h"

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
#include "Objects/Generic/Switches/AirlockSwitch.h"
#include "Objects/Generic/Switches/switch.h"

// Doors
#include "Objects/Generic/Doors/generic_doors.h"
#include "Objects/Generic/Doors/double_doors.h"
#include "Objects/Generic/Doors/pushpull_kick_door.h"
#include "Objects/Generic/Doors/sequence_door.h"
#include "Objects/Generic/Doors/steel_door.h"
#include "Objects/Generic/Doors/underwater_door.h"

// Traps
#include "Objects/Generic/Traps/dart_emitter.h"
#include "Objects/Generic/Traps/falling_block.h"

using namespace TEN::Entities::Doors;
using namespace TEN::Entities::Generic;
using namespace TEN::Entities::Switches;
using namespace TEN::Entities::Traps;

static void StartObject(ObjectInfo* object)
{
	for (int objectNumber = ID_TRAPDOOR1; objectNumber <= ID_TRAPDOOR3; objectNumber++)
	{
		object = &Objects[objectNumber];
		if (object->loaded)
		{
			object->Initialize = InitializeTrapDoor;
			object->collision = TrapDoorCollision;
			object->control = TrapDoorControl;
			object->floorBorder = TrapDoorFloorBorder;
			object->ceilingBorder = TrapDoorCeilingBorder;
			object->floor = TrapDoorFloor;
			object->ceiling = TrapDoorCeiling;
			object->SetupHitEffect(true);
		}
	}

	for (int objectNumber = ID_FLOOR_TRAPDOOR1; objectNumber <= ID_FLOOR_TRAPDOOR2; objectNumber++)
	{
		object = &Objects[objectNumber];
		if (object->loaded)
		{
			object->Initialize = InitializeTrapDoor;
			object->collision = FloorTrapDoorCollision;
			object->control = TrapDoorControl;
			object->floorBorder = TrapDoorFloorBorder;
			object->ceilingBorder = TrapDoorCeilingBorder;
			object->floor = TrapDoorFloor;
			object->ceiling = TrapDoorCeiling;
			object->SetupHitEffect(true);
		}
	}

	for (int objectNumber = ID_CEILING_TRAPDOOR1; objectNumber <= ID_CEILING_TRAPDOOR2; objectNumber++)
	{
		object = &Objects[objectNumber];
		if (object->loaded)
		{
			object->Initialize = InitializeTrapDoor;
			object->collision = CeilingTrapDoorCollision;
			object->control = TrapDoorControl;
			object->floorBorder = TrapDoorFloorBorder;
			object->ceilingBorder = TrapDoorCeilingBorder;
			object->floor = TrapDoorFloor;
			object->ceiling = TrapDoorCeiling;
			object->SetupHitEffect(true);
		}
	}

	object = &Objects[ID_BRIDGE_FLAT];
	if (object->loaded)
	{
		object->Initialize = InitializeBridge;
		object->floor = BridgeFloor<0>;
		object->ceiling = BridgeCeiling<0>;
		object->floorBorder = BridgeFloorBorder<0>;
		object->ceilingBorder = BridgeCeilingBorder<0>;
	}

	object = &Objects[ID_BRIDGE_TILT1];
	if (object->loaded)
	{
		object->Initialize = InitializeBridge;
		object->floor = BridgeFloor<1>;
		object->ceiling = BridgeCeiling<1>;
		object->floorBorder = BridgeFloorBorder<1>;
		object->ceilingBorder = BridgeCeilingBorder<1>;
	}

	object = &Objects[ID_BRIDGE_TILT2];
	if (object->loaded)
	{
		object->Initialize = InitializeBridge;
		object->floor = BridgeFloor<2>;
		object->ceiling = BridgeCeiling<2>;
		object->floorBorder = BridgeFloorBorder<2>;
		object->ceilingBorder = BridgeCeilingBorder<2>;
	}

	object = &Objects[ID_BRIDGE_TILT3];
	if (object->loaded)
	{
		object->Initialize = InitializeBridge;
		object->floor = BridgeFloor<3>;
		object->ceiling = BridgeCeiling<3>;
		object->floorBorder = BridgeFloorBorder<3>;
		object->ceilingBorder = BridgeCeilingBorder<3>;
	}

	object = &Objects[ID_BRIDGE_TILT4];
	if (object->loaded)
	{
		object->Initialize = InitializeBridge;
		object->floor = BridgeFloor<4>;
		object->ceiling = BridgeCeiling<4>;
		object->floorBorder = BridgeFloorBorder<4>;
		object->ceilingBorder = BridgeCeilingBorder<4>;
	}
}

void StartSwitches(ObjectInfo* object)
{
	object = &Objects[ID_COG_SWITCH];
	if (object->loaded)
	{
		object->collision = CogSwitchCollision;
		object->control = CogSwitchControl;
		object->SetupHitEffect(true);
	}

	object = &Objects[ID_LEVER_SWITCH];
	if (object->loaded)
	{
		object->collision = RailSwitchCollision;
		object->control = SwitchControl;
		object->SetupHitEffect(true);
	}

	object = &Objects[ID_JUMP_SWITCH];
	if (object->loaded)
	{
		object->collision = JumpSwitchCollision;
		object->control = SwitchControl;
		object->SetupHitEffect(true);
	}

	for (int objectNumber = ID_SWITCH_TYPE1; objectNumber <= ID_SWITCH_TYPE16; objectNumber++)
	{
		object = &Objects[objectNumber];
		if (object->loaded)
		{
			object->collision = SwitchCollision;
			object->control = SwitchControl;
			object->SetupHitEffect(true);
		}
	}

	object = &Objects[ID_AIRLOCK_SWITCH];
	if (object->loaded)
	{
		object->collision = AirlockSwitchCollision;
		object->control = SwitchControl;
		object->SetupHitEffect(true);
	}

	object = &Objects[ID_CROWBAR_SWITCH];
	if (object->loaded)
	{
		object->collision = CrowbarSwitchCollision;
		object->control = SwitchControl;
		object->SetupHitEffect(true);
	}

	object = &Objects[ID_MINECART_SWITCH];
	if (object->loaded)
	{
		object->Initialize = InitializeAnimating;
		object->control = AnimatingControl;
		object->collision = ObjectCollision;
		object->SetupHitEffect(true);
		object->shadowType = ShadowMode::All;
	}

	for (int objectNumber = ID_UNDERWATER_SWITCH1; objectNumber <= ID_UNDERWATER_SWITCH4; objectNumber++)
	{
		object = &Objects[objectNumber];
		if (object->loaded)
		{
			object->control = SwitchControl;
			object->collision = UnderwaterSwitchCollision;
		}
	}

	object = &Objects[ID_PULLEY];
	if (object->loaded)
	{
		object->Initialize = InitializePulleySwitch;
		object->control = SwitchControl;
		object->collision = PulleySwitchCollision;
	}

	object = &Objects[ID_TURN_SWITCH];
	if (object->loaded)
	{
		object->control = TurnSwitchControl;
		object->collision = TurnSwitchCollision;
		object->SetupHitEffect(true);
	}

	object = &Objects[ID_SEQUENCE_SWITCH1];
	if (object->loaded)
	{
		object->collision = FullBlockSwitchCollision;
		object->control = FullBlockSwitchControl;
		object->SetupHitEffect(true);
	}

	object = &Objects[ID_SEQUENCE_SWITCH2];
	if (object->loaded)
	{
		object->collision = FullBlockSwitchCollision;
		object->control = FullBlockSwitchControl;
		object->SetupHitEffect(true);
	}

	object = &Objects[ID_SEQUENCE_SWITCH3];
	if (object->loaded)
	{
		object->collision = FullBlockSwitchCollision;
		object->control = FullBlockSwitchControl;
		object->SetupHitEffect(true);
	}

	for (int objectID = ID_SHOOT_SWITCH1; objectID <= ID_SHOOT_SWITCH4; objectID++)
	{
		object = &Objects[objectID];
		if (object->loaded)
		{
			object->Initialize = InitializeShootSwitch;
			object->control = ControlAnimatingSlots;
			object->collision = ShootSwitchCollision;
		}
	}

	for (int objectID = ID_KEY_HOLE1; objectID <= ID_KEY_HOLE16; objectID++)
		InitKeyHole(object, objectID);

	for (int objectID = ID_PUZZLE_HOLE1; objectID <= ID_PUZZLE_HOLE16; objectID++)
		InitPuzzleHole(object, objectID);

	for (int objectID = ID_PUZZLE_DONE1; objectID <= ID_PUZZLE_DONE16; objectID++)
		InitPuzzleDone(object, objectID);
}

void StartDoors(ObjectInfo* object)
{
	for (int objectNumber = ID_DOOR_TYPE1; objectNumber <= ID_DOOR_TYPE30; objectNumber++)
	{
		object = &Objects[objectNumber];
		if (object->loaded)
		{
			object->Initialize = InitializeDoor;
			object->control = DoorControl;
			object->collision = DoorCollision;
			object->SetupHitEffect(true);
		}
	}

	object = &Objects[ID_LIFT_DOORS1];
	if (object->loaded)
	{
		object->Initialize = InitializeDoor;
		object->control = DoorControl;
		object->SetupHitEffect(true);
	}

	object = &Objects[ID_LIFT_DOORS2];
	if (object->loaded)
	{
		object->Initialize = InitializeDoor;
		object->control = DoorControl;
		object->SetupHitEffect(true);
	}

	object = &Objects[ID_SEQUENCE_DOOR1];
	if (object->loaded)
	{
		object->Initialize = InitializeDoor;
		object->collision = DoorCollision;
		object->control = SequenceDoorControl;
		object->SetupHitEffect(true);
	}

	for (int i = ID_DOUBLE_DOORS1; i <= ID_DOUBLE_DOORS4; i++)
	{
		object = &Objects[i];
		if (object->loaded)
		{
			object->Initialize = InitializeDoor;
			object->collision = DoubleDoorCollision;
			object->control = PushPullKickDoorControl;
			object->SetupHitEffect(true);
		}
	}

	for (int i = ID_UNDERWATER_DOOR1; i <= ID_UNDERWATER_DOOR4; i++)
	{
		object = &Objects[i];
		if (object->loaded)
		{
			object->Initialize = InitializeDoor;
			object->collision = UnderwaterDoorCollision;
			object->control = PushPullKickDoorControl;
			object->SetupHitEffect(true);
		}
	}

	for (int objectNum = ID_PUSHPULL_DOOR1; objectNum <= ID_KICK_DOOR4; objectNum++)
	{
		object = &Objects[objectNum];
		if (object->loaded)
		{
			object->Initialize = InitializeDoor;
			object->collision = PushPullKickDoorCollision;
			object->control = PushPullKickDoorControl;
			object->SetupHitEffect(true);
		}
	}

	object = &Objects[ID_STEEL_DOOR];
	if (object->loaded)
	{
		object->Initialize = InitializeSteelDoor;
		object->collision = SteelDoorCollision;
		object->SetupHitEffect(true);
	}
}

void StartTraps(ObjectInfo* object)
{
	object = &Objects[ID_DARTS];
	if (object->loaded)
	{
		object->collision = ObjectCollision;
		object->control = DartControl;
		object->shadowType = ShadowMode::All;
	}

	object = &Objects[ID_DART_EMITTER];
	if (object->loaded)
	{
		object->control = DartEmitterControl;
		object->drawRoutine = nullptr;
		object->usingDrawAnimatingItem = false;
	}

	object = &Objects[ID_HOMING_DART_EMITTER];
	if (object->loaded)
	{
		object->control = DartEmitterControl;
		object->drawRoutine = nullptr;
		object->usingDrawAnimatingItem = false;
	}

	object = &Objects[ID_ROPE];
	if (object->loaded)
	{
		object->Initialize = InitializeRope;
		object->control = RopeControl;
		object->collision = RopeCollision;
		object->drawRoutine = nullptr;
		object->usingDrawAnimatingItem = false;
	}

	object = &Objects[ID_POLEROPE];
	if (object->loaded)
	{
		object->collision = PoleCollision;
		object->SetupHitEffect(true);
	}

	object = &Objects[ID_BURNING_TORCH_ITEM];
	if (object->loaded)
	{
		object->control = TorchControl;
		object->collision = PickupCollision;
		object->usingDrawAnimatingItem = true;
		object->isPickup = true;
	}

	object = &Objects[ID_FALLING_BLOCK];
	if (object->loaded)
	{
		object->Initialize = InitializeFallingBlock;
		object->collision = FallingBlockCollision;
		object->control = FallingBlockControl;
		object->floor = FallingBlockFloor;
		object->ceiling = FallingBlockCeiling;
		object->floorBorder = FallingBlockFloorBorder;
		object->ceilingBorder = FallingBlockCeilingBorder;
	}

	object = &Objects[ID_FALLING_BLOCK2];
	if (object->loaded)
	{
		object->Initialize = InitializeFallingBlock;
		object->collision = FallingBlockCollision;
		object->control = FallingBlockControl;
		object->floor = FallingBlockFloor;
		object->ceiling = FallingBlockCeiling;
		object->floorBorder = FallingBlockFloorBorder;
		object->ceilingBorder = FallingBlockCeilingBorder;
	}

	object = &Objects[ID_CRUMBLING_FLOOR];
	if (object->loaded)
	{
		object->Initialize = InitializeFallingBlock;
		object->collision = FallingBlockCollision;
		object->control = FallingBlockControl;
	}

	object = &Objects[ID_PARALLEL_BARS];
	if (object->loaded)
		object->collision = HorizontalBarCollision;
}

void StartServiceObjects(ObjectInfo* object)
{
	object = &Objects[ID_CAMERA_TARGET];
	if (object->loaded)
	{
		object->drawRoutine = nullptr;
		object->usingDrawAnimatingItem = false;
	}

	object = &Objects[ID_TIGHT_ROPE];
	if (object->loaded)
	{
		object->Initialize = InitializeTightrope;
		object->collision = TightropeCollision;
		object->drawRoutine = nullptr;

		object->usingDrawAnimatingItem = false;
	}

	object = &Objects[ID_EARTHQUAKE];
	if (object->loaded)
		object->drawRoutine = nullptr;

	object = &Objects[ID_KILL_ALL_TRIGGERS];
	if (object->loaded)
	{
		object->control = KillAllCurrentItems;
		object->drawRoutine = nullptr;
		object->HitPoints = 0;
		object->usingDrawAnimatingItem = false;
	}

	object = &Objects[ID_TRIGGER_TRIGGERER];
	if (object->loaded)
	{
		object->control = ControlTriggerTriggerer;
		object->drawRoutine = nullptr;

		object->usingDrawAnimatingItem = false;
	}

	object = &Objects[ID_WATERFALLMIST];
	if (object->loaded)
	{
		object->control = ControlWaterfallMist;
		object->drawRoutine = nullptr;
	}

	for (int objectID = ID_ANIMATING1; objectID <= ID_ANIMATING128; objectID++)
		InitAnimating(object, objectID);
}

void InitializeGenericObjects()
{
	ObjectInfo* objToInit = nullptr;
	StartTraps(objToInit);
	StartObject(objToInit);
	StartSwitches(objToInit);
	StartDoors(objToInit);
	StartServiceObjects(objToInit);
}
