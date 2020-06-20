#include "framework.h"
#include "tr4_objects.h"
/// entities
#include "tr4_ahmet.h" // OK
#include "tr4_baddy.h" // OK
#include "tr4_bat.h" // OK
#include "tr4_bigscorpion.h" // OK
#include "tr4_crocodile.h" // OK
#include "tr4_demigod.h" // OK
#include "tr4_guide.h" // OK
#include "tr4_harpy.h" // OK
#include "tr4_horseman.h" // OFF
#include "tr4_jeanyves.h" // OK
#include "tr4_knighttemplar.h" // OK
#include "tr4_littlebeetle.h"
#include "tr4_mummy.h" // OK
#include "tr4_sas.h" // OK
#include "tr4_sentrygun.h" // OK
#include "tr4_skeleton.h" // OK
#include "tr4_smallscorpion.h" // OK
#include "tr4_sphinx.h" // OK
#include "tr4_troops.h" // OK
#include "tr4_wildboar.h" // OK
#include "tr4_wraith.h" // OFF
#include "tr4_baboon.h" // OK
#include "tr4_mutant.h" // OK
#include "tr4_locusts.h" // OK
/// objects
#include "tr4_sarcophagus.h"
/// puzzle
#include "tr4_scales.h"
/// switch

/// traps
#include "tr4_birdblade.h"
#include "tr4_blade.h"
#include "tr4_catwalkblade.h"
#include "tr4_chain.h"
#include "tr4_fourblades.h"
#include "tr4_mine.h"
#include "tr4_plinthblade.h"
#include "tr4_plough.h"
#include "tr4_sethblade.h"
#include "tr4_slicerdicer.h"
#include "tr4_spikeball.h"
#include "tr4_spikywall.h"
#include "tr4_spikyceiling.h"
#include "tr4_stargate.h"
#include "tr4_cog.h"
#include "tr4_laradouble.h"
/// vehicles
#include "motorbike.h"
#include "jeep.h"
/// necessary import
#include "collide.h"
#include "objects.h"
#include "setup.h"
#include "level.h"

static void StartBaddy(ObjectInfo* obj)
{
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
		obj->zoneType = ZONE_BASIC;
	}

	obj = &Objects[ID_BIG_SCORPION];
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
		obj->zoneType = ZONE_BASIC;
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
		obj->zoneType = ZONE_BASIC;

		Bones[obj->boneIndex + 48 * 4] |= ROT_Z;
		Bones[obj->boneIndex + 48 * 4] |= ROT_Y;
		Bones[obj->boneIndex + 52 * 4] |= ROT_Z;
		Bones[obj->boneIndex + 52 * 4] |= ROT_Y;
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
		obj->zoneType = ZONE_FLYER;
	}

	obj = &Objects[ID_AHMET];
	if (obj->loaded)
	{
		obj->initialise = InitialiseAhmet;
		obj->control = AhmetControl;
		obj->collision = CreatureCollision;
		obj->shadowSize = 128;
		obj->hitPoints = 80;
		obj->pivotLength = 300;
		obj->radius = 341;
		obj->intelligent = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->saveHitpoints = true;
		obj->savePosition = true;
		obj->hitEffect = HIT_BLOOD;
		obj->zoneType = ZONE_BASIC;

		Bones[obj->boneIndex + 9 * 4] |= ROT_Y;
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
		obj->meshSwapSlot = ID_MESHSWAP_BADDY1;
		obj->zoneType = ZONE_HUMAN_JUMP_AND_MONKEY;

		Bones[obj->boneIndex + 28 * 4] |= ROT_Y;
		Bones[obj->boneIndex + 28 * 4] |= ROT_X;
		Bones[obj->boneIndex + 88 * 4] |= ROT_Y;
		Bones[obj->boneIndex + 88 * 4] |= ROT_X;
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
		obj->meshSwapSlot = ID_MESHSWAP_BADDY2;
		obj->zoneType = ZONE_HUMAN_JUMP_AND_MONKEY;

		Bones[obj->boneIndex + 28 * 4] |= ROT_Y;
		Bones[obj->boneIndex + 28 * 4] |= ROT_X;
		Bones[obj->boneIndex + 88 * 4] |= ROT_Y;
		Bones[obj->boneIndex + 88 * 4] |= ROT_X;
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
		obj->zoneType = ZONE_HUMAN_CLASSIC;

		Bones[obj->boneIndex] |= ROT_Y;
		Bones[obj->boneIndex] |= ROT_X;
		Bones[obj->boneIndex + 28 * 4] |= ROT_Y;
		Bones[obj->boneIndex + 28 * 4] |= ROT_X;
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
		obj->zoneType = ZONE_BASIC;

		Bones[obj->boneIndex + 28 * 4] |= ROT_Y;
		Bones[obj->boneIndex + 28 * 4] |= ROT_X;
		Bones[obj->boneIndex + 72 * 4] |= ROT_Y;
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
		obj->zoneType = ZONE_BASIC;
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
		obj->zoneType = ZONE_BASIC;

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
		obj->hitEffect = HIT_FRAGMENT;
		obj->undead = true;
		obj->zoneType = ZONE_BASIC;

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
		obj->hitEffect = HIT_FRAGMENT;
		obj->zoneType = ZONE_BASIC;
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
		obj->hitEffect = HIT_FRAGMENT;
		obj->zoneType = ZONE_BASIC;
		Bones[obj->boneIndex + 4 * 4] |= ROT_X | ROT_Y | ROT_Z;
		Bones[obj->boneIndex + 5 * 4] |= ROT_Y;
	}

	obj = &Objects[ID_JEAN_YVES];
	if (obj->loaded)
	{
		obj->initialise = InitialiseJeanYves;
		obj->control = JeanYvesControl;
		obj->collision = ObjectCollision;
		obj->nonLot = true;
		obj->savePosition = true;
		obj->zoneType = ZONE_BASIC;
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
		obj->zoneType = ZONE_BASIC;

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
		obj->zoneType = ZONE_BASIC;

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
		obj->zoneType = ZONE_FLYER;
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
		obj->zoneType = ZONE_HUMAN_CLASSIC;

		Bones[obj->boneIndex + 6 * 4] |= ROT_X | ROT_Y;
		Bones[obj->boneIndex + 20 * 4] |= ROT_X | ROT_Y;
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
		obj->zoneType = ZONE_WATER;

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
		obj->zoneType = ZONE_BASIC;
	}

	obj = &Objects[ID_BABOON_NORMAL];
	if (obj->loaded)
	{
		obj->initialise = InitialiseBaboon;
		obj->control = BaboonControl;
		obj->collision = CreatureCollision;
		obj->shadowSize = 128;
		obj->hitPoints = 30;
		obj->pivotLength = 200;
		obj->radius = 256;
		obj->intelligent = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->saveHitpoints = true;
		obj->savePosition = true;
		obj->hitEffect = HIT_BLOOD;
		obj->zoneType = ZONE_BASIC;
	}

	obj = &Objects[ID_BABOON_INV];
	if (obj->loaded)
	{
		obj->initialise = InitialiseBaboon;
		obj->control = BaboonControl;
		obj->collision = CreatureCollision;
		obj->shadowSize = 128;
		obj->hitPoints = 30;
		obj->pivotLength = 200;
		obj->radius = 256;
		obj->intelligent = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->saveHitpoints = true;
		obj->savePosition = true;
		obj->hitEffect = HIT_BLOOD;
		obj->zoneType = ZONE_BASIC;

		if (Objects[ID_BABOON_NORMAL].loaded)
			Objects[ID_BABOON_INV].animIndex = Objects[ID_BABOON_NORMAL].animIndex;
	}

	obj = &Objects[ID_BABOON_SILENT];
	if (obj->loaded)
	{
		obj->initialise = InitialiseBaboon;
		obj->control = BaboonControl;
		obj->collision = CreatureCollision;
		obj->shadowSize = 128;
		obj->hitPoints = 30;
		obj->pivotLength = 200;
		obj->radius = 256;
		obj->intelligent = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->saveHitpoints = true;
		obj->savePosition = true;
		obj->hitEffect = HIT_BLOOD;
		obj->zoneType = ZONE_BASIC;

		if (Objects[ID_BABOON_NORMAL].loaded)
			Objects[ID_BABOON_SILENT].animIndex = Objects[ID_BABOON_NORMAL].animIndex;
	}

	obj = &Objects[ID_CROCODILE_GOD];
	if (obj->loaded)
	{
		obj->initialise = InitialiseMutant;
		obj->control = MutantControl;
		obj->collision = CreatureCollision;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->hitPoints = NOT_TARGETABLE;
		obj->pivotLength = 50;
		obj->radius = 128;
		obj->intelligent = TRUE;
		obj->saveAnim = TRUE;
		obj->saveFlags = TRUE;
		obj->saveMesh = TRUE;
		obj->savePosition = TRUE;
		obj->undead = TRUE;
		obj->hitEffect = HIT_SMOKE;
		obj->zoneType = ZONE_WATER;
		Bones[obj->boneIndex + 6 * 4] |= ROT_Y | ROT_X;
		Bones[obj->boneIndex + 7 * 4] |= ROT_Y | ROT_X;
	}

	obj = &Objects[ID_LOCUSTS_EMITTER];
	if (obj->loaded)
	{
		obj->initialise = InitialiseLocust;
		obj->control = LocustControl;
		obj->drawRoutine = NULL;
		obj->saveFlags = TRUE;
		obj->zoneType = ZONE_BASIC;
	}

	obj = &Objects[ID_WRAITH3];
	if (obj->loaded)
	{
		//not decompiled yet and multiple versions of wraiths exist
	}
}

static void StartObject(ObjectInfo* obj)
{
	obj = &Objects[ID_SARCOPHAGUS];
	if (obj->loaded)
	{
		obj->control = AnimatingControl;
		obj->collision = SarcophagusCollision;
		obj->saveFlags = true;
		obj->saveAnim = true;
	}
}

static void StartTrap(ObjectInfo* obj)
{
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

	obj = &Objects[ID_COG];
	if (obj->loaded)
	{
		obj->control = CogControl;
		obj->collision = GenericSphereBoxCollision;
		obj->saveAnim = true;
		obj->saveFlags = true;
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
		obj->zoneType = ZONE_BASIC;
	}
}

static void StartVehicles(ObjectInfo* obj)
{
	obj = &Objects[ID_JEEP];
	if (obj->loaded)
	{
		obj->initialise = InitialiseJeep;
		obj->collision = JeepCollision;
		obj->savePosition = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
	}

	obj = &Objects[ID_MOTORBIKE];
	if (obj->loaded)
	{
		obj->initialise = InitialiseMotorbike;
		obj->collision = MotorbikeCollision;
		//obj->drawRoutine = DrawMotorbike; // for wheel rotation
		obj->drawRoutineExtra = DrawMotorbikeEffect;
		obj->savePosition = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
	}
}

static void StartSwitch(ObjectInfo* obj)
{
	
}

static ObjectInfo* objToInit;
void InitialiseTR4Objects()
{
	StartBaddy(objToInit);
	StartObject(objToInit);
	StartSwitch(objToInit);
	StartTrap(objToInit);
	StartVehicles(objToInit);
}
