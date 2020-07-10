#include "framework.h"
#include "setup.h"
#include "draw.h"
#include "collide.h"
#include "effect2.h"
#include "effect.h"
#include "tomb4fx.h"
#include "switch.h"
#include "missile.h"
#include "control.h"
#include "pickup.h"
#include "camera.h"
#include "lara1gun.h"
#include "laraflar.h"
#include "larafire.h"
#include "laramisc.h"
#include "objects.h"
#include "door.h"
#include "rope.h"
#include "traps.h"
#include "flmtorch.h"
#include "level.h"
#include "tr4_bubbles.h"
/// objects initializer
#include "tr1_objects.h"
#include "tr2_objects.h"
#include "tr3_objects.h"
#include "tr4_objects.h"
#include "tr5_objects.h"
/// register objects
#include "object_helper.h"

extern byte SequenceUsed[6];
extern byte SequenceResults[3][3][3];
extern byte Sequences[3];
extern byte CurrentSequence;
extern int NumRPickups;

OBJECT_INFO Objects[ID_NUMBER_OBJECTS];
StaticInfo StaticObjects[MAX_STATICS];

void InitialiseGameFlags()
{
	ZeroMemory(FlipMap, MAX_FLIPMAP * sizeof(int));
	ZeroMemory(FlipStats, MAX_FLIPMAP * sizeof(int));

	FlipEffect = -1;
	FlipStatus = 0;
	IsAtmospherePlaying = 0;
	Camera.underwater = 0;
}

void ObjectObjects()
{
	OBJECT_INFO* obj;

	obj = &Objects[ID_CAMERA_TARGET];
	if (obj->loaded)
	{
		obj->drawRoutine = nullptr;
		obj->usingDrawAnimatingItem = false;
	}

	obj = &Objects[ID_SMASH_OBJECT1];
	if (obj->loaded)
	{
		obj->initialise = InitialiseSmashObject;
		obj->collision = ObjectCollision;
		obj->control = SmashObjectControl;
		obj->saveFlags = true;
		obj->saveAnim = true;
		obj->saveMesh = true;
	}

	obj = &Objects[ID_SMASH_OBJECT2];
	if (obj->loaded)
	{
		obj->initialise = InitialiseSmashObject;
		obj->collision = ObjectCollision;
		obj->control = SmashObjectControl;
		obj->saveFlags = true;
		obj->saveAnim = true;
		obj->saveMesh = true;
	}

	obj = &Objects[ID_SMASH_OBJECT3];
	if (obj->loaded)
	{
		obj->initialise = InitialiseSmashObject;
		obj->collision = ObjectCollision;
		obj->control = SmashObjectControl;
		obj->saveFlags = true;
		obj->saveAnim = true;
		obj->saveMesh = true;
	}

	obj = &Objects[ID_SMASH_OBJECT4];
	if (obj->loaded)
	{
		obj->initialise = InitialiseSmashObject;
		obj->collision = ObjectCollision;
		obj->control = SmashObjectControl;
		obj->saveFlags = true;
		obj->saveAnim = true;
		obj->saveMesh = true;
	}

	obj = &Objects[ID_SMASH_OBJECT5];
	if (obj->loaded)
	{
		obj->initialise = InitialiseSmashObject;
		obj->collision = ObjectCollision;
		obj->control = SmashObjectControl;
		obj->saveFlags = true;
		obj->saveAnim = true;
		obj->saveMesh = true;
	}

	obj = &Objects[ID_SMASH_OBJECT6];
	if (obj->loaded)
	{
		obj->initialise = InitialiseSmashObject;
		obj->collision = ObjectCollision;
		obj->control = SmashObjectControl;
		obj->saveFlags = true;
		obj->saveAnim = true;
		obj->saveMesh = true;
	}

	obj = &Objects[ID_SMASH_OBJECT7];
	if (obj->loaded)
	{
		obj->initialise = InitialiseSmashObject;
		obj->collision = ObjectCollision;
		obj->control = SmashObjectControl;
		obj->saveFlags = true;
		obj->saveAnim = true;
		obj->saveMesh = true;
	}

	obj = &Objects[ID_SMASH_OBJECT8];
	if (obj->loaded)
	{
		obj->initialise = InitialiseSmashObject;
		obj->collision = ObjectCollision;
		obj->control = SmashObjectControl;
		obj->saveFlags = true;
		obj->saveAnim = true;
		obj->saveMesh = true;
	}

	obj = &Objects[ID_BRIDGE_FLAT];
	if (obj->loaded)
	{
		obj->floor = BridgeFlatFloor;
		obj->ceiling = BridgeFlatCeiling;
	}

	obj = &Objects[ID_BRIDGE_TILT1];
	if (obj->loaded)
	{
		obj->floor = BridgeTilt1Floor;
		obj->ceiling = BridgeTilt1Ceiling;
	}

	obj = &Objects[ID_BRIDGE_TILT2];
	if (obj->loaded)
	{
		obj->floor = BridgeTilt2Floor;
		obj->ceiling = BridgeTilt2Ceiling;
	}

	obj = &Objects[ID_CRUMBLING_FLOOR];
	if (obj->loaded)
	{
		obj->initialise = InitialiseFallingBlock;
		obj->collision = FallingBlockCollision;
		obj->control = FallingBlockControl;
		obj->floor = FallingBlockFloor;
		obj->ceiling = FallingBlockCeiling;
		obj->saveFlags = true;
		obj->savePosition = true;
		obj->saveMesh = true;
	}

	for (int objNum = ID_SWITCH_TYPE1; objNum <= ID_SWITCH_TYPE16; objNum++)
	{
		obj = &Objects[objNum];
		if (obj->loaded)
		{
			obj->initialise = InitialiseSwitch;
			obj->collision = SwitchCollision;
			obj->control = SwitchControl;
			obj->saveFlags = true;
			obj->saveAnim = true;
			obj->saveMesh = true;
		}
	}

	obj = &Objects[ID_AIRLOCK_SWITCH];
	if (obj->loaded)
	{
		obj->collision = SwitchCollision;
		obj->control = SwitchControl;
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

	for (int objNum = ID_UNDERWATER_SWITCH1; objNum <= ID_UNDERWATER_SWITCH4; objNum++)
	{
		obj = &Objects[objNum];
		if (obj->loaded)
		{
			obj->control = SwitchControl;
			obj->collision = UnderwaterSwitchCollision;
			obj->saveFlags = true;
			obj->saveAnim = true;
		}
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

	obj = &Objects[ID_CROWBAR_SWITCH];
	if (obj->loaded)
	{
		obj->collision = CrowbarSwitchCollision;
		obj->control = SwitchControl;
		obj->saveFlags = true;
		obj->saveAnim = true;
	}

	obj = &Objects[ID_PULLEY];
	if (obj->loaded)
	{
		obj->initialise = InitialisePulleySwitch;
		obj->control = SwitchControl;
		obj->collision = PulleyCollision;
		obj->saveFlags = true;
		obj->saveAnim = true;
	}

	

	obj = &Objects[ID_CROWDOVE_SWITCH];
	if (obj->loaded)
	{
		obj->initialise = InitialiseCrowDoveSwitch;
		obj->collision = CrowDoveSwitchCollision;
		obj->control = CrowDoveSwitchControl;
		obj->saveFlags = true;
		obj->saveAnim = true;
		obj->saveMesh = true;
	}

	for (int objNum = ID_DOOR_TYPE1; objNum <= ID_CLOSED_DOOR6; objNum++)
	{
		obj = &Objects[objNum];
		if (obj->loaded)
		{
			obj->initialise = InitialiseDoor;
			obj->control = DoorControl;
			obj->collision = DoorCollision;
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
		obj->saveFlags = true;
	}

	obj = &Objects[ID_LIFT_DOORS2];
	if (obj->loaded)
	{
		obj->initialise = InitialiseDoor;
		obj->control = DoorControl;
		obj->saveFlags = true;
	}

	obj = &Objects[ID_SEQUENCE_DOOR1];
	if (obj->loaded)
	{
		obj->initialise = InitialiseDoor;
		obj->collision = DoorCollision;
		obj->control = SequenceDoorControl;
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
			obj->saveAnim = true;
			obj->saveFlags = true;
		}
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

	obj = &Objects[ID_TRAPDOOR1];
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

	for (int objNum = ID_KEY_HOLE1; objNum <= ID_KEY_HOLE16; objNum++)
	{
		InitKeyHole(obj, objNum);
	}

	for (int objNum = ID_PUZZLE_HOLE1; objNum <= ID_PUZZLE_HOLE16; objNum++)
	{
		InitPuzzleHole(obj, objNum);
	}

	for (int objNum = ID_PUZZLE_DONE1; objNum <= ID_PUZZLE_DONE16; objNum++)
	{
		InitPuzzleDone(obj, objNum);
	}
	
	for (int objNum = ID_ANIMATING1; objNum <= ID_ANIMATING128; objNum++)
	{
		InitAnimating(obj, objNum);
	}

	InitAnimating(obj, ID_LASERHEAD_BASE);
	InitAnimating(obj, ID_LASERHEAD_TENTACLE);

	obj = &Objects[ID_ANIMATING13];
	if (obj->loaded)
	{
		obj->initialise = InitialiseAnimating;
		obj->control = AnimatingControl;
		obj->collision = nullptr;
		obj->saveFlags = true;
		obj->saveAnim = true;
		obj->saveMesh = true;
		Bones[obj->boneIndex] |= ROT_Y;
		Bones[obj->boneIndex + 4] |= ROT_X;
	}

	obj = &Objects[ID_ANIMATING14];
	if (obj->loaded)
	{
		obj->initialise = InitialiseAnimating;
		obj->control = AnimatingControl;
		obj->collision = nullptr;
		obj->saveFlags = true;
		obj->saveAnim = true;
		obj->saveMesh = true;
		Bones[obj->boneIndex] |= ROT_Y;
		Bones[obj->boneIndex + 4] |= ROT_X;
	}

	obj = &Objects[ID_ANIMATING15];
	if (obj->loaded)
	{
		obj->initialise = InitialiseAnimating;
		obj->control = AnimatingControl;
		obj->collision = nullptr;
		obj->saveFlags = true;
		obj->saveAnim = true;
		obj->saveMesh = true;
		Bones[obj->boneIndex] |= ROT_Y;
		Bones[obj->boneIndex + 4] |= ROT_X;
	}

	obj = &Objects[ID_ANIMATING16];
	if (obj->loaded)
	{
		obj->initialise = InitialiseAnimating;
		obj->control = AnimatingControl;
		obj->collision = nullptr;
		obj->saveFlags = true;
		obj->saveAnim = true;
		obj->saveMesh = true;
		Bones[obj->boneIndex] |= ROT_Y;
		Bones[obj->boneIndex + 4] |= ROT_X;
	}

	obj = &Objects[ID_TIGHT_ROPE];
	if (obj->loaded)
	{
		obj->initialise = InitialiseTightRope;
		obj->collision = TightRopeCollision;
		obj->drawRoutine = nullptr;
		obj->saveFlags = true;
		obj->usingDrawAnimatingItem = false;
	}

	obj = &Objects[ID_PARALLEL_BARS];
	if (obj->loaded)
	{
		obj->collision = ParallelBarsCollision;
	}

	obj = &Objects[ID_STEEL_DOOR];
	if (obj->loaded)
	{
		obj->initialise = InitialiseSteelDoor;
		obj->collision = SteelDoorCollision;
		//obj->control = Legacy_SteelDoorControl;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->saveMesh = true;
		obj->savePosition = true;
	}

	obj = &Objects[ID_EARTHQUAKE];
	if (obj->loaded)
	{
		obj->drawRoutine = nullptr;
		obj->saveFlags = true;
	}

	obj = &Objects[ID_HIGH_OBJECT2];
	if (obj->loaded)
	{
		obj->drawRoutine = nullptr;
		obj->control = HighObject2Control;
	}

	obj = &Objects[ID_LENS_FLARE];
	if (obj->loaded)
	{
		//obj->drawRoutine = DrawLensFlare;
		obj->saveFlags = true;
	}

	obj = &Objects[ID_WATERFALLMIST];
	if (obj->loaded)
	{
		obj->control = ControlWaterfallMist;
		obj->drawRoutine = nullptr;
		obj->saveFlags = true;
	}

	for (int objNum = ID_WATERFALL1; objNum <= ID_WATERFALL6; objNum++)
	{
		obj = &Objects[objNum];
		if (obj->loaded)
		{
			obj->control = ControlWaterfall;
			obj->saveFlags = true;
		}
	}

	obj = &Objects[ID_WATERFALLSS1];
	if (obj->loaded)
	{
		obj->control = ControlWaterfall;
		obj->saveFlags = true;
	}

	obj = &Objects[ID_WATERFALLSS2];
	if (obj->loaded)
	{
		obj->control = ControlWaterfall;
		obj->saveFlags = true;
	}

	for (int objNum = ID_SHOOT_SWITCH1; objNum <= ID_SHOOT_SWITCH4; objNum++)
	{
		obj = &Objects[objNum];
		if (obj->loaded)
		{
			obj->initialise = InitialiseShootSwitch;
			obj->control = ControlAnimatingSlots;
			obj->collision = ShootSwitchCollision;
			obj->saveAnim = true;
			obj->saveFlags = true;
			obj->saveMesh = true;
		}
	}
}

void TrapObjects()
{
	OBJECT_INFO* obj;
	obj = &Objects[ID_KILL_ALL_TRIGGERS];
	if (obj->loaded)
	{
		obj->control = KillAllCurrentItems;
		obj->drawRoutine = nullptr;
		obj->hitPoints = 0;
		obj->saveFlags = true;
		obj->usingDrawAnimatingItem = false;
	}

	obj = &Objects[ID_FALLING_BLOCK];
	if (obj->loaded)
	{
		obj->initialise = InitialiseFallingBlock;
		obj->collision = FallingBlockCollision;
		obj->control = FallingBlockControl;
		obj->floor = FallingBlockFloor;
		obj->ceiling = FallingBlockCeiling;
		obj->saveFlags = true;
		obj->savePosition = true;
	}

	obj = &Objects[ID_FALLING_BLOCK2];
	if (obj->loaded)
	{
		obj->initialise = InitialiseFallingBlock;
		obj->collision = FallingBlockCollision;
		obj->control = FallingBlockControl;
		obj->floor = FallingBlockFloor;
		obj->ceiling = FallingBlockCeiling;
		obj->saveFlags = true;
		obj->savePosition = true;
	}

	// Flame is always loaded
	obj = &Objects[ID_FLAME];
	{
		obj->control = FlameControl;
		obj->drawRoutine = nullptr;
		obj->saveFlags = true;
		obj->usingDrawAnimatingItem = false;
	}

	obj = &Objects[ID_FLAME_EMITTER];
	if (obj->loaded)
	{
		obj->initialise = InitialiseFlameEmitter;
		obj->collision = FlameEmitterCollision;
		obj->control = FlameEmitterControl;
		obj->drawRoutine = nullptr;
		obj->saveFlags = true;
		obj->usingDrawAnimatingItem = false;
	}

	obj = &Objects[ID_FLAME_EMITTER2];
	if (obj->loaded)
	{
		obj->initialise = InitialiseFlameEmitter2;
		obj->collision = FlameEmitterCollision;
		obj->control = FlameEmitter2Control;
		obj->drawRoutine = nullptr;
		obj->saveFlags = true;
		obj->usingDrawAnimatingItem = false;
	}

	obj = &Objects[ID_FLAME_EMITTER3];
	if (obj->loaded)
	{
		obj->control = FlameEmitter3Control;
		obj->drawRoutine = nullptr;
		obj->saveFlags = true;
		obj->usingDrawAnimatingItem = false;
	}

	obj = &Objects[ID_GEN_SLOT2];
	if (obj->loaded)
	{
		/*obj->initialise = InitialiseGenSlot2;
		obj->control = GenSlot2Control;
		obj->drawRoutine = DrawGenSlot2;*/
		obj->usingDrawAnimatingItem = false;
		obj->saveFlags = true;
	}

	obj = &Objects[ID_PORTAL];
	if (obj->loaded)
	{
		//obj->initialise = InitialisePortal;
		//obj->control = PortalControl;        // TODO: found the control procedure !
		obj->drawRoutine = nullptr;             // go to nullsub_44() !
		obj->saveFlags = true; 
		obj->usingDrawAnimatingItem = false;
	}
	
	obj = &Objects[ID_TRIGGER_TRIGGERER];
	if (obj->loaded)
	{
		obj->control = ControlTriggerTriggerer;
		obj->drawRoutine = nullptr;
		obj->saveFlags = true;
		obj->usingDrawAnimatingItem = false;
	}

	//FIXME
	//InitialiseRopeTrap();

	obj = &Objects[ID_ROPE];
	if (obj->loaded)
	{
		obj->initialise = InitialiseRope;
		obj->control = RopeControl;
		obj->collision = RopeCollision;
		obj->drawRoutine = nullptr;
		obj->saveFlags = true;
		obj->usingDrawAnimatingItem = false;
	}

	obj = &Objects[ID_POLEROPE];
	if (obj->loaded)
	{
		obj->collision = PoleCollision;
		obj->saveFlags = true;
	}

	obj = &Objects[ID_WRECKING_BALL];
	if (obj->loaded)
	{
		obj->initialise = InitialiseWreckingBall;
		obj->collision = WreckingBallCollision;
		obj->control = WreckingBallControl;
		//obj->drawRoutineExtra = DrawWreckingBall;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->savePosition = true;
	}
}

void InitialiseSpecialEffects()
{
	int i;
	SPARKS* sptr;

	memset(&Sparks, 0, MAX_SPARKS * sizeof(SPARKS));
	memset(&FireSparks, 0, MAX_SPARKS_FIRE * sizeof(FIRE_SPARKS));
	memset(&SmokeSparks, 0, MAX_SPARKS_SMOKE * sizeof(SMOKE_SPARKS));
	memset(&Gunshells, 0, MAX_GUNSHELL * sizeof(GUNSHELL_STRUCT));
	memset(&Gunflashes, 0, (MAX_GUNFLASH * sizeof(GUNFLASH_STRUCT)));
	memset(&Blood, 0, MAX_SPARKS_BLOOD * sizeof(BLOOD_STRUCT));
	memset(&Splashes, 0, MAX_SPLASHES * sizeof(SPLASH_STRUCT));
	memset(&Ripples, 0, MAX_RIPPLES * sizeof(RIPPLE_STRUCT));
	memset(&Drips, 0, MAX_DRIPS * sizeof(DRIP_STRUCT));
	memset(&ShockWaves, 0, MAX_SHOCKWAVE * sizeof(SHOCKWAVE_STRUCT));

	sptr = &Sparks[0];
	for (i = 0; i < MAX_SPARKS; i++)
	{
		sptr->on = false;
		sptr->dynamic = -1;
		sptr++;
	}

	NextFireSpark = 1;
	NextSmokeSpark = 0;
	NextGunShell = 0;
	NextBubble = 0;
	NextDrip = 0;
	NextBlood = 0;
	WBRoom = -1;
}

void CustomObjects()
{
	
}

void InitialiseObjects()
{
	OBJECT_INFO* obj;

	for (int i = 0; i < ID_NUMBER_OBJECTS; i++)
	{
		obj = &Objects[i];
		obj->initialise = nullptr;
		obj->collision = nullptr;
		obj->control = nullptr;
		obj->floor = nullptr;
		obj->ceiling = nullptr;
		obj->drawRoutine = DrawAnimatingItem;
		obj->drawRoutineExtra = nullptr;
		obj->pivotLength = 0;
		obj->radius = DEFAULT_RADIUS;
		obj->shadowSize = NO_SHADOW;
		obj->hitPoints = -16384;
		obj->hitEffect = HIT_NONE;
		obj->explodableMeshbits = 0;
		obj->intelligent = false;
		obj->waterCreature = false;
		obj->saveMesh = false;
		obj->saveAnim = false;
		obj->saveFlags = false;
		obj->saveHitpoints = false;
		obj->savePosition = false;
		obj->nonLot = true;
		obj->usingDrawAnimatingItem = true;
		obj->semiTransparent = false;
		obj->undead = false;
		obj->zoneType = ZONE_NULL;
		obj->biteOffset = -1;
		obj->meshSwapSlot = NO_ITEM;
		obj->isPickup = false;
		obj->isPuzzleHole = false;
		obj->frameBase += (short)Frames;
	}

	InitialiseTR1Objects(); // Standard TR1 objects
	InitialiseTR2Objects(); // Standard TR2 objects
	InitialiseTR3Objects(); // Standard TR3 objects
	InitialiseTR4Objects(); // Standard TR4 objects
	InitialiseTR5Objects(); // Standard TR5 objects
	ObjectObjects();
	TrapObjects();

	// User defined objects
	CustomObjects();

	InitialiseHair();
	InitialiseSpecialEffects();

	NumRPickups = 0;
	CurrentSequence = 0;
	SequenceResults[0][1][2] = 0;
	SequenceResults[0][2][1] = 1;
	SequenceResults[1][0][2] = 2;
	SequenceResults[1][2][0] = 3;
	SequenceResults[2][0][1] = 4;
	SequenceResults[2][1][0] = 5;
	SequenceUsed[0] = 0;
	SequenceUsed[1] = 0;
	SequenceUsed[2] = 0;
	SequenceUsed[3] = 0;
	SequenceUsed[4] = 0;
	SequenceUsed[5] = 0;

	AllocTR5Objects();
}
