#pragma once

#include "setup.h"
#include "..\Game\draw.h"
#include "..\Game\collide.h"
#include "..\Game\Box.h"
#include "..\Game\missile.h"
#include "..\Game\control.h"

/*#include "..\CustomObjects\tiger.h"
#include "..\CustomObjects\customtraps.h"
#include "..\CustomObjects\cobra.h"
#include "..\CustomObjects\dino.h"
#include "..\CustomObjects\eagle.h"
#include "..\CustomObjects\bear.h"
#include "..\CustomObjects\frogman.h"
#include "..\CustomObjects\tribesman.h"
#include "..\CustomObjects\raptor.h"
#include "..\CustomObjects\shark.h"*/

#include <stdlib.h>
#include <stdio.h>

void __cdecl NewObjects()
{
	/*OBJECT_INFO* obj;

	obj = &Objects[ID_SPIKEY_WALL];
	if (obj->loaded)
	{
		obj->control = ControlSpikeWall;
		obj->collision = ObjectCollision;
		obj->savePosition = true;
		obj->saveFlags = true;
	}

	obj = &Objects[ID_TIGER];
	if (obj->loaded)
	{
		obj->control = TigerControl;
		obj->collision = CreatureCollision;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->hitPoints = TIGER_hitPoints;
		obj->pivotLength = 200;
		obj->radius = TIGER_RADIUS;
		obj->intelligent = true;
		obj->savePosition = true;
		obj->saveHitpoints = true;
		obj->saveAnim = true;
		obj->saveFlags = true;

		Bones[obj->boneIndex + 21 * 4] |= ROT_Y;
	}

	obj = &Objects[ID_COBRA];
	if (obj->loaded)
	{
		obj->initialise = InitialiseCobra;
		obj->control = CobraControl;
		obj->collision = CreatureCollision;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->hitPoints = COBRA_hitPoints;
		obj->radius = COBRA_RADIUS;
		obj->intelligent = true;
		obj->nonLot = true;
		obj->savePosition = true;
		obj->saveHitpoints = true;
		obj->saveAnim = true;
		obj->saveFlags = true;

		Bones[obj->boneIndex + 0 * 4] |= ROT_Y;
		Bones[obj->boneIndex + 6 * 4] |= ROT_Y;
	}

	obj = &Objects[ID_TREX];
	if (obj->loaded)
	{
		OBJECT_INFO* obj = &Objects[ID_TREX];

		obj->control = DinoControl;
		obj->collision = CreatureCollision;
		obj->hitPoints = DINO_hitPoints;
		obj->shadowSize = UNIT_SHADOW / 4;
		obj->pivotLength = 1800;
		obj->radius = DINO_RADIUS;
		obj->intelligent = true;
		obj->savePosition = true;
		obj->saveHitpoints = true;
		obj->saveAnim = true;
		obj->saveFlags = true;

		Bones[obj->boneIndex + 10 * 4] |= ROT_Y;
		Bones[obj->boneIndex + 11 * 4] |= ROT_Y;
	}

	obj = &Objects[ID_FROGMAN];
	if (obj->loaded)
	{
		obj->control = FrogManControl;
		obj->collision = CreatureCollision;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->hitPoints = DIVER_hitPoints;
		obj->radius = DIVER_RADIUS;
		obj->intelligent = true;
		obj->waterCreature = true;
		obj->savePosition = true;
		obj->saveHitpoints = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->pivotLength = 50;

		Bones[obj->boneIndex + 10 * 4] |= ROT_Y;
		Bones[obj->boneIndex + 14 * 4] |= ROT_Z;
	}

	obj = &Objects[ID_FROGMAN_HARPOON];
	if (obj->loaded)
	{
		obj->control = HarpoonControl;
		obj->collision = ObjectCollision;
		obj->savePosition = true;

		printf("nmeshes: %d\n", obj->nmeshes);
		printf("meshIndex: %d\n", obj->meshIndex);
		printf("boneIndex: %d\n", obj->boneIndex);
		printf("frameBase: %d\n", obj->frameBase);
		printf("initialise: %d\n", obj->initialise);
		printf("control: %d\n", obj->control);
		printf("floor: %d\n", obj->floor);
		printf("ceiling: %d\n", obj->ceiling);
		printf("collision: %d\n", obj->collision);
		printf("drawRoutine: %d\n", obj->drawRoutine);
		printf("drawRoutineExtra: %d\n", obj->drawRoutineExtra);
		printf("objectMip: %d\n", obj->objectMip);
	}

	obj = &Objects[ID_TRIBESMAN_AX];
	if (obj->loaded)
	{
		obj->control = TribeAxeControl;
		obj->collision = CreatureCollision;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->hitPoints = TRIBEAXE_hitPoints;
		obj->radius = TRIBEAXE_RADIUS;
		obj->intelligent = 1;
		obj->savePosition = obj->saveHitpoints = obj->saveAnim = obj->saveFlags = 1;
		obj->pivotLength = 0;

		Bones[obj->boneIndex + 13 * 4] |= ROT_Y;
		Bones[obj->boneIndex + 6 * 4] |= ROT_Y;
	}

	obj = &Objects[ID_EAGLE];
	if (obj->loaded)
	{
		obj->initialise = InitialiseEagle;
		obj->control = EagleControl;
		obj->collision = CreatureCollision;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->hitPoints = EAGLE_hitPoints;
		obj->radius = EAGLE_RADIUS;
		obj->intelligent = 1;
		obj->savePosition = obj->saveHitpoints = obj->saveAnim = obj->saveFlags = 1;
		obj->pivotLength = 0;
	}

	obj = &Objects[ID_CROW];
	if (obj->loaded)
	{
		obj->initialise = InitialiseEagle;
		obj->control = EagleControl;
		obj->collision = CreatureCollision;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->hitPoints = CROW_hitPoints;
		obj->radius = CROW_RADIUS;
		obj->intelligent = 1;
		obj->savePosition = obj->saveHitpoints = obj->saveAnim = obj->saveFlags = 1;
		obj->pivotLength = 0;
	}

	obj = &Objects[ID_RAPTOR];
	if (obj->loaded)
	{
		obj->control = RaptorControl;
		obj->collision = CreatureCollision;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->hitPoints = RAPTOR_hitPoints;
		obj->radius = RAPTOR_RADIUS;
		obj->intelligent = 1;
		obj->savePosition = obj->saveHitpoints = obj->saveAnim = obj->saveFlags = 1;
		obj->pivotLength = 600;
		Bones[obj->boneIndex + 20 * 4] |= ROT_Y;
		Bones[obj->boneIndex + 21 * 4] |= ROT_Y;
		Bones[obj->boneIndex + 23 * 4] |= ROT_Y;
		Bones[obj->boneIndex + 25 * 4] |= ROT_Y;
	}

	obj = &Objects[ID_SHARK];
	if (obj->loaded)
	{
		obj->control = SharkControl;
		//obj->draw_routine = DrawUnclippedItem;
		obj->collision = CreatureCollision;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->hitPoints = SHARK_HIT_POINTS;
		obj->pivotLength = 200;
		obj->radius = SHARK_RADIUS;
		obj->intelligent = 1;
		obj->waterCreature = 1;
		obj->savePosition = obj->saveHitpoints = obj->saveAnim = obj->saveFlags = 1;
		Bones[obj->boneIndex + 9 * 4] |= ROT_Y;
	}

	obj = &Objects[ID_BARACUDDA];
	if (obj->loaded)
	{	
		obj->control = BaracuddaControl;
		obj->collision = CreatureCollision;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->hitPoints = BARACUDDA_HIT_POINTS;
		obj->pivotLength = 200;
		obj->radius = BARACUDDA_RADIUS;
		obj->intelligent = 1;
		obj->waterCreature = 1;
		obj->savePosition = obj->saveHitpoints = obj->saveAnim = obj->saveFlags = 1;
		Bones[obj->boneIndex + 6 * 4] |= ROT_Y;
	}

	obj = &Objects[ID_BELL_SWITCH];
	obj->control = BellControl;
	obj->collision = ObjectCollision;
	obj->saveAnim = obj->saveFlags = 1;

	obj = &Objects[ID_ROLLING_SPINDLE];
	obj->initialise = InitialiseSpinningBlade; // just conveniently needs same start state and anim type
	obj->control = SpinningBlade;
	obj->collision = ObjectCollision;
	obj->savePosition = obj->saveAnim = obj->saveFlags = 1;

	obj = &Objects[ID_SPRING_BOARD];
	obj->control = SpringBoardControl;
	obj->saveAnim = obj->saveFlags = 1;*/
}

void __cdecl CustomObjects()
{

}

void __cdecl InitialiseObjects()
{
	for (__int32 i = 0; i < NUM_OBJECTS; i++)
	{
		Objects[i].drawRoutine = DrawAnimatingItem;
		Objects[i].floor = NULL;
		Objects[i].ceiling = NULL;
		Objects[i].pivotLength = 0;
		Objects[i].radius = 10;
		Objects[i].shadowSize = 0;
		Objects[i].hitPoints = -16384;
		Objects[i].explodableMeshbits = 0;
		Objects[i].intelligent = 0;
		Objects[i].waterCreature = 0;
		Objects[i].saveMesh = 0;
		Objects[i].saveAnim = 0;
		Objects[i].saveFlags = 0;
		Objects[i].saveHitpoints = 0;
		Objects[i].savePosition = 0;
		Objects[i].frameBase += (ptrdiff_t)Frames;
	}

	// Standard TR5 objects
	BaddyObjects();
	ObjectObjects();
	TrapObjects();

	// Reset MIP flag so we can reuse slots
	for (__int16 i = 0; i < NUM_OBJECTS; i++)
		Objects[i].objectMip = 0;
		
	// New objects imported from old TRs
	NewObjects();

	// User defined objects
	CustomObjects();

	InitialiseHairs();
	InitialiseSpecialEffects();

	CurrentSequence = 0;
	OldPickupPos.roomNumber = 0;
	
	// TODO: with the new renderer this can be safely deleted
	for (__int32 i = 0; i < 6; i++)
	{
		SequenceUsed[i] = 0;
	}

	for (__int32 i = 0; i < gfNumMips; i++)
	{
	}

	if (Objects[ID_BATS].loaded)
		Bats = GameMalloc(1920);

	if (Objects[ID_SPIDER].loaded)
		Spiders = GameMalloc(1664);

	if (Objects[ID_RATS].loaded)
		Rats = GameMalloc(832);
}

void Inject_Setup()
{
	INJECT(0x00473600, InitialiseObjects);
}