#pragma once

#include "setup.h"
#include "..\Game\draw.h"
#include "..\Game\collide.h"
#include "..\Game\Box.h"
#include "..\Game\switch.h"
#include "..\Game\missile.h"
#include "..\Game\control.h"
#include "..\Game\pickup.h"
#include "..\game\lara1gun.h"
#include "..\game\objects.h"
#include "..\Objects\newobjects.h"

#include <stdlib.h>
#include <stdio.h>

extern byte SequenceUsed[6];
extern byte SequenceResults[3][3][3];
extern byte Sequences[3];
extern byte CurrentSequence;

void __cdecl NewObjects()
{
	OBJECT_INFO* obj;

	obj = &Objects[ID_SMALL_SCORPION];
	if (obj->loaded)
	{
		obj->initialise = InitialiseSmallScorpion;
		obj->control = SmallScorpionControl;
		obj->collision = CreatureCollision;
		obj->shadowSize = 128;
		obj->hitPoints = 8;
		obj->pivotLength = 20;
		obj->radius = 128;
		obj->intelligent = true;
		obj->savePosition = true;
		obj->saveHitpoints = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->hitEffect = true;
	}

	obj = &Objects[ID_WILD_BOAR];
	if (obj->loaded)
	{
		obj->initialise = InitialiseWildBoar;
		obj->control = WildBoarControl;
		obj->collision = CreatureCollision;
		obj->shadowSize = 128;
		obj->hitPoints = 40;
		obj->pivotLength = 50;
		obj->radius = 102;
		obj->intelligent = true;
		obj->savePosition = true;
		obj->saveHitpoints = true;
		obj->saveAnim = true;
		obj->saveFlags = true;

		Bones[obj->boneIndex + 192] |= ROT_Z;
		Bones[obj->boneIndex + 192] |= ROT_Y;
		Bones[obj->boneIndex + 208] |= ROT_Z;
		Bones[obj->boneIndex + 208] |= ROT_Y;
	}

	obj = &Objects[ID_BAT];
	if (obj->loaded)
	{
		obj->initialise = InitialiseBat;
		obj->control = BatControl;
		obj->collision = CreatureCollision;
		obj->shadowSize = 128;
		obj->hitPoints = 5;
		obj->pivotLength = 10;
		obj->radius = 102;
		obj->intelligent = true;
		obj->savePosition = true;
		obj->saveHitpoints = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
	}

	obj = &Objects[ID_CHAIN];
	if (obj->loaded)
	{
		obj->control = ChainControl;
		obj->collision = GenericSphereBoxCollision;
		obj->saveAnim = true;
		obj->saveFlags = true;
	}

	obj = &Objects[ID_PLOUGH];
	if (obj->loaded)
	{
		obj->control = PloughControl;
		obj->collision = GenericSphereBoxCollision;
		obj->saveAnim = true;
		obj->saveFlags = true;
	}

	obj = &Objects[ID_FLOOR_4BLADES];
	if (obj->loaded)
	{
		obj->control = FourBladesControl;
		obj->collision = GenericSphereBoxCollision;
		obj->saveAnim = true;
		obj->saveFlags = true;
	}

	obj = &Objects[ID_CEILING_4BLADES];
	if (obj->loaded)
	{
		obj->control = FourBladesControl;
		obj->collision = GenericSphereBoxCollision;
		obj->saveAnim = true;
		obj->saveFlags = true;
	}

	obj = &Objects[ID_SPIKEBALL];
	if (obj->loaded)
	{
		obj->control = SpikeballControl;
		obj->collision = GenericSphereBoxCollision;
		obj->saveAnim = true;
		obj->saveFlags = true;
	}

	obj = &Objects[ID_COG];
	if (obj->loaded)
	{
		obj->control = CogControl;
		obj->collision = GenericSphereBoxCollision;
		obj->saveAnim = true;
		obj->saveFlags = true;
	}

	obj = &Objects[ID_BADDY1];
	if (obj->loaded)
	{
		obj->initialise = InitialiseBaddy;
		obj->control = BaddyControl;
		obj->collision = CreatureCollision;
		obj->shadowSize = 128;
		obj->hitPoints = 25;
		obj->pivotLength = 50;
		obj->radius = 102;
		obj->intelligent = true;
		obj->savePosition = true;
		obj->saveHitpoints = true;
		obj->saveAnim = true;
		obj->saveFlags = true;

		Bones[obj->boneIndex + 112] |= ROT_Y;
		Bones[obj->boneIndex + 112] |= ROT_X;
		Bones[obj->boneIndex + 352] |= ROT_Y;
		Bones[obj->boneIndex + 352] |= ROT_X;

		Meshes[obj->meshIndex + 36] = Meshes[Objects[ID_MESHSWAP1].meshIndex + 32];
		Meshes[obj->meshIndex + 60] = Meshes[Objects[ID_MESHSWAP1].meshIndex + 56];
		Meshes[obj->meshIndex + 36] = Meshes[Objects[ID_MESHSWAP1].meshIndex + 22];
	}

	obj = &Objects[ID_BADDY2];
	if (obj->loaded)
	{
		obj->initialise = InitialiseBaddy;
		obj->control = BaddyControl;
		obj->collision = CreatureCollision;
		obj->shadowSize = 128;
		obj->hitPoints = 25;
		obj->pivotLength = 50;
		obj->radius = 102;
		obj->intelligent = true;
		obj->savePosition = true;
		obj->saveHitpoints = true;
		obj->saveAnim = true;
		obj->saveFlags = true;

		Bones[obj->boneIndex + 112] |= ROT_Y;
		Bones[obj->boneIndex + 112] |= ROT_X;
		Bones[obj->boneIndex + 352] |= ROT_Y;
		Bones[obj->boneIndex + 352] |= ROT_X;
	}

	obj = &Objects[ID_SAS_CAIRO];
	if (obj->loaded)
	{
		obj->initialise = InitialiseSas;
		obj->control = SasControl;
		obj->collision = CreatureCollision;
		obj->shadowSize = 128;
		obj->hitPoints = 40;
		obj->pivotLength = 50;
		obj->radius = 102;
		obj->intelligent = true;
		obj->savePosition = true;
		obj->saveHitpoints = true;
		obj->saveAnim = true;
		obj->saveFlags = true;

		Bones[obj->boneIndex] |= ROT_Y;
		Bones[obj->boneIndex] |= ROT_X;
		Bones[obj->boneIndex + 112] |= ROT_Y;
		Bones[obj->boneIndex + 112] |= ROT_X;
	}

	obj = &Objects[ID_MUMMY];
	if (obj->loaded)
	{
		obj->initialise = InitialiseMummy;
		obj->control = MummyControl;
		obj->collision = CreatureCollision;
		obj->shadowSize = 128;
		obj->hitPoints = 15;
		obj->radius = 170;
		obj->intelligent = true;
		obj->savePosition = true;
		obj->saveHitpoints = true;
		obj->saveAnim = true;
		obj->saveFlags = true;

		Bones[obj->boneIndex + 112] |= ROT_Y;
		Bones[obj->boneIndex + 112] |= ROT_X;
		Bones[obj->boneIndex + 288] |= ROT_Y;
	}

	obj = &Objects[ID_QUAD];
	if (obj->loaded)
	{
		obj->initialise = InitialiseQuadBike;
		obj->collision = QuadBikeCollision;
		obj->savePosition = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
	}

	obj = &Objects[ID_SHARK];
	if (obj->loaded)
	{
		obj->control = SharkControl;
		obj->collision = CreatureCollision;
		obj->shadowSize = 128;
		obj->hitPoints = 30;
		obj->pivotLength = 200;
		obj->radius = 340;
		obj->intelligent = true;
		obj->waterCreature = true;
		obj->savePosition = true;
		obj->saveHitpoints = true;
		obj->saveAnim = true;
		obj->saveFlags = true;

		Bones[obj->boneIndex + 9 * 4] |= ROT_Y;
	}

	obj = &Objects[ID_BARRACUDA];
	if (obj->loaded)
	{
		obj->control = BarracudaControl;
		obj->collision = CreatureCollision;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->hitPoints = 12;
		obj->pivotLength = 200;
		obj->radius = 204;
		obj->intelligent = true;
		obj->waterCreature = true;
		obj->savePosition = true;
		obj->saveHitpoints = true;
		obj->saveAnim = true;
		obj->saveFlags = true;

		Bones[obj->boneIndex + 6 * 4] |= ROT_Y;
	}

	obj = &Objects[ID_ROLLING_SPINDLE];
	if (obj->loaded)
	{
		obj->initialise = InitialiseSpinningBlade;
		obj->control = SpinningBlade;
		obj->collision = ObjectCollision;
		obj->savePosition = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
	}

	obj = &Objects[ID_SPRINGBOARD];
	if (obj->loaded)
	{
		obj->control = SpringBoardControl;
		obj->saveAnim = true;
		obj->saveFlags = true;
	}

	obj = &Objects[ID_TIGER];
	if (obj->loaded)
	{
		obj->control = TigerControl;
		obj->collision = CreatureCollision;
		obj->shadowSize = 128;
		obj->hitPoints = 24;
		obj->pivotLength = 200;
		obj->radius = 340;
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
		obj->shadowSize = 128;
		obj->hitPoints = 8;
		obj->radius = 102;
		obj->intelligent = true;
		obj->nonLot = true;
		obj->savePosition = true;
		obj->saveHitpoints = true;
		obj->saveAnim = true;
		obj->saveFlags = true;

		Bones[obj->boneIndex + 0 * 4] |= ROT_Y;
		Bones[obj->boneIndex + 6 * 4] |= ROT_Y;
	}

	obj = &Objects[ID_RAPTOR];
	if (obj->loaded)
	{
		obj->control = RaptorControl;
		obj->collision = CreatureCollision;
		obj->shadowSize = 128;
		obj->hitPoints = 100;
		obj->radius = 341;
		obj->pivotLength = 600;
		obj->intelligent = true;
		obj->savePosition = true;
		obj->saveHitpoints = true;
		obj->saveAnim = true;
		obj->saveFlags = true;

		Bones[obj->boneIndex + 20 * 4] |= ROT_Y;
		Bones[obj->boneIndex + 21 * 4] |= ROT_Y;
		Bones[obj->boneIndex + 23 * 4] |= ROT_Y;
		Bones[obj->boneIndex + 25 * 4] |= ROT_Y;
	}

	obj = &Objects[ID_SCUBA_DIVER];
	if (obj->loaded)
	{
		obj->control = ScubaControl;
		obj->collision = CreatureCollision;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->hitPoints = 20;
		obj->radius = 340;
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

	obj = &Objects[ID_SCUBA_HARPOON];
	if (obj->loaded)
	{
		obj->control = HarpoonControl;
		obj->collision = ObjectCollision;
		obj->savePosition = true;
	}

	obj = &Objects[ID_EAGLE];
	if (obj->loaded)
	{
		obj->initialise = InitialiseEagle;
		obj->control = EagleControl;
		obj->collision = CreatureCollision;
		obj->shadowSize = 128;
		obj->hitPoints = 20;
		obj->radius = 204;
		obj->intelligent = true;
		obj->savePosition = true;
		obj->saveHitpoints = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->pivotLength = 0;
	}

	obj = &Objects[ID_CROW];
	if (obj->loaded)
	{
		obj->initialise = InitialiseEagle;
		obj->control = EagleControl;
		obj->collision = CreatureCollision;
		obj->shadowSize = 128;
		obj->hitPoints = 15;
		obj->radius = 204;
		obj->intelligent = true;
		obj->savePosition = true;
		obj->saveHitpoints = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->pivotLength = 0;
	}

	obj = &Objects[ID_TRIBESMAN_WITH_AX];
	if (obj->loaded)
	{
		obj->control = TribemanAxeControl;
		obj->collision = CreatureCollision;
		obj->shadowSize = 128;
		obj->hitPoints = 28;
		obj->radius = 102;
		obj->intelligent = true;
		obj->savePosition = true;
		obj->saveHitpoints = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->pivotLength = 0;

		Bones[obj->boneIndex + 13 * 4] |= ROT_Y;
		Bones[obj->boneIndex + 6 * 4] |= ROT_Y;
	}

	obj = &Objects[ID_TRIBESMAN_WITH_DARTS];
	if (obj->loaded)
	{
		obj->control = TribesmanDartsControl;
		obj->collision = CreatureCollision;
		obj->shadowSize = 128;
		obj->hitPoints = 28;
		obj->radius = 102;
		obj->intelligent = true;
		obj->savePosition = true;
		obj->saveHitpoints = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->pivotLength = 0;

		Bones[obj->boneIndex + 13 * 4] |= ROT_Y;
		Bones[obj->boneIndex + 6 * 4] |= ROT_Y;
	}

	obj = &Objects[ID_WOLF];
	if (obj->loaded)
	{
		obj->initialise = InitialiseWolf;
		obj->control = WolfControl;
		obj->collision = CreatureCollision;
		obj->shadowSize = 128;
		obj->hitPoints = 6;
		obj->pivotLength = 375;
		obj->radius = 340;
		obj->intelligent = true;
		obj->savePosition = true;
		obj->saveHitpoints = true;
		obj->saveAnim = true;
		obj->saveFlags = true;

		Bones[obj->boneIndex + 2 * 4] |= ROT_Y;
	}

	obj = &Objects[ID_BEAR];
	if (obj->loaded)
	{
		obj->initialise = InitialiseCreature;
		obj->control = BearControl;
		obj->collision = CreatureCollision;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->hitPoints = 20;
		obj->pivotLength = 500;
		obj->radius = 340;
		obj->intelligent = true;
		obj->savePosition = true;
		obj->saveHitpoints = true;
		obj->saveAnim = true;
		obj->saveFlags = true;

		Bones[obj->boneIndex + 13 * 4] |= ROT_Y;
	}

	obj = &Objects[ID_TYRANNOSAUR];
	if (obj->loaded)
	{
		obj->control = TyrannosaurControl;
		obj->collision = CreatureCollision;
		obj->hitPoints = 800;
		obj->shadowSize = 64;
		obj->pivotLength = 1800;
		obj->radius = 512;
		obj->intelligent = true;
		obj->savePosition = true;
		obj->saveHitpoints = true;
		obj->saveAnim = true;
		obj->saveFlags = true;

		Bones[obj->boneIndex + 10 * 4] |= ROT_Y;
		Bones[obj->boneIndex + 11 * 4] |= ROT_Y;
	}

	obj = &Objects[ID_APE];
	if (obj->loaded)
	{
		obj->control = ApeControl;
		obj->collision = CreatureCollision;
		obj->hitPoints = 22;
		obj->shadowSize = 128;
		obj->pivotLength = 250;
		obj->radius = 340;
		obj->intelligent = true;
		obj->savePosition = true;
		obj->saveHitpoints = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
	}

	obj = &Objects[ID_RAT];
	if (obj->loaded)
	{
		obj->control = RatControl;
		obj->collision = CreatureCollision;
		obj->hitPoints = 5;
		obj->shadowSize = 128;
		obj->pivotLength = 50;
		obj->radius = 204;
		obj->intelligent = true;
		obj->savePosition = true;
		obj->saveHitpoints = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
	}

	obj = &Objects[ID_SKELETON];
	if (obj->loaded)
	{
		obj->initialise = InitialiseSkeleton;
		obj->control = SkeletonControl;
		obj->collision = CreatureCollision;
		obj->hitPoints = 15;
		obj->shadowSize = 128;
		obj->pivotLength = 50;
		obj->radius = 128;
		obj->explodableMeshbits = 0xA00;
		obj->intelligent = true;
		obj->savePosition = true;
		obj->saveHitpoints = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
	}

	obj = &Objects[ID_GRENADE_ITEM];
	if (obj->loaded)
	{
		obj->collision = PickupCollision;
		obj->savePosition = true;
		obj->saveFlags = true;
	}

	obj = &Objects[ID_GRENADE_AMMO1_ITEM];
	if (obj->loaded)
	{
		obj->collision = PickupCollision;
		obj->savePosition = true;
		obj->saveFlags = true;
	}

	obj = &Objects[ID_GRENADE_AMMO2_ITEM];
	if (obj->loaded)
	{
		obj->collision = PickupCollision;
		obj->savePosition = true;
		obj->saveFlags = true;
	}

	obj = &Objects[ID_GRENADE_AMMO3_ITEM];
	if (obj->loaded)
	{
		obj->collision = PickupCollision;
		obj->savePosition = true;
		obj->saveFlags = true;
	}

	obj = &Objects[ID_CROSSBOW_AMMO3_ITEM];
	if (obj->loaded)
	{
		obj->collision = PickupCollision;
		obj->savePosition = true;
		obj->saveFlags = true;
	}

	obj = &Objects[ID_GRENADE];
	if (obj->loaded)
	{
		obj->collision = NULL;
		obj->control = ControlGrenade;
	}

	obj = &Objects[ID_HARPOON_ITEM];
	if (obj->loaded)
	{
		obj->collision = PickupCollision;
		obj->savePosition = true;
		obj->saveFlags = true;
	}

	obj = &Objects[ID_HARPOON_AMMO_ITEM];
	if (obj->loaded)
	{
		obj->collision = PickupCollision;
		obj->savePosition = true;
		obj->saveFlags = true;
	}

	obj = &Objects[ID_HARPOON];
	if (obj->loaded)
	{
		obj->initialise = NULL;
		obj->collision = NULL;
		obj->control = ControlHarpoonBolt;
	}

	obj = &Objects[ID_CROSSBOW_BOLT];
	if (obj->loaded)
	{
		obj->initialise = NULL;
		obj->control = NULL;
		obj->control = ControlCrossbowBolt;
	}

	obj = &Objects[ID_STARGATE];
	if (obj->loaded)
	{
		obj->control = StargateControl;
		obj->collision = StargateCollision;
		obj->saveAnim = true;
		obj->saveFlags = true;
	}

	obj = &Objects[ID_SLICER_DICER];
	if (obj->loaded)
	{
		obj->initialise = InitialiseSlicerDicer;
		obj->control = SlicerDicerControl;
		obj->collision = BladeCollision;
		obj->saveFlags = true;
	}

	obj = &Objects[ID_SARCOPHAGUS];
	if (obj->loaded)
	{
		obj->control = AnimatingControl;
		obj->collision = SarcophagusCollision;
		obj->saveFlags = true;
		obj->saveAnim = true;
	}

	obj = &Objects[ID_LARA_DOUBLE];
	if (obj->loaded)
	{
		obj->initialise = InitialiseLaraDouble;
		obj->control = LaraDoubleControl;
		obj->collision = CreatureCollision;
		obj->shadowSize = 128;
		obj->hitPoints = 1000;
		obj->pivotLength = 50;
		obj->radius = 128;
		obj->intelligent = true;
		obj->savePosition = true;
		obj->saveHitpoints = true;
		obj->saveFlags = true;
		obj->saveAnim = true;
		obj->hitEffect = 3;
	}

	obj = &Objects[ID_KNIGHT_TEMPLAR];
	if (obj->loaded)
	{
		obj->initialise = InitialiseKnightTemplar;
		obj->control = KnightTemplarControl;
		obj->collision = CreatureCollision;
		obj->shadowSize = 128;
		obj->hitPoints = 15;
		obj->pivotLength = 50;
		obj->radius = 128;
		obj->intelligent = true;
		obj->savePosition = true;
		obj->saveHitpoints = true;
		obj->saveFlags = true;
		obj->saveAnim = true;

		Bones[obj->boneIndex + 6 * 4] |= ROT_X | ROT_Y;
		Bones[obj->boneIndex + 7 * 4] |= ROT_Y;
	}

	obj = &Objects[ID_DEMIGOD1];
	if (obj->loaded)
	{
		obj->initialise = InitialiseDemigod;
		obj->control = DemigodControl;
		obj->collision = CreatureCollision;
		obj->shadowSize = 128;
		obj->hitPoints = 200;
		obj->pivotLength = 50;
		obj->radius = 341;
		obj->intelligent = true;
		obj->savePosition = true;
		obj->saveHitpoints = true;
		obj->saveFlags = true;
		obj->saveAnim = true;
		obj->hitEffect = 3;
		obj->undead = true;

		Bones[obj->boneIndex + 4 * 4] |= ROT_X | ROT_Y | ROT_Z;
		Bones[obj->boneIndex + 5 * 4] |= ROT_Y;
	}

	obj = &Objects[ID_DEMIGOD2];
	if (obj->loaded)
	{
		obj->initialise = InitialiseDemigod;
		obj->control = DemigodControl;
		obj->collision = CreatureCollision;
		obj->shadowSize = 128;
		obj->hitPoints = 200;
		obj->pivotLength = 50;
		obj->radius = 341;
		obj->intelligent = true;
		obj->savePosition = true;
		obj->saveHitpoints = true;
		obj->saveFlags = true;
		obj->saveAnim = true;

		Bones[obj->boneIndex + 4 * 4] |= ROT_X | ROT_Y | ROT_Z;
		Bones[obj->boneIndex + 5 * 4] |= ROT_Y;
	}

	obj = &Objects[ID_DEMIGOD3];
	if (obj->loaded)
	{
		obj->initialise = InitialiseDemigod;
		obj->control = DemigodControl;
		obj->collision = CreatureCollision;
		obj->shadowSize = 128;
		obj->hitPoints = 200;
		obj->pivotLength = 50;
		obj->radius = 341;
		obj->intelligent = true;
		obj->savePosition = true;
		obj->saveHitpoints = true;
		obj->saveFlags = true;
		obj->saveAnim = true;

		Bones[obj->boneIndex + 4 * 4] |= ROT_X | ROT_Y | ROT_Z;
		Bones[obj->boneIndex + 5 * 4] |= ROT_Y;
	}

	obj = &Objects[ID_MINE];
	if (obj->loaded)
	{
		obj->initialise = InitialiseMine;
		obj->control = MineControl;
		obj->collision = MineCollision;
		obj->saveFlags = true;
		obj->saveAnim = true;
		obj->savePosition = true;
	}

	obj = &Objects[ID_JEAN_YVES];
	if (obj->loaded)
	{
		obj->initialise = InitialiseJeanYves;
		obj->control = JeanYvesControl;
		obj->collision = ObjectCollision;
		obj->nonLot = true;
		obj->savePosition = true;
	}

	obj = &Objects[ID_WATERSKIN1_EMPTY];
	if (obj->loaded)
	{
		obj->initialise = InitialisePickup;
		obj->control = PickupControl;
		obj->collision = PickupCollision;
		obj->saveFlags = true;
		obj->savePosition = true;
	}

	obj = &Objects[ID_WATERSKIN2_EMPTY];
	if (obj->loaded)
	{
		obj->initialise = InitialisePickup;
		obj->control = PickupControl;
		obj->collision = PickupCollision;
		obj->saveFlags = true;
		obj->savePosition = true;
	}

	obj = &Objects[ID_SCORPION];
	if (obj->loaded)
	{
		obj->initialise = InitialiseScorpion;
		obj->control = ScorpionControl;
		obj->collision = CreatureCollision;
		obj->shadowSize = 128;
		obj->hitPoints = 80;
		obj->pivotLength = 50;
		obj->radius = 512;
		obj->intelligent = true;
		obj->savePosition = true;
		obj->saveHitpoints = true;
		obj->saveFlags = true;
		obj->saveAnim = true;
		obj->hitEffect = 2;
	}

	obj = &Objects[ID_TROOPS];
	if (obj->loaded)
	{
		obj->initialise = InitialiseTroops;
		obj->control = TroopsControl;
		obj->collision = CreatureCollision;
		obj->shadowSize = 128;
		obj->hitPoints = 40;
		obj->pivotLength = 50;
		obj->radius = 102;
		obj->intelligent = true;
		obj->saveHitpoints = true;
		obj->saveAnim = true;
		obj->saveFlags = true;

		Bones[obj->boneIndex] |= ROT_X | ROT_Y;
		Bones[obj->boneIndex + 7 * 4] |= ROT_X | ROT_Y;
	}

	obj = &Objects[ID_SENTRY_GUN];
	if (obj->loaded)
	{
		obj->initialise = InitialiseSentryGun;
		obj->control = SentryGunControl;
		obj->collision = CreatureCollision;
		obj->shadowSize = 128;
		obj->hitPoints = 30;
		obj->pivotLength = 50;
		obj->radius = 204;
		obj->intelligent = true;
		obj->saveHitpoints = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->explodableMeshbits = 64;

		Bones[obj->boneIndex + 0] |= ROT_Y;
		Bones[obj->boneIndex + 1 * 4] |= ROT_X;
		Bones[obj->boneIndex + 2 * 4] |= ROT_Z;
		Bones[obj->boneIndex + 3 * 4] |= ROT_Z;
	}

	obj = &Objects[ID_HARPY];
	if (obj->loaded)
	{
		obj->initialise = InitialiseHarpy;
		obj->control = HarpyControl;
		obj->collision = CreatureCollision;
		obj->shadowSize = 128;
		obj->hitPoints = 60;
		obj->pivotLength = 50;
		obj->radius = 409;
		obj->intelligent = true;
		obj->saveHitpoints = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
	}

	obj = &Objects[ID_GUIDE];
	if (obj->loaded)
	{
		obj->initialise = InitialiseGuide;
		obj->control = GuideControl;
		obj->collision = CreatureCollision;
		obj->shadowSize = 128;
		obj->hitPoints = -16384;
		obj->pivotLength = 0;
		obj->radius = 128;
		obj->intelligent = true;
		obj->saveHitpoints = true;
		obj->saveAnim = true;
		obj->saveFlags = true;

		Bones[obj->boneIndex + 6 * 4] |= ROT_X | ROT_Y;
		Bones[obj->boneIndex + 20 * 4] |= ROT_X | ROT_Y;

		// TODO: check if constants are byte, __int16 or __int32
		Meshes[obj->meshIndex + 124] = Meshes[Objects[ID_MESHSWAP2].meshIndex + 120];
		Meshes[obj->meshIndex + 148] = Meshes[Objects[ID_MESHSWAP2].meshIndex + 144];
		Meshes[obj->meshIndex + 172] = Meshes[Objects[ID_MESHSWAP2].meshIndex + 168];
	}

	obj = &Objects[ID_CROCODILE];
	if (obj->loaded)
	{
		obj->initialise = InitialiseCrocodile;
		obj->control = CrocodileControl;
		obj->collision = CreatureCollision;
		obj->shadowSize = 128;
		obj->hitPoints = 36;
		obj->pivotLength = 300;
		obj->radius = 409;
		obj->intelligent = true;
		obj->saveHitpoints = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->waterCreature = true;

		Bones[obj->boneIndex] |= ROT_Y;
		Bones[obj->boneIndex + 7 * 4] |= ROT_Y;
		Bones[obj->boneIndex + 9 * 4] |= ROT_Y;
		Bones[obj->boneIndex + 10 * 4] |= ROT_Y;
	}

	obj = &Objects[ID_SPHINX];
	if (obj->loaded)
	{
		obj->initialise = InitialiseSphinx;
		obj->control = SphinxControl;
		obj->collision = CreatureCollision;
		obj->shadowSize = 128;
		obj->hitPoints = 1000;
		obj->pivotLength = 500;
		obj->radius = 512;
		obj->intelligent = true;
		obj->saveHitpoints = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
	}

	obj = &Objects[ID_FLAMETHROWER_BADDY];
	if (obj->loaded)
	{
		obj->control = FlameThrowerControl;
		obj->collision = CreatureCollision;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->hitPoints = 36;
		obj->radius = 102;
		obj->intelligent = true;
		obj->savePosition = true;
		obj->saveHitpoints = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->pivotLength = 0;

		Bones[obj->boneIndex + 0 * 4] |= ROT_Y;
		Bones[obj->boneIndex + 0 * 4] |= ROT_X;
		Bones[obj->boneIndex + 7 * 4] |= ROT_Y;
	}

	obj = &Objects[ID_SPIKY_WALL];
	if (obj->loaded)
	{
		obj->control = ControlSpikyWall;
		obj->collision = ObjectCollision;
		obj->savePosition = true;
		obj->saveFlags = true;
	}

	obj = &Objects[ID_SPIKY_CEILING];
	if (obj->loaded)
	{
		obj->control = ControlSpikyCeiling;
		obj->collision = TrapCollision;
		obj->savePosition = true;
		obj->saveFlags = true;
	}

	obj = &Objects[ID_MONKEY];
	if (obj->loaded)
	{
		//if (!Objects[MESHSWAP2].loaded)
		//	S_ExitSystem("FATAL: Monkey requires MESHSWAP2 (Monkey + Pickups)");

		//obj->draw_routine = (void*)DrawMonkey;
		obj->initialise = InitialiseMonkey;
		obj->control = MonkeyControl;
		obj->collision = CreatureCollision;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->hitPoints = 8;
		obj->radius = 102;
		obj->intelligent = true;
		obj->savePosition = true;
		obj->saveHitpoints = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->pivotLength = 0;

		Bones[obj->boneIndex + 0 * 4] |= ROT_Y; // index 0 (used in CreatureJoint "joint" argument)
		Bones[obj->boneIndex + 0 * 4] |= ROT_X; // index 1
		Bones[obj->boneIndex + 7 * 4] |= ROT_Y; // index 2
	}

	obj = &Objects[ID_JEEP];
	if (obj->loaded)
	{
		obj->initialise = InitialiseJeep;
		obj->collision = JeepCollision;
		obj->savePosition = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
	}

	// TODO: fix this
	obj = &Objects[ID_ENERGY_BUBBLES];
	obj->loaded = true;
	obj->control = BubblesControl;
	obj->nmeshes = 0;

	obj = &Objects[ID_MP_WITH_GUN];
	if (obj->loaded)
	{
		obj->control = MPGunControl;
		obj->collision = CreatureCollision;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->hitPoints = 28;
		obj->radius = 102;
		obj->intelligent = true;
		obj->savePosition = true; 
		obj->saveHitpoints = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->pivotLength = 0;
		obj->biteOffset = 0;

		Bones[obj->boneIndex + 6 * 4] |= ROT_Y;
		Bones[obj->boneIndex + 6 * 4] |= ROT_X;
		Bones[obj->boneIndex + 13 * 4] |= ROT_Y;
	}

	obj = &Objects[ID_MP_WITH_STICK];
	if (obj->loaded)
	{
		obj->initialise = InitialiseMPStick;
		obj->control = MPStickControl;
		obj->collision = CreatureCollision;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->hitPoints = 28;
		obj->radius = 102;
		obj->intelligent = true;
		obj->savePosition = true;
		obj->saveHitpoints = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->pivotLength = 0;
		Bones[obj->boneIndex + 6 * 4] |= ROT_Y;
		Bones[obj->boneIndex + 6 * 4] |= ROT_X;
		Bones[obj->boneIndex + 13 * 4] |= ROT_Y;
	}
}

// TODO: check for all the flags, some is surely missing.
void __cdecl BaddyObjects()
{
	OBJECT_INFO* obj;

	/* Initialise Lara directly since lara will be used all the time. */
	obj = &Objects[ID_LARA];
	obj->initialise = InitialiseLaraLoad;
	obj->shadowSize = 160;
	obj->hitPoints = 1000;
	obj->drawRoutine = NULL;
	obj->saveAnim = true;
	obj->saveFlags = true;
	obj->saveHitpoints = true;
	obj->savePosition = true;
	obj->usingDrawanimatingItem = false;

	obj = &Objects[ID_SAS];
	if (obj->loaded)
	{
		obj->biteOffset = 0;
		obj->initialise = InitialiseGuard;
		obj->collision = CreatureCollision;
		obj->control = ControlGuard;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->hitPoints = 40;
		obj->pivotLength = 50;
		obj->radius = 102;
		obj->intelligent = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->saveHitpoints = true;
		obj->savePosition = true;
		Bones[obj->boneIndex + 6 * 4] |= ROT_Y;
		Bones[obj->boneIndex + 6 * 4] |= ROT_X;
		Bones[obj->boneIndex + 13 * 4] |= ROT_Y;
		Bones[obj->boneIndex + 13 * 4] |= ROT_X;
	}
	
	obj = &Objects[ID_BLUE_GUARD];
	if (obj->loaded)
	{
		if (Objects[ID_SWAT].loaded) // object required
			obj->animIndex = Objects[ID_SWAT].animIndex;
		obj->biteOffset = 4;
		obj->initialise = InitialiseGuard;
		obj->collision = CreatureCollision;
		obj->control = ControlGuard;
		obj->drawRoutineExtra = DrawBaddieGunFlash;
		obj->pivotLength = 50;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->hitPoints = 24;
		obj->radius = 102;
		obj->explodableMeshbits = 0x4000;
		obj->intelligent = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->saveHitpoints = true;
		obj->savePosition = true;
		Bones[obj->boneIndex + 6 * 4] |= ROT_Y;
		Bones[obj->boneIndex + 6 * 4] |= ROT_X;
		Bones[obj->boneIndex + 13 * 4] |= ROT_Y;
		Bones[obj->boneIndex + 13 * 4] |= ROT_X;
		Meshes[obj->meshIndex + 10 * 2] = Meshes[Objects[ID_MESHSWAP1].meshIndex + 10 * 2];
		Meshes[obj->meshIndex + 13 * 2] = Meshes[Objects[ID_MESHSWAP1].meshIndex + 13 * 2];
	}

	obj = &Objects[ID_SWAT];
	if (obj->loaded)
	{
		obj->biteOffset = 0;
		obj->initialise = InitialiseGuard;
		obj->collision = CreatureCollision;
		obj->control = ControlGuard;
		obj->drawRoutineExtra = DrawBaddieGunFlash;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->hitPoints = 24;
		obj->pivotLength = 50;
		obj->radius = 102;
		obj->explodableMeshbits = 0x4000;
		obj->intelligent = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->saveHitpoints = true;
		obj->savePosition = true;
		Bones[obj->boneIndex + 6 * 4] |= ROT_Y;
		Bones[obj->boneIndex + 6 * 4] |= ROT_X;
		Bones[obj->boneIndex + 13 * 4] |= ROT_Y;
		Bones[obj->boneIndex + 13 * 4] |= ROT_X;
		Meshes[obj->meshIndex + 10 * 2] = Meshes[Objects[ID_MESHSWAP1].meshIndex + 10 * 2];
		Meshes[obj->meshIndex + 13 * 2] = Meshes[Objects[ID_MESHSWAP1].meshIndex + 13 * 2];
	}
	
	obj = &Objects[ID_SWAT_PLUS];
	if (obj->loaded)
	{
		__int16 animIndex;
		if (!Objects[ID_SWAT].loaded)
			animIndex = Objects[ID_BLUE_GUARD].animIndex;
		else
			animIndex = Objects[ID_SWAT].animIndex;
		obj->animIndex = animIndex;
		obj->biteOffset = 0;
		obj->initialise = InitialiseGuard;
		obj->collision = CreatureCollision;
		obj->control = ControlGuard;
		obj->drawRoutineExtra = DrawBaddieGunFlash;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->hitPoints = 24;
		obj->pivotLength = 50;
		obj->radius = 102;
		obj->intelligent = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->saveHitpoints = true;
		obj->savePosition = true;
		Bones[obj->boneIndex + 6 * 4] |= ROT_Y;
		Bones[obj->boneIndex + 6 * 4] |= ROT_X;
		Bones[obj->boneIndex + 13 * 4] |= ROT_Y;
		Bones[obj->boneIndex + 13 * 4] |= ROT_X;
		Meshes[obj->meshIndex + 10 * 2] = Meshes[Objects[ID_MESHSWAP1].meshIndex + 10 * 2];
		Meshes[obj->meshIndex + 13 * 2] = Meshes[Objects[ID_MESHSWAP1].meshIndex + 13 * 2];
	}

	obj = &Objects[ID_MAFIA];
	if (obj->loaded)
	{
		__int16 animIndex;
		if (!Objects[ID_SWAT].loaded)
			animIndex = Objects[ID_BLUE_GUARD].animIndex;
		else
			animIndex = Objects[ID_SWAT].animIndex;
		obj->animIndex = animIndex;
		obj->biteOffset = 0;
		obj->initialise = InitialiseGuard;
		obj->collision = CreatureCollision;
		obj->control = ControlGuard;
		obj->drawRoutineExtra = DrawBaddieGunFlash;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->hitPoints = 24;
		obj->pivotLength = 50;
		obj->radius = 102;
		obj->explodableMeshbits = 0x4000;
		obj->intelligent = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->saveHitpoints = true;
		obj->savePosition = true;
		Bones[obj->boneIndex + 6 * 4] |= ROT_Y;
		Bones[obj->boneIndex + 6 * 4] |= ROT_X;
		Bones[obj->boneIndex + 13 * 4] |= ROT_Y;
		Bones[obj->boneIndex + 13 * 4] |= ROT_X;
		Meshes[obj->meshIndex + 10 * 2] = Meshes[Objects[ID_MESHSWAP1].meshIndex + 10 * 2];
		Meshes[obj->meshIndex + 13 * 2] = Meshes[Objects[ID_MESHSWAP1].meshIndex + 13 * 2];
	}

	obj = &Objects[ID_SCIENTIST];
	if (obj->loaded)
	{
		__int16 animIndex;
		if (!Objects[ID_SWAT].loaded)
			animIndex = Objects[ID_BLUE_GUARD].animIndex;
		else
			animIndex = Objects[ID_SWAT].animIndex;
		obj->animIndex = animIndex;
		obj->initialise = InitialiseGuard;
		obj->control = ControlGuard;
		obj->collision = CreatureCollision;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->hitPoints = 24;
		obj->pivotLength = 50;
		obj->radius = 102;
		obj->intelligent = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->saveHitpoints = true;
		obj->savePosition = true;
		Bones[Objects[69].boneIndex + 6 * 4] |= ROT_Y;
		Bones[Objects[69].boneIndex + 6 * 4] |= ROT_X;
		Bones[Objects[69].boneIndex + 13 * 4] |= ROT_Y;
		Bones[Objects[69].boneIndex + 13 * 4] |= ROT_X;
		Meshes[Objects[69].meshIndex + 10 * 2] = Meshes[Objects[ID_MESHSWAP1].meshIndex + 10 * 2];
		Meshes[Objects[69].meshIndex + 13 * 2] = Meshes[Objects[ID_MESHSWAP1].meshIndex + 13 * 2];
	}

	obj = &Objects[ID_CRANE_GUY];
	if (obj->loaded)
	{
		__int16 animIndex;
		if (!Objects[ID_SWAT].loaded)
			animIndex = Objects[ID_BLUE_GUARD].animIndex;
		else
			animIndex = Objects[ID_SWAT].animIndex;
		obj->animIndex = animIndex;
		obj->biteOffset = 4;
		obj->initialise = InitialiseGuard;
		obj->control = ControlGuard;
		obj->collision = CreatureCollision;
		obj->drawRoutineExtra = DrawBaddieGunFlash;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->hitPoints = 24;
		obj->pivotLength = 50;
		obj->radius = 102;
		obj->explodableMeshbits = 0x4000;
		obj->intelligent = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->saveHitpoints = true;
		obj->savePosition = true;
		Bones[obj->boneIndex + 6 * 4] |= ROT_Y;
		Bones[obj->boneIndex + 6 * 4] |= ROT_X;
		Bones[obj->boneIndex + 13 * 4] |= ROT_Y;
		Bones[obj->boneIndex + 13 * 4] |= ROT_X;
		Meshes[obj->meshIndex + 10 * 2] = Meshes[Objects[405].meshIndex + 10 * 2];
		Meshes[obj->meshIndex + 13 * 2] = Meshes[Objects[405].meshIndex + 13 * 2];
	}

	obj = &Objects[ID_SAILOR];
	if (obj->loaded)
	{
		__int16 animIndex;
		if (!Objects[ID_SWAT].loaded)
			animIndex = Objects[ID_BLUE_GUARD].animIndex;
		else
			animIndex = Objects[ID_SWAT].animIndex;
		obj->animIndex = animIndex;
		obj->initialise = InitialiseGuard;
		obj->collision = CreatureCollision;
		obj->control = ControlGuard;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->hitPoints = 24;
		obj->pivotLength = 50;
		obj->radius = 102;
		obj->intelligent = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->saveHitpoints = true;
		obj->savePosition = true;
		Bones[obj->boneIndex + 6 * 4] |= ROT_Y;
		Bones[obj->boneIndex + 6 * 4] |= ROT_X;
		Bones[obj->boneIndex + 13 * 4] |= ROT_Y;
		Bones[obj->boneIndex + 13 * 4] |= ROT_X;
	}

	obj = &Objects[ID_ATTACK_SUB];
	if (obj->loaded)
	{
		obj->initialise = InitialiseSubmarine;
		obj->collision = CreatureCollision;
		obj->control = ControlSubmarine;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->hitPoints = 100;
		obj->pivotLength = 200;
		obj->radius = 512;
		obj->intelligent = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->saveHitpoints = true;
		obj->savePosition = true;
		obj->waterCreature = true;
		Bones[obj->boneIndex] |= ROT_X;
		Bones[obj->boneIndex + 4] |= ROT_X;
	}
	
	obj = &Objects[ID_LION];
	if (obj->loaded)
	{
		obj->initialise = InitialiseLion;
		obj->collision = CreatureCollision;
		obj->control = LionControl;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->hitPoints = 40;
		obj->pivotLength = 50;
		obj->radius = 341;
		obj->intelligent = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->saveHitpoints = true;
		obj->savePosition = true;
		Bones[obj->boneIndex + 6 * 4] |= ROT_Y;
		Bones[obj->boneIndex + 19 * 4] |= ROT_Y;
	}

	obj = &Objects[ID_DOG];
	if (obj->loaded)
	{
		obj->initialise = InitialiseDoberman;
		obj->collision = CreatureCollision;
		obj->control = ControlDoberman;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->hitPoints = 18;
		obj->pivotLength = 50;
		obj->radius = 256;
		obj->intelligent = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->saveHitpoints = true;
		obj->savePosition = true;
		Bones[obj->boneIndex + 19 * 4] |= ROT_Y;
	}

	obj = &Objects[ID_HUSKIE];
	if (obj->loaded)
	{
		obj->initialise = InitialiseDog;
		obj->collision = CreatureCollision;
		obj->control = ControlDog;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->hitPoints = 24;
		obj->pivotLength = 50;
		obj->radius = 256;
		obj->intelligent = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->saveHitpoints = true;
		obj->savePosition = true;
		Bones[obj->boneIndex + 19 * 4] |= ROT_Y;
	}

	obj = &Objects[ID_REAPER];
	
	if (obj->loaded)
	{
		obj->initialise = InitialiseReaper;
		obj->collision = CreatureCollision;
		obj->control = ControlReaper;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->hitPoints = 10;
		obj->pivotLength = 50;
		obj->radius = 102;
		obj->intelligent = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->saveHitpoints = true;
		obj->savePosition = true;
	}

	obj = &Objects[ID_MAFIA2];
	if (obj->loaded)
	{
		obj->biteOffset = 7;
		obj->initialise = InitialiseArmedBaddy2;
		obj->collision = CreatureCollision;
		obj->control = ControlArmedBaddy2;
		obj->drawRoutineExtra = DrawBaddieGunFlash;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->hitPoints = 26;
		obj->pivotLength = 50;
		obj->radius = 102;
		obj->explodableMeshbits = 0x4000;
		obj->intelligent = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->saveHitpoints = true;
		obj->savePosition = true;
		Bones[obj->boneIndex + 6 * 4] |= ROT_Y;
		Bones[obj->boneIndex + 6 * 4] |= ROT_X;
		Bones[obj->boneIndex + 13 * 4] |= ROT_Y;
		Bones[obj->boneIndex + 13 * 4] |= ROT_X;
		Meshes[obj->meshIndex + 7 * 2] = Meshes[Objects[ID_MESHSWAP2].meshIndex + 7 * 2];
		Meshes[obj->meshIndex + 10 * 2] = Meshes[Objects[ID_MESHSWAP2].meshIndex + 10 * 2];
		Meshes[obj->meshIndex + 13 * 2] = Meshes[Objects[ID_MESHSWAP2].meshIndex + 13 * 2];
	}

	obj = &Objects[ID_PIERRE];
	if (obj->loaded)
	{
		obj->biteOffset = 1;
		obj->initialise = InitialiseLarson;
		obj->collision = CreatureCollision;
		obj->control = ControlLarson;
		obj->drawRoutineExtra = DrawBaddieGunFlash;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->hitPoints = 60;
		obj->pivotLength = 50;
		obj->radius = 102;
		obj->intelligent = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->saveHitpoints = true;
		obj->savePosition = true;
		Bones[obj->boneIndex + 6 * 4] |= ROT_Y;
		Bones[obj->boneIndex + 6 * 4] |= ROT_X;
		Bones[obj->boneIndex + 7 * 4] |= ROT_Y;
		Bones[obj->boneIndex + 7 * 4] |= ROT_X;
	}

	obj = &Objects[ID_LARSON];
	if (obj->loaded)
	{
		obj->biteOffset = 3;
		obj->initialise = InitialiseLarson;
		obj->collision = CreatureCollision;
		obj->control = ControlLarson;
		obj->drawRoutineExtra = DrawBaddieGunFlash;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->hitPoints = 60;
		obj->pivotLength = 50;
		obj->radius = 102;
		obj->intelligent = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->saveHitpoints = true;
		obj->savePosition = true;
		Bones[obj->boneIndex + 6 * 4] |= ROT_Y;
		Bones[obj->boneIndex + 6 * 4] |= ROT_X;
		Bones[obj->boneIndex + 7 * 4] |= ROT_Y;
		Bones[obj->boneIndex + 7 * 4] |= ROT_X;
	}

	obj = &Objects[ID_HITMAN];
	if (obj->loaded)
	{
		obj->biteOffset = 5;
		obj->initialise = InitialiseCyborg;
		obj->collision = CreatureCollision;
		obj->control = ControlCyborg;
		obj->drawRoutineExtra = DrawBaddieGunFlash;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->hitPoints = 50;
		obj->pivotLength = 50;
		obj->radius = 102;
		obj->intelligent = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->saveHitpoints = true;
		obj->savePosition = true;
		//HIBYTE(v23) = *(&Objects[ID_HITMAN] + 51) | 0x1C;
		Bones[obj->boneIndex + 6 * 4] |= ROT_Y;
		Bones[obj->boneIndex + 6 * 4] |= ROT_X;
		Bones[obj->boneIndex + 13 * 4] |= ROT_Y;
		Bones[obj->boneIndex + 13 * 4] |= ROT_X;
		/*
		v44 = 20;(__int16)
		v24 = 0;(__int16)
		v41 = 0;(__int16)
		do
		{
			(&meshes[v24 + 1])[Objects[ID_HITMAN].mesh_index] = meshes[v24 + Objects[ID_MESHSWAP1].mesh_index];
			v24 = v41 + 2;
			v25 = v44 == 1;
			v41 += 2;
			--v44;
		} while (!v25);
		*/

		for (__int32 i = (obj->nmeshes - 1); i > 0; i--)
		{
			Meshes[obj->meshIndex + i * 2] = Meshes[Objects[ID_MESHSWAP1].meshIndex + i * 2];
		}
	}

	obj = &Objects[ID_SNIPER];
	if (obj->loaded)
	{
		obj->biteOffset = 6;
		obj->initialise = InitialiseGuardM16;
		obj->collision = CreatureCollision;
		obj->control = ControlGuardM16;
		obj->drawRoutineExtra = DrawBaddieGunFlash;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->hitPoints = 35;
		obj->pivotLength = 50;
		obj->radius = 102;
		obj->explodableMeshbits = 0x4000;
		obj->intelligent = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->saveHitpoints = true;
		obj->savePosition = true;
		Bones[obj->boneIndex + 6 * 4] |= ROT_Y;
		Bones[obj->boneIndex + 6 * 4] |= ROT_X;
		Bones[obj->boneIndex + 13 * 4] |= ROT_Y;
		Bones[obj->boneIndex + 13 * 4] |= ROT_X;
	}

	obj = &Objects[ID_SAS_CAIRO];
	if (obj->loaded)
	{
		obj->biteOffset = 0;
		obj->initialise = InitialiseChef;
		obj->collision = CreatureCollision;
		obj->control = ControlChef;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->hitPoints = 35;
		obj->pivotLength = 50;
		obj->radius = 102;
		obj->intelligent = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->saveHitpoints = true;
		obj->savePosition = true;
		Bones[obj->boneIndex + 6 * 4] |= ROT_Y;
		Bones[obj->boneIndex + 6 * 4] |= ROT_X;
		Bones[obj->boneIndex + 13 * 4] |= ROT_Y;
		Bones[obj->boneIndex + 13 * 4] |= ROT_X;
	}
	
	obj = &Objects[ID_TWOGUN];
	if (obj->loaded)
	{
		obj->biteOffset = 0;
		obj->initialise = InitialiseGuardLaser;
		obj->collision = CreatureCollision;
		obj->control = ControlGuardLaser;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->hitPoints = 24;
		obj->pivotLength = 50;
		obj->radius = 128;
		obj->explodableMeshbits = 4;
		obj->intelligent = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->saveHitpoints = true;
		obj->savePosition = true;
		Bones[obj->boneIndex] |= ROT_Y;
		Bones[obj->boneIndex] |= ROT_X;
		Bones[obj->boneIndex + 4] |= ROT_Y;
		Bones[obj->boneIndex + 4] |= ROT_X;
		Meshes[obj->meshIndex + 10 * 2] = Meshes[Objects[ID_MESHSWAP1].meshIndex + 2 * 2];
	}
	
	obj = &Objects[ID_HYDRA];
	if (obj->loaded)
	{
		obj->initialise = InitialiseSmallDragon;
		obj->collision = CreatureCollision;
		obj->control = ControlSmallDragon;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->hitPoints = 30;
		obj->pivotLength = 50;
		obj->radius = 102;
		obj->biteOffset = 1024;
		obj->intelligent = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->saveHitpoints = true;
		obj->savePosition = true;
		Bones[obj->boneIndex + 0] |= ROT_Y;
		Bones[obj->boneIndex + 8 * 4] |= ROT_Y;
		Bones[obj->boneIndex + 8 * 4] |= ROT_X;
		Bones[obj->boneIndex + 8 * 4] |= ROT_Z;
	}
	
	obj = &Objects[ID_IMP];
	if (obj->loaded)
	{
		obj->biteOffset = 256;
		obj->initialise = InitialiseImp;
		obj->collision = CreatureCollision;
		obj->control = ControlImp;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->hitPoints = 12;
		obj->pivotLength = 20;
		obj->radius = 102;
		obj->intelligent = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->saveHitpoints = true;
		obj->savePosition = true;
		Bones[obj->meshIndex + 4 * 4] |= ROT_Z;
		Bones[obj->meshIndex + 4 * 4] |= ROT_X;
		Bones[obj->meshIndex + 9 * 4] |= ROT_Z;
		Bones[obj->meshIndex + 9 * 4] |= ROT_X;
		Meshes[obj->meshIndex + 10 * 2] = Meshes[Objects[ID_MESHSWAP1].meshIndex + 10 * 2];
	}
	
	obj = &Objects[ID_WILLOWISP];
	if (obj->loaded)
	{
		obj->biteOffset = 256;
		obj->initialise = InitialiseLightingGuide;
		obj->control = ControlLightingGuide;
		obj->drawRoutine = NULL;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->radius = 256;
		obj->hitPoints = 16;
		obj->pivotLength = 20;
		obj->intelligent = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->saveHitpoints = true;
		obj->savePosition = true;
		Bones[obj->boneIndex + 4 * 4] |= ROT_Z;
		Bones[obj->boneIndex + 4 * 4] |= ROT_X;
		Bones[obj->boneIndex + 9 * 4] |= ROT_Z;
		Bones[obj->boneIndex + 9 * 4] |= ROT_X;
	}
	
	obj = &Objects[ID_MAZE_MONSTER];
	if (obj->loaded)
	{
		obj->biteOffset = 256;
		obj->initialise = InitialiseBrownBeast;
		obj->collision = CreatureCollision;
		obj->control = ControlBrowsBeast;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->hitPoints = 4000;
		obj->pivotLength = 20;
		obj->radius = 341;
		obj->intelligent = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->saveHitpoints = true;
		obj->savePosition = true;
		Bones[obj->boneIndex + 4 * 4] |= ROT_Z;
		Bones[obj->boneIndex + 4 * 4] |= ROT_X;
		Bones[obj->boneIndex + 9 * 4] |= ROT_Z;
		Bones[obj->boneIndex + 9 * 4] |= ROT_X;
	}
	
	obj = &Objects[ID_GREEN_TEETH];
	if (obj->loaded)
	{
		//v33 = (*(&Objects[79] + 25) | 2) & 0xF7FF;
		//HIBYTE(v33) |= 4u;
		//v33 |= 0x100u;
		//LOBYTE(v33) = v33 | 0x70;
		//*(&Objects[79] + 25) = v33 | 8;
		obj->biteOffset = 256;
		obj->initialise = InitialiseLagoonWitch;
		obj->collision = CreatureCollision;
		obj->control = ControlLagoonWitch;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->hitPoints = 100;
		obj->pivotLength = 20;
		obj->radius = 256;
		obj->intelligent = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->saveHitpoints = true;
		obj->savePosition = true;
		Bones[obj->boneIndex + 4 * 4] |= ROT_Z;
		Bones[obj->boneIndex + 4 * 4] |= ROT_X;
		Bones[obj->boneIndex + 9 * 4] |= ROT_Z;
		Bones[obj->boneIndex + 9 * 4] |= ROT_X;
	}

	obj = &Objects[ID_INVISIBLE_GHOST];
	if (obj->loaded)
	{
		obj->biteOffset = 256;
		obj->initialise = InitialiseInvisibleGhost;
		obj->collision = CreatureCollision;
		obj->control = ControlInvisibleGhost;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->hitPoints = 100;
		obj->pivotLength = 20;
		obj->radius = 256;
		obj->intelligent = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->saveHitpoints = true;
		obj->savePosition = true;
		Bones[obj->boneIndex + 6 * 4] |= ROT_Y;
		Bones[obj->boneIndex + 6 * 4] |= ROT_X;
		Bones[obj->boneIndex + 8 * 4] |= ROT_Y;
		Bones[obj->boneIndex + 8 * 4] |= ROT_X;
	}
	
	obj = &Objects[ID_RATS];
	if (obj->loaded)
	{
		//*(&Objects[94] + 25) &= 0xFDFFu;
		obj->drawRoutine = NULL;
		obj->initialise = InitialiseLittleRats;
		obj->control = ControlLittleRats;
	}

	obj = &Objects[ID_BATS];
	if (obj->loaded)
	{
		//*(&Objects[93] + 25) &= 0xFDFFu;
		obj->drawRoutine = NULL;
		obj->initialise = InitialiseLittleBats;
		obj->control = ControlLittleBats;
	}

	obj = &Objects[ID_SPIDER];
	if (obj->loaded)
	{
		//*(&Objects[95] + 25) &= 0xFDFFu;
		obj->drawRoutine = NULL;
		obj->initialise = InitialiseSpiders;
		obj->control = ControlSpiders;
	}
	
	obj = &Objects[ID_GLADIATOR];
	if (obj->loaded)
	{
		obj->biteOffset = 0;
		obj->initialise = InitialiseGladiator;
		obj->control = ControlGladiator;
		obj->collision = CreatureCollision;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->hitPoints = 20;
		obj->pivotLength = 50;
		obj->radius = 102;
		obj->intelligent = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->saveHitpoints = true;
		obj->savePosition = true;
		Bones[obj->boneIndex + 6 * 4] |= ROT_Y;
		Bones[obj->boneIndex + 6 * 4] |= ROT_X;
		Bones[obj->boneIndex + 13 * 4] |= ROT_Y;
		Bones[obj->boneIndex + 13 * 4] |= ROT_X;

		for (__int32 i = (obj->nmeshes - 1); i > 0; i--)
		{
			Meshes[obj->meshIndex + i * 2] = Meshes[Objects[ID_MESHSWAP1].meshIndex + i * 2];
		}
		/*
		do
		{
			(&Meshes[v36 + 1])[obj->meshIndex] = Meshes[v36 + Objects[ID_MESHSWAP1].meshIndex];
			v36 = v45 + 2;
			v25 = v42 == 1;
			v45 += 2;
			--v42;
		} while (!v25);*/
	}

	obj = &Objects[ID_ROMAN_GOD];
	if (obj->loaded)
	{
		obj->biteOffset = 0;
		obj->initialise = InitialiseRomanStatue;
		obj->collision = CreatureCollision;
		obj->control = ControlRomanStatue;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->hitPoints = 300;
		obj->pivotLength = 50;
		obj->radius = 256;
		obj->intelligent = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->saveHitpoints = true;
		obj->savePosition = true;
		Bones[obj->boneIndex + 24] |= ROT_Y;
		Bones[obj->boneIndex + 24] |= ROT_X;
		Bones[obj->boneIndex + 52] |= ROT_Y;
		Bones[obj->boneIndex + 52] |= ROT_X;

		for (__int32 i = (obj->nmeshes - 1); i > 0; i--)
		{
			Meshes[obj->meshIndex + i * 2] = Meshes[Objects[ID_MESHSWAP1].meshIndex + i * 2];
		}
		/*
		do
		{
			(&meshes[v38 + 1])[Objects[61].mesh_index] = meshes[v38 + Objects[405].mesh_index];
			v38 = v46 + 2;
			v25 = v43 == 1;
			v46 += 2;
			--v43;
		} while (!v25);
		*/
	}

	obj = &Objects[ID_GUARDIAN];
	if (obj->loaded)
	{
		//HIBYTE(v39) = ((*(&Objects[65] + 25) | 0xC08) >> 8) | 0x20;
		//LOBYTE(v39) = *(&Objects[65] + 50) | 0x78;
		//*(&Objects[65] + 25) = v39;
		obj->initialise = InitialiseLaserHead;
		obj->collision = CreatureCollision;
		obj->control = ControlLaserHead;
		obj->explodableMeshbits = 6;
		obj->intelligent = false;
		obj->nonLot = true;
		obj->savePosition = false;
		obj->saveHitpoints = false;
		obj->saveFlags = true;
		obj->saveAnim = true;
		obj->usingDrawanimatingItem = false;
		obj->hitEffect = 0;
		obj->undead = true; // ??
	}

	obj = &Objects[ID_AUTOGUN];
	if (obj->loaded)
	{
		//HIBYTE(v40) = *(&Objects[97] + 51) | 0xC;
		//LOBYTE(v40) = *(&Objects[97] + 50) | 0x70;
		//*(&Objects[97] + 25) = v40;
		obj->initialise = InitialiseAutoGuns;
		obj->control = ControlAutoGuns;
		obj->intelligent = true;
		obj->nonLot = false;
		obj->savePosition = false;
		obj->saveHitpoints = false;
		obj->saveFlags = true;
		obj->saveAnim = true;
		obj->usingDrawanimatingItem = false;
		obj->hitEffect = 1;
		Bones[obj->boneIndex + 6 * 4] |= ROT_Y;
		Bones[obj->boneIndex + 6 * 4] |= ROT_X;
		Bones[obj->boneIndex + 8 * 4] |= ROT_Y;
	}

	obj = &Objects[ID_GUNSHIP];
	if (obj->loaded)
	{
		// *(&Objects[91] + 25) |= 0x60u;
		obj->control = ControlGunShip;
		Bones[obj->boneIndex + 0] |= ROT_Y;
		Bones[obj->boneIndex + 4] |= ROT_X;
	}
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

	// Standard TR5 Objects
	BaddyObjects();
	ObjectObjects();
	TrapObjects();

	// Reset MIP flag so we can reuse slots
	for (__int16 i = 0; i < NUM_OBJECTS; i++)
		Objects[i].objectMip = 0;
		
	// New Objects imported from old TRs
	NewObjects();

	// User defined Objects
	CustomObjects();

	InitialiseHairs();
	InitialiseSpecialEffects();

	OldPickupPos.roomNumber = 0;
	
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

	if (Objects[ID_BATS].loaded)
		Bats = (BAT_STRUCT*)GameMalloc(NUM_BATS * sizeof(BAT_STRUCT));

	if (Objects[ID_SPIDER].loaded)
		Spiders = (SPIDER_STRUCT*)GameMalloc(NUM_SPIDERS * sizeof(SPIDER_STRUCT));

	if (Objects[ID_RATS].loaded)
		Rats = (RAT_STRUCT*)GameMalloc(NUM_RATS * sizeof(RAT_STRUCT));
}

void Inject_Setup()
{
	INJECT(0x00473600, InitialiseObjects);
	INJECT(0x004737C0, BaddyObjects);
}