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
		//if (!objects[MESHSWAP2].loaded)
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

		Bones[obj->boneIndex + 0 * 4] |= ROT_Z;
		Bones[obj->boneIndex + 7 * 4] |= ROT_X;
		Bones[obj->boneIndex + 7 * 4] |= ROT_Y;
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
}