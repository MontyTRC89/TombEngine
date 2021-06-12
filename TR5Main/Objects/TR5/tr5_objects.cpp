#include "framework.h"
#include "tr5_objects.h"
/// entities
#include "tr5_autoguns.h"	  // OK
#include "tr5_brownbeast.h"	  // OK
#include "tr5_chef.h"		  // OK
#include "tr5_cyborg.h"		  // OK
#include "tr5_doberman.h"	  // OK
#include "tr5_dog.h"		  // OK
#include "tr5_ghost.h"		  // OK
#include "tr5_gladiator.h"	  // OK
#include "tr5_guard.h"		  // OK
#include "tr5_gunship.h"	  // OK
#include "tr5_hydra.h"		  // OK
#include "tr5_imp.h"		  // OK
#include "tr5_lagoon_witch.h" // OK
#include "tr5_larson.h"		  // OK
#include "tr5_laser_head.h"	  // OK
#include "tr5_lion.h"		  // OK
#include "tr5_reaper.h"		  // OK
#include "tr5_roman_statue.h" // OK
#include "tr5_submarine.h"	  // OK
#include "tr5_willowwisp.h"	  // OK
/// emitter
#include "tr5_rats_emitter.h"
#include "tr5_bats_emitter.h"
#include "tr5_spider_emitter.h"
#include "tr5_dart_emitter.h"
#include "tr5_smoke_emitter.h"
/// objects
#include "tr5_pushableblock.h"
#include "tr5_twoblockplatform.h"
#include "tr5_raisingcog.h"
#include "tr5_raisingblock.h"
#include "tr5_expandingplatform.h"
#include "tr5_light.h"
#include "tr5_bodypart.h"
#include "tr5_teleporter.h"
#include "tr5_highobject.h"
#include "tr4_bubbles.h"
#include "tr5_missile.h"
#include "tr5_genslot.h"
/// traps
#include "tr5_teethspike.h"
#include "tr5_ventilator.h"
#include "tr5_deathslide.h"
#include "tr5_electricity.h"
#include "tr5_romehammer.h"
#include "tr5_fallingceiling.h"
#include "tr5_rollingball.h"
#include "tr5_explosion.h"
/// switch

/// shatter
#include "tr5_smashobject.h"
/// necessary import
#include "collide.h"
#include "lara_one_gun.h"
#include "lara_flare.h"
#include "lara_initialise.h"
#include "pickup.h"
#include "flmtorch.h"
#include "setup.h"
#include "switch.h"
#include "objects.h"
#include "level.h"
/// register objects
#include "object_helper.h"

static void StartBaddy(OBJECT_INFO *obj)
{
	obj = &Objects[ID_LARA];
	if (obj->loaded)
	{
		obj->initialise = InitialiseLaraLoad;
		obj->shadowSize = 160;
		obj->hitPoints = 1000;
		obj->drawRoutine = nullptr;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->saveHitpoints = true;
		obj->savePosition = true;
		obj->usingDrawAnimatingItem = false;
	}

	obj = &Objects[ID_SAS];
	if (obj->loaded)
	{
		obj->initialise = InitialiseGuard;
		obj->control = GuardControl;
		obj->collision = CreatureCollision;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->hitPoints = 40;
		obj->radius = 102;
		obj->pivotLength = 50;
		obj->intelligent = true;
		obj->savePosition = true;
		obj->saveFlags = true;
		obj->saveAnim = true;
		obj->saveHitpoints = true;
		obj->hitEffect = HIT_BLOOD;
		obj->zoneType = ZONE_HUMAN_CLASSIC;
		g_Level.Bones[obj->boneIndex + 6 * 4] |= ROT_Y;
		g_Level.Bones[obj->boneIndex + 6 * 4] |= ROT_X;
		g_Level.Bones[obj->boneIndex + 13 * 4] |= ROT_Y;
		g_Level.Bones[obj->boneIndex + 13 * 4] |= ROT_X;
	}

	obj = &Objects[ID_SWAT];
	if (obj->loaded)
	{
		obj->biteOffset = 0;
		obj->initialise = InitialiseGuard;
		obj->collision = CreatureCollision;
		obj->control = GuardControl;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->hitPoints = 24;
		obj->pivotLength = 50;
		obj->radius = 102;
		obj->explodableMeshbits = 0x4000;
		obj->intelligent = true;
		obj->savePosition = true;
		obj->saveFlags = true;
		obj->saveAnim = true;
		obj->saveHitpoints = true;
		obj->hitEffect = HIT_BLOOD;
		obj->zoneType = ZONE_HUMAN_CLASSIC;
		g_Level.Bones[obj->boneIndex + 6 * 4] |= ROT_Y;
		g_Level.Bones[obj->boneIndex + 6 * 4] |= ROT_X;
		g_Level.Bones[obj->boneIndex + 13 * 4] |= ROT_Y;
		g_Level.Bones[obj->boneIndex + 13 * 4] |= ROT_X;
	}

	obj = &Objects[ID_GUARD1];
	if (obj->loaded)
	{
		if (Objects[ID_SWAT].loaded) // object required
			obj->animIndex = Objects[ID_SWAT].animIndex;
		obj->biteOffset = 4;
		obj->initialise = InitialiseGuard;
		obj->collision = CreatureCollision;
		obj->control = GuardControl;
		obj->pivotLength = 50;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->hitPoints = 24;
		obj->radius = 102;
		obj->explodableMeshbits = 0x4000;
		obj->intelligent = true;
		obj->savePosition = true;
		obj->saveFlags = true;
		obj->saveAnim = true;
		obj->saveHitpoints = true;
		obj->hitEffect = HIT_BLOOD;
		obj->zoneType = ZONE_HUMAN_CLASSIC;
		g_Level.Bones[obj->boneIndex + 6 * 4] |= ROT_Y;
		g_Level.Bones[obj->boneIndex + 6 * 4] |= ROT_X;
		g_Level.Bones[obj->boneIndex + 13 * 4] |= ROT_Y;
		g_Level.Bones[obj->boneIndex + 13 * 4] |= ROT_X;
	}

	obj = &Objects[ID_SWAT_PLUS];
	if (obj->loaded)
	{
		short animIndex;
		if (!Objects[ID_SWAT].loaded)
			animIndex = Objects[ID_GUARD1].animIndex;
		else
			animIndex = Objects[ID_SWAT].animIndex;
		obj->animIndex = animIndex;
		obj->biteOffset = 0;
		obj->initialise = InitialiseGuard;
		obj->collision = CreatureCollision;
		obj->control = GuardControl;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->hitPoints = 24;
		obj->pivotLength = 50;
		obj->radius = 102;
		obj->intelligent = true;
		obj->savePosition = true;
		obj->saveFlags = true;
		obj->saveAnim = true;
		obj->saveHitpoints = true;
		obj->hitEffect = HIT_BLOOD;
		obj->zoneType = ZONE_HUMAN_CLASSIC;
		g_Level.Bones[obj->boneIndex + 6 * 4] |= ROT_Y;
		g_Level.Bones[obj->boneIndex + 6 * 4] |= ROT_X;
		g_Level.Bones[obj->boneIndex + 13 * 4] |= ROT_Y;
		g_Level.Bones[obj->boneIndex + 13 * 4] |= ROT_X;
	}

	obj = &Objects[ID_MAFIA];
	if (obj->loaded)
	{
		short animIndex;
		if (!Objects[ID_SWAT].loaded)
			animIndex = Objects[ID_GUARD1].animIndex;
		else
			animIndex = Objects[ID_SWAT].animIndex;
		obj->animIndex = animIndex;
		obj->biteOffset = 0;
		obj->initialise = InitialiseGuard;
		obj->collision = CreatureCollision;
		obj->control = GuardControl;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->hitPoints = 24;
		obj->pivotLength = 50;
		obj->radius = 102;
		obj->explodableMeshbits = 0x4000;
		obj->intelligent = true;
		obj->savePosition = true;
		obj->saveFlags = true;
		obj->saveAnim = true;
		obj->saveHitpoints = true;
		obj->hitEffect = HIT_BLOOD;
		obj->zoneType = ZONE_HUMAN_CLASSIC;
		g_Level.Bones[obj->boneIndex + 6 * 4] |= ROT_Y;
		g_Level.Bones[obj->boneIndex + 6 * 4] |= ROT_X;
		g_Level.Bones[obj->boneIndex + 13 * 4] |= ROT_Y;
		g_Level.Bones[obj->boneIndex + 13 * 4] |= ROT_X;
	}

	obj = &Objects[ID_SCIENTIST];
	if (obj->loaded)
	{
		short animIndex;
		if (!Objects[ID_SWAT].loaded)
			animIndex = Objects[ID_GUARD1].animIndex;
		else
			animIndex = Objects[ID_SWAT].animIndex;
		obj->animIndex = animIndex;
		obj->initialise = InitialiseGuard;
		obj->control = GuardControl;
		obj->collision = CreatureCollision;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->hitPoints = 24;
		obj->pivotLength = 50;
		obj->radius = 102;
		obj->intelligent = true;
		obj->savePosition = true;
		obj->saveFlags = true;
		obj->saveAnim = true;
		obj->saveHitpoints = true;
		obj->hitEffect = HIT_BLOOD;
		obj->zoneType = ZONE_HUMAN_CLASSIC;
		g_Level.Bones[Objects[69].boneIndex + 6 * 4] |= ROT_Y;
		g_Level.Bones[Objects[69].boneIndex + 6 * 4] |= ROT_X;
		g_Level.Bones[Objects[69].boneIndex + 13 * 4] |= ROT_Y;
		g_Level.Bones[Objects[69].boneIndex + 13 * 4] |= ROT_X;
	}

	obj = &Objects[ID_GUARD2];
	if (obj->loaded)
	{
		short animIndex;
		if (!Objects[ID_SWAT].loaded)
			animIndex = Objects[ID_GUARD1].animIndex;
		else
			animIndex = Objects[ID_SWAT].animIndex;
		obj->animIndex = animIndex;
		obj->biteOffset = 4;
		obj->initialise = InitialiseGuard;
		obj->control = GuardControl;
		obj->collision = CreatureCollision;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->hitPoints = 24;
		obj->pivotLength = 50;
		obj->radius = 102;
		obj->explodableMeshbits = 0x4000;
		obj->intelligent = true;
		obj->savePosition = true;
		obj->saveFlags = true;
		obj->saveAnim = true;
		obj->saveHitpoints = true;
		obj->hitEffect = HIT_BLOOD;
		obj->zoneType = ZONE_HUMAN_CLASSIC;
		g_Level.Bones[obj->boneIndex + 6 * 4] |= ROT_Y;
		g_Level.Bones[obj->boneIndex + 6 * 4] |= ROT_X;
		g_Level.Bones[obj->boneIndex + 13 * 4] |= ROT_Y;
		g_Level.Bones[obj->boneIndex + 13 * 4] |= ROT_X;
	}

	obj = &Objects[ID_GUARD3];
	if (obj->loaded)
	{
		short animIndex;
		if (!Objects[ID_SWAT].loaded)
			animIndex = Objects[ID_GUARD1].animIndex;
		else
			animIndex = Objects[ID_SWAT].animIndex;
		obj->animIndex = animIndex;
		obj->biteOffset = 4;
		obj->initialise = InitialiseGuard;
		obj->control = GuardControl;
		obj->collision = CreatureCollision;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->hitPoints = 24;
		obj->pivotLength = 50;
		obj->radius = 102;
		obj->explodableMeshbits = 0x4000;
		obj->intelligent = true;
		obj->savePosition = true;
		obj->saveFlags = true;
		obj->saveAnim = true;
		obj->saveHitpoints = true;
		obj->hitEffect = HIT_BLOOD;
		obj->zoneType = ZONE_HUMAN_CLASSIC;
		g_Level.Bones[obj->boneIndex + 6 * 4] |= ROT_Y;
		g_Level.Bones[obj->boneIndex + 6 * 4] |= ROT_X;
		g_Level.Bones[obj->boneIndex + 13 * 4] |= ROT_Y;
		g_Level.Bones[obj->boneIndex + 13 * 4] |= ROT_X;
	}

	obj = &Objects[ID_ATTACK_SUB];
	if (obj->loaded)
	{
		obj->initialise = InitialiseSubmarine;
		obj->collision = CreatureCollision;
		obj->control = SubmarineControl;
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
		obj->hitEffect = HIT_RICOCHET;
		obj->zoneType = ZONE_FLYER;
		obj->undead = true;
		g_Level.Bones[obj->boneIndex] |= ROT_X;
		g_Level.Bones[obj->boneIndex + 4] |= ROT_X;
	}

	obj = &Objects[ID_CHEF];
	if (obj->loaded)
	{
		obj->initialise = InitialiseChef;
		obj->control = ControlChef;
		obj->collision = CreatureCollision;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->hitPoints = 35;
		obj->pivotLength = 50;
		obj->radius = 102;
		obj->biteOffset = 0;
		obj->intelligent = true;
		obj->savePosition = true;
		obj->saveFlags = true;
		obj->saveAnim = true;
		obj->saveHitpoints = true;
		obj->hitEffect = HIT_BLOOD;
		obj->zoneType = ZONE_HUMAN_CLASSIC;

		g_Level.Bones[obj->boneIndex + 4 * 6] |= ROT_Y;
		g_Level.Bones[obj->boneIndex + 4 * 6] |= ROT_X;
		g_Level.Bones[obj->boneIndex + 4 * 13] |= ROT_Y;
		g_Level.Bones[obj->boneIndex + 4 * 13] |= ROT_X;
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
		obj->savePosition = true;
		obj->saveFlags = true;
		obj->saveAnim = true;
		obj->saveHitpoints = true;
		obj->hitEffect = HIT_BLOOD;
		obj->zoneType = ZONE_BASIC;
		g_Level.Bones[obj->boneIndex + 6 * 4] |= ROT_Y;
		g_Level.Bones[obj->boneIndex + 19 * 4] |= ROT_Y;
	}

	obj = &Objects[ID_DOBERMAN];
	if (obj->loaded)
	{
		obj->initialise = InitialiseDoberman;
		obj->collision = CreatureCollision;
		obj->control = DobermanControl;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->hitPoints = 18;
		obj->pivotLength = 50;
		obj->radius = 256;
		obj->intelligent = true;
		obj->savePosition = true;
		obj->saveFlags = true;
		obj->saveAnim = true;
		obj->saveHitpoints = true;
		obj->hitEffect = HIT_BLOOD;
		obj->zoneType = ZONE_BASIC;
		g_Level.Bones[obj->boneIndex + 19 * 4] |= ROT_Y;
	}

	obj = &Objects[ID_HUSKIE];
	if (obj->loaded)
	{
		obj->initialise = InitialiseTr5Dog;
		obj->collision = CreatureCollision;
		obj->control = Tr5DogControl;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->hitPoints = 24;
		obj->pivotLength = 50;
		obj->radius = 256;
		obj->intelligent = true;
		obj->savePosition = true;
		obj->saveFlags = true;
		obj->saveAnim = true;
		obj->saveHitpoints = true;
		obj->hitEffect = HIT_BLOOD;
		obj->zoneType = ZONE_BASIC;
		g_Level.Bones[obj->boneIndex + 19 * 4] |= ROT_Y;
	}

	obj = &Objects[ID_REAPER];
	if (obj->loaded)
	{
		obj->initialise = InitialiseReaper;
		obj->collision = CreatureCollision;
		obj->control = ReaperControl;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->hitPoints = 10;
		obj->pivotLength = 50;
		obj->radius = 102;
		obj->intelligent = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->saveHitpoints = true;
		obj->savePosition = true;
		obj->hitEffect = HIT_BLOOD;
		obj->waterCreature = true;
		obj->zoneType = ZONE_FLYER;
	}

	obj = &Objects[ID_MAFIA2];
	if (obj->loaded)
	{
		obj->biteOffset = 7;
		obj->initialise = InitialiseMafia2;
		obj->collision = CreatureCollision;
		obj->control = Mafia2Control;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->hitPoints = 26;
		obj->pivotLength = 50;
		obj->radius = 102;
		obj->explodableMeshbits = 0x4000;
		obj->intelligent = true;
		obj->savePosition = true;
		obj->saveFlags = true;
		obj->saveAnim = true;
		obj->saveHitpoints = true;
		obj->hitEffect = HIT_BLOOD;
		obj->zoneType = ZONE_HUMAN_CLASSIC;
		obj->meshSwapSlot = ID_MESHSWAP_MAFIA2;

		g_Level.Bones[obj->boneIndex + 6 * 4] |= ROT_Y;
		g_Level.Bones[obj->boneIndex + 6 * 4] |= ROT_X;
		g_Level.Bones[obj->boneIndex + 13 * 4] |= ROT_Y;
		g_Level.Bones[obj->boneIndex + 13 * 4] |= ROT_X;
	}

	obj = &Objects[ID_PIERRE];
	if (obj->loaded)
	{
		obj->biteOffset = 1;
		obj->initialise = InitialiseLarson;
		obj->collision = CreatureCollision;
		obj->control = LarsonControl;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->hitPoints = 60;
		obj->pivotLength = 50;
		obj->radius = 102;
		obj->intelligent = true;
		obj->savePosition = true;
		obj->saveFlags = true;
		obj->saveAnim = true;
		obj->saveHitpoints = true;
		obj->hitEffect = HIT_BLOOD;
		obj->zoneType = ZONE_HUMAN_CLASSIC;
		g_Level.Bones[obj->boneIndex + 6 * 4] |= ROT_Y;
		g_Level.Bones[obj->boneIndex + 6 * 4] |= ROT_X;
		g_Level.Bones[obj->boneIndex + 7 * 4] |= ROT_Y;
		g_Level.Bones[obj->boneIndex + 7 * 4] |= ROT_X;
	}

	obj = &Objects[ID_LARSON];
	if (obj->loaded)
	{
		obj->biteOffset = 3;
		obj->initialise = InitialiseLarson;
		obj->collision = CreatureCollision;
		obj->control = LarsonControl;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->hitPoints = 60;
		obj->pivotLength = 50;
		obj->radius = 102;
		obj->intelligent = true;
		obj->savePosition = true;
		obj->saveFlags = true;
		obj->saveAnim = true;
		obj->saveHitpoints = true;
		obj->hitEffect = HIT_BLOOD;
		obj->zoneType = ZONE_HUMAN_CLASSIC;
		g_Level.Bones[obj->boneIndex + 6 * 4] |= ROT_Y;
		g_Level.Bones[obj->boneIndex + 6 * 4] |= ROT_X;
		g_Level.Bones[obj->boneIndex + 7 * 4] |= ROT_Y;
		g_Level.Bones[obj->boneIndex + 7 * 4] |= ROT_X;
	}

	obj = &Objects[ID_HITMAN];
	if (obj->loaded)
	{
		obj->biteOffset = 5;
		obj->initialise = InitialiseHitman;
		obj->collision = CreatureCollision;
		obj->control = HitmanControl;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->hitPoints = 50;
		obj->pivotLength = 50;
		obj->radius = 102;
		obj->intelligent = true;
		obj->savePosition = true;
		obj->saveFlags = true;
		obj->saveAnim = true;
		obj->saveHitpoints = true;
		obj->hitEffect = HIT_RICOCHET;
		obj->undead = true;
		obj->zoneType = ZONE_HUMAN_CLASSIC;
		obj->meshSwapSlot = ID_MESHSWAP_HITMAN;

		g_Level.Bones[obj->boneIndex + 6 * 4] |= ROT_Y;
		g_Level.Bones[obj->boneIndex + 6 * 4] |= ROT_X;
		g_Level.Bones[obj->boneIndex + 13 * 4] |= ROT_Y;
		g_Level.Bones[obj->boneIndex + 13 * 4] |= ROT_X;
	}

	obj = &Objects[ID_SNIPER];
	if (obj->loaded)
	{
		obj->biteOffset = 6;
		obj->initialise = InitialiseSniper;
		obj->collision = CreatureCollision;
		obj->control = SniperControl;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->hitPoints = 35;
		obj->pivotLength = 50;
		obj->radius = 102;
		obj->explodableMeshbits = 0x4000;
		obj->intelligent = true;
		obj->savePosition = true;
		obj->saveFlags = true;
		obj->saveAnim = true;
		obj->saveHitpoints = true;
		obj->hitEffect = HIT_BLOOD;
		g_Level.Bones[obj->boneIndex + 6 * 4] |= ROT_Y;
		g_Level.Bones[obj->boneIndex + 6 * 4] |= ROT_X;
		g_Level.Bones[obj->boneIndex + 13 * 4] |= ROT_Y;
		g_Level.Bones[obj->boneIndex + 13 * 4] |= ROT_X;
	}

	obj = &Objects[ID_GUARD_LASER];
	if (obj->loaded)
	{
		obj->biteOffset = 0;
		obj->initialise = InitialiseGuardLaser;
		obj->collision = CreatureCollision;
		//obj->control = GuardControlLaser;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->hitPoints = 24;
		obj->pivotLength = 50;
		obj->radius = 128;
		obj->explodableMeshbits = 4;
		obj->intelligent = true;
		obj->savePosition = true;
		obj->saveFlags = true;
		obj->saveAnim = true;
		obj->saveHitpoints = true;
		obj->hitEffect = HIT_RICOCHET;
		obj->undead = true;
		obj->zoneType = ZONE_HUMAN_CLASSIC;
		g_Level.Bones[obj->boneIndex] |= ROT_Y;
		g_Level.Bones[obj->boneIndex] |= ROT_X;
		g_Level.Bones[obj->boneIndex + 4] |= ROT_Y;
		g_Level.Bones[obj->boneIndex + 4] |= ROT_X;
	}

	obj = &Objects[ID_HYDRA];
	if (obj->loaded)
	{
		obj->initialise = InitialiseHydra;
		obj->collision = CreatureCollision;
		obj->control = HydraControl;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->hitPoints = 30;
		obj->pivotLength = 50;
		obj->radius = 102;
		obj->biteOffset = 1024;
		obj->intelligent = true;
		obj->savePosition = true;
		obj->saveFlags = true;
		obj->saveAnim = true;
		obj->saveHitpoints = true;
		obj->hitEffect = HIT_RICOCHET;
		obj->undead = true;
		obj->zoneType = ZONE_BASIC;
		g_Level.Bones[obj->boneIndex + 0] |= ROT_Y;
		g_Level.Bones[obj->boneIndex + 8 * 4] |= ROT_Y;
		g_Level.Bones[obj->boneIndex + 8 * 4] |= ROT_X;
		g_Level.Bones[obj->boneIndex + 8 * 4] |= ROT_Z;
	}

	obj = &Objects[ID_IMP];
	if (obj->loaded)
	{
		obj->biteOffset = 256;
		obj->initialise = InitialiseImp;
		obj->collision = CreatureCollision;
		obj->control = ImpControl;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->hitPoints = 12;
		obj->pivotLength = 20;
		obj->radius = 102;
		obj->intelligent = true;
		obj->savePosition = true;
		obj->saveFlags = true;
		obj->saveAnim = true;
		obj->saveHitpoints = true;
		obj->hitEffect = HIT_BLOOD;
		obj->meshSwapSlot = ID_MESHSWAP_IMP;
		obj->zoneType = ZONE_BASIC;

		g_Level.Bones[obj->meshIndex + 4 * 4] |= ROT_Z;
		g_Level.Bones[obj->meshIndex + 4 * 4] |= ROT_X;
		g_Level.Bones[obj->meshIndex + 9 * 4] |= ROT_Z;
		g_Level.Bones[obj->meshIndex + 9 * 4] |= ROT_X;
	}

	obj = &Objects[ID_WILLOWISP];
	if (obj->loaded)
	{
		obj->biteOffset = 256;
		obj->initialise = InitialiseLightingGuide;
		//obj->control = ControlLightingGuide;
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
		obj->zoneType = ZONE_FLYER;
		g_Level.Bones[obj->boneIndex + 4 * 4] |= ROT_Z;
		g_Level.Bones[obj->boneIndex + 4 * 4] |= ROT_X;
		g_Level.Bones[obj->boneIndex + 9 * 4] |= ROT_Z;
		g_Level.Bones[obj->boneIndex + 9 * 4] |= ROT_X;
	}

	obj = &Objects[ID_BROWN_BEAST];
	if (obj->loaded)
	{
		obj->biteOffset = 256;
		obj->initialise = InitialiseBrownBeast;
		obj->collision = CreatureCollision;
		obj->control = ControlBrowsBeast;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->hitPoints = 100;
		obj->pivotLength = 20;
		obj->radius = 341;
		obj->intelligent = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->saveHitpoints = true;
		obj->savePosition = true;
		obj->hitEffect = HIT_BLOOD;
		obj->zoneType = ZONE_BASIC;
		g_Level.Bones[obj->boneIndex + 4 * 4] |= ROT_Z;
		g_Level.Bones[obj->boneIndex + 4 * 4] |= ROT_X;
		g_Level.Bones[obj->boneIndex + 9 * 4] |= ROT_Z;
		g_Level.Bones[obj->boneIndex + 9 * 4] |= ROT_X;
	}

	obj = &Objects[ID_LAGOON_WITCH];
	if (obj->loaded)
	{
		obj->biteOffset = 256;
		obj->initialise = InitialiseLagoonWitch;
		obj->collision = CreatureCollision;
		obj->control = LagoonWitchControl;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->hitPoints = 100;
		obj->pivotLength = 20;
		obj->radius = 256;
		obj->intelligent = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->saveHitpoints = true;
		obj->savePosition = true;
		obj->waterCreature = true;
		obj->zoneType = ZONE_BASIC;

		g_Level.Bones[obj->boneIndex + 4 * 4] |= ROT_Z;
		g_Level.Bones[obj->boneIndex + 4 * 4] |= ROT_X;
		g_Level.Bones[obj->boneIndex + 9 * 4] |= ROT_Z;
		g_Level.Bones[obj->boneIndex + 9 * 4] |= ROT_X;
	}

	obj = &Objects[ID_INVISIBLE_GHOST];
	if (obj->loaded)
	{
		obj->biteOffset = 256;
		obj->initialise = InitialiseInvisibleGhost;
		obj->collision = CreatureCollision;
		obj->control = InvisibleGhostControl;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->hitPoints = 100;
		obj->pivotLength = 20;
		obj->radius = 256;
		obj->intelligent = true;
		obj->savePosition = true;
		obj->saveFlags = true;
		obj->saveAnim = true;
		obj->saveHitpoints = true;
		obj->hitEffect = HIT_BLOOD;
		obj->zoneType = ZONE_HUMAN_CLASSIC;
		g_Level.Bones[obj->boneIndex + 6 * 4] |= ROT_Y;
		g_Level.Bones[obj->boneIndex + 6 * 4] |= ROT_X;
		g_Level.Bones[obj->boneIndex + 8 * 4] |= ROT_Y;
		g_Level.Bones[obj->boneIndex + 8 * 4] |= ROT_X;
	}

	obj = &Objects[ID_RATS_EMITTER];
	if (obj->loaded)
	{
		obj->drawRoutine = NULL;
		obj->initialise = InitialiseLittleRats;
		obj->control = LittleRatsControl;
		obj->usingDrawAnimatingItem = false;
	}

	obj = &Objects[ID_BATS_EMITTER];
	if (obj->loaded)
	{
		obj->drawRoutine = NULL;
		obj->initialise = InitialiseLittleBats;
		obj->control = LittleBatsControl;
		obj->usingDrawAnimatingItem = false;
	}

	obj = &Objects[ID_SPIDERS_EMITTER];
	if (obj->loaded)
	{
		obj->drawRoutine = NULL;
		obj->initialise = InitialiseSpiders;
		obj->control = SpidersEmitterControl;
		obj->usingDrawAnimatingItem = false;
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
		obj->savePosition = true;
		obj->saveFlags = true;
		obj->saveAnim = true;
		obj->saveHitpoints = true;
		obj->hitEffect = HIT_BLOOD;
		obj->zoneType = ZONE_HUMAN_CLASSIC;
		g_Level.Bones[obj->boneIndex + 6 * 4] |= ROT_Y;
		g_Level.Bones[obj->boneIndex + 6 * 4] |= ROT_X;
		g_Level.Bones[obj->boneIndex + 13 * 4] |= ROT_Y;
		g_Level.Bones[obj->boneIndex + 13 * 4] |= ROT_X;
	}

	for (int i = 0; i < 2; i++)
	{
		obj = &Objects[ID_ROMAN_GOD1 + i];
		if (obj->loaded)
		{
			obj->biteOffset = 0;
			obj->initialise = InitialiseRomanStatue;
			obj->collision = CreatureCollision;
			obj->control = RomanStatueControl;
			obj->shadowSize = UNIT_SHADOW / 2;
			obj->hitPoints = 300;
			obj->pivotLength = 50;
			obj->radius = 256;
			obj->intelligent = true;
			obj->savePosition = true;
			obj->saveFlags = true;
			obj->saveAnim = true;
			obj->saveHitpoints = true;
			obj->hitEffect = HIT_SMOKE;
			obj->meshSwapSlot = ID_MESHSWAP_ROMAN_GOD1 + i;
			obj->zoneType = ZONE_HUMAN_CLASSIC;
			obj->castShadows = true;

			g_Level.Bones[obj->boneIndex + 24] |= ROT_Y;
			g_Level.Bones[obj->boneIndex + 24] |= ROT_X;
			g_Level.Bones[obj->boneIndex + 52] |= ROT_Y;
			g_Level.Bones[obj->boneIndex + 52] |= ROT_X;
		}
	}

	obj = &Objects[ID_LASERHEAD];
	if (obj->loaded)
	{
		obj->initialise = InitialiseLaserHead;
		obj->collision = CreatureCollision;
		obj->control = LaserHeadControl;
		obj->explodableMeshbits = 6;
		obj->nonLot = true;
		obj->savePosition = true;
		obj->saveHitpoints = true;
		obj->saveFlags = true;
		obj->saveAnim = true;
		obj->usingDrawAnimatingItem = false;
		obj->undead = true;
		obj->hitEffect = HIT_RICOCHET;
		obj->saveMesh = true;
	}

	obj = &Objects[ID_AUTOGUN];
	if (obj->loaded)
	{
		obj->initialise = InitialiseAutoGuns;
		obj->control = AutoGunsControl;
		obj->saveHitpoints = true;
		obj->saveFlags = true;
		obj->saveAnim = true;
		obj->hitEffect = HIT_BLOOD;
		g_Level.Bones[obj->boneIndex + 6 * 4] |= ROT_Y;
		g_Level.Bones[obj->boneIndex + 6 * 4] |= ROT_X;
		g_Level.Bones[obj->boneIndex + 8 * 4] |= ROT_Y;
	}

	obj = &Objects[ID_GUNSHIP];
	if (obj->loaded)
	{
		obj->control = ControlGunShip;
		obj->saveFlags = true;
		obj->saveAnim = true;
		g_Level.Bones[obj->boneIndex + 0] |= ROT_Y;
		g_Level.Bones[obj->boneIndex + 4] |= ROT_X;
	}

	// TR5 SUBMARINE
	obj = &Objects[ID_ATTACK_SUB];
	if (obj->loaded)
	{
		obj->initialise = InitialiseSubmarine;
		obj->control = SubmarineControl;
		obj->saveAnim = true;
		obj->zoneType = ZONE_BASIC;
		obj->hitEffect = HIT_BLOOD;
		obj->castShadows = true;
		obj->hitPoints = 100;
	}
}

static void StartObject(OBJECT_INFO *obj)
{
	InitPickupItem(obj, FlareControl, ID_FLARE_ITEM);
	InitPickupItem(obj, TorchControl, ID_BURNING_TORCH_ITEM, true);

	for (int objNumber = ID_SEARCH_OBJECT1; objNumber <= ID_SEARCH_OBJECT4; objNumber++)
		InitSearchObject(obj, objNumber);

	for (int objNumber = ID_PUSHABLE_OBJECT1; objNumber <= ID_PUSHABLE_OBJECT10; objNumber++)
		InitPushableObject(obj, objNumber);

	obj = &Objects[ID_TWOBLOCK_PLATFORM];
	if (obj->loaded)
	{
		obj->initialise = InitialiseTwoBlocksPlatform;
		obj->control = TwoBlocksPlatformControl;
		obj->floor = TwoBlocksPlatformFloor;
		obj->ceiling = TwoBlocksPlatformCeiling;
		obj->floorBorder = TwoBlocksPlatformFloorBorder;
		obj->ceilingBorder = TwoBlocksPlatformCeilingBorder;
		obj->saveFlags = true;
		obj->savePosition = true;
		obj->saveAnim = true;
	}

	for (int objNum = ID_RAISING_BLOCK1; objNum <= ID_RAISING_BLOCK4; objNum++)
	{
		obj = &Objects[objNum];
		if (obj->loaded)
		{
			obj->initialise = InitialiseRaisingBlock;
			obj->control = ControlRaisingBlock;
			obj->floor = RaisingBlockFloor;
			obj->ceiling = RaisingBlockCeiling;
			obj->floorBorder = RaisingBlockFloorBorder;
			obj->ceilingBorder = RaisingBlockCeilingBorder;
			obj->saveFlags = true;
		}
	}

	obj = &Objects[ID_EXPANDING_PLATFORM];
	if (obj->loaded)
	{
		obj->initialise = InitialiseExpandingPlatform;
		obj->control = ControlExpandingPlatform;
		obj->floor = ExpandingPlatformFloor;
		obj->ceiling = ExpandingPlatformCeiling;
		obj->floorBorder = ExpandingPlatformFloorBorder;
		obj->ceilingBorder = ExpandingPlatformCeilingBorder;
		obj->saveFlags = true;
		obj->savePosition = true;
		obj->saveAnim = true;
	}

	obj = &Objects[ID_ELECTRICAL_LIGHT];
	if (obj->loaded)
	{
		obj->control = ElectricalLightControl;
		obj->drawRoutine = nullptr;
		obj->usingDrawAnimatingItem = false;
		obj->saveFlags = true;
	}

	obj = &Objects[ID_PULSE_LIGHT];
	if (obj->loaded)
	{
		obj->control = PulseLightControl;
		obj->drawRoutine = nullptr;
		obj->usingDrawAnimatingItem = false;
		obj->saveFlags = true;
	}

	obj = &Objects[ID_STROBE_LIGHT];
	if (obj->loaded)
	{
		obj->control = StrobeLightControl;
		obj->saveFlags = true;
	}

	obj = &Objects[ID_COLOR_LIGHT];
	if (obj->loaded)
	{
		obj->control = ColorLightControl;
		obj->drawRoutine = nullptr;
		obj->usingDrawAnimatingItem = false;
		obj->saveFlags = true;
	}

	obj = &Objects[ID_BLINKING_LIGHT];
	if (obj->loaded)
	{
		obj->control = BlinkingLightControl;
		obj->drawRoutine = nullptr;
		obj->saveFlags = true;
	}

	obj = &Objects[ID_BODY_PART];
	obj->loaded = true;
	obj->control = ControlBodyPart;

	obj = &Objects[ID_SMOKE_EMITTER_BLACK];
	if (obj->loaded)
	{
		obj->initialise = InitialiseSmokeEmitter;
		obj->control = SmokeEmitterControl;
		obj->drawRoutine = nullptr;
		obj->usingDrawAnimatingItem = false;
		obj->saveFlags = true;
	}

	obj = &Objects[ID_SMOKE_EMITTER_WHITE];
	if (obj->loaded)
	{
		obj->initialise = InitialiseSmokeEmitter;
		obj->control = SmokeEmitterControl;
		obj->drawRoutine = nullptr;
		obj->usingDrawAnimatingItem = false;
		obj->saveFlags = true;
	}

	obj = &Objects[ID_SMOKE_EMITTER];
	if (obj->loaded)
	{
		obj->initialise = InitialiseSmokeEmitter;
		obj->control = SmokeEmitterControl;
		obj->drawRoutine = nullptr;
		obj->usingDrawAnimatingItem = false;
		obj->saveFlags = true;
	}

	obj = &Objects[ID_TELEPORTER];
	if (obj->loaded)
	{
		obj->initialise = InitialiseTeleporter;
		obj->control = ControlTeleporter;
		obj->drawRoutine = nullptr;
		obj->saveFlags = true;
	}

	obj = &Objects[ID_HIGH_OBJECT1];
	if (obj->loaded)
	{
		obj->initialise = InitialiseHighObject1;
		obj->control = ControlHighObject1;
		obj->collision = ObjectCollision;
		obj->saveFlags = true;
		obj->savePosition = true;
	}

	obj = &Objects[ID_GEN_SLOT1];
	if (obj->loaded)
	{
		obj->control = GenSlot1Control;
		obj->saveAnim = true;
		obj->saveFlags = true;
	}

	for (int objectNumber = ID_AI_GUARD; objectNumber <= ID_AI_X2; objectNumber++)
	{
		obj = &Objects[objectNumber];
		if (obj->loaded)
		{
			obj->drawRoutine = nullptr;
			obj->collision = AIPickupCollision;
			obj->hitPoints = 0;
		}
	}
}

static void StartTrap(OBJECT_INFO *obj)
{
	obj = &Objects[ID_ZIPLINE_HANDLE];
	if (obj->loaded)
	{
		obj->initialise = InitialiseDeathSlide;
		obj->collision = DeathSlideCollision;
		obj->control = ControlDeathSlide;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->savePosition = true;
	}

	obj = &Objects[ID_PROPELLER_H];
	if (obj->loaded)
	{
		obj->initialise = InitialiseVentilator;
		obj->control = VentilatorControl;
	}

	obj = &Objects[ID_PROPELLER_V];
	if (obj->loaded)
	{
		obj->initialise = InitialiseVentilator;
		obj->control = VentilatorControl;
	}

	obj = &Objects[ID_TEETH_SPIKES];
	if (obj->loaded)
	{
		obj->initialise = InitialiseTeethSpikes;
		obj->control = ControlTeethSpikes;
		obj->saveFlags = 1;
	}

	obj = &Objects[ID_ELECTRICAL_CABLES];
	if (obj->loaded)
	{
		obj->control = ElectricityWiresControl;
		obj->saveAnim = true;
		obj->saveFlags = true;
	}

	obj = &Objects[ID_ROME_HAMMER];
	if (obj->loaded)
	{
		obj->initialise = InitialiseRomeHammer;
		obj->collision = GenericSphereBoxCollision;
		obj->control = AnimatingControl;
		obj->saveAnim = true;
		obj->saveFlags = true;
	}

	obj = &Objects[ID_FALLING_CEILING];
	if (obj->loaded)
	{
		obj->collision = TrapCollision;
		obj->control = FallingCeilingControl;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->savePosition = true;
	}

	obj = &Objects[ID_ROLLINGBALL];
	if (obj->loaded)
	{
		obj->collision = RollingBallCollision;
		obj->control = RollingBallControl;
		obj->savePosition = true;
		obj->saveFlags = true;
	}
	
	obj = &Objects[ID_CLASSIC_ROLLING_BALL];
	if (obj->loaded)
	{
		obj->initialise = InitialiseClassicRollingBall;
		obj->control = ClassicRollingBallControl;
		obj->collision = ClassicRollingBallCollision;
		obj->savePosition = true;
		obj->saveFlags = true;
		obj->saveAnim = true;
	}

	obj = &Objects[ID_BIG_ROLLING_BALL];
	if (obj->loaded)
	{
		obj->collision = ClassicRollingBallCollision;
		obj->control = ClassicRollingBallControl;
		obj->initialise = InitialiseClassicRollingBall;
		obj->savePosition = true;
		obj->saveFlags = true;
		obj->saveAnim = true;
	}

	obj = &Objects[ID_DARTS];
	if (obj->loaded)
	{
		obj->shadowSize = UNIT_SHADOW / 2;
		//obj->drawRoutine = DrawDart;
		obj->collision = ObjectCollision;
		obj->control = DartControl;
		obj->usingDrawAnimatingItem = false;
	}

	obj = &Objects[ID_DART_EMITTER];
	if (obj->loaded)
	{
		obj->control = DartEmitterControl;
		obj->drawRoutine = nullptr;
		obj->saveFlags = true;
		obj->usingDrawAnimatingItem = false;
	}

	obj = &Objects[ID_HOMING_DART_EMITTER];
	if (obj->loaded)
	{
		obj->control = DartEmitterControl;
		obj->drawRoutine = nullptr;
		obj->saveFlags = true;
		obj->usingDrawAnimatingItem = false;
	}

	obj = &Objects[ID_GEN_SLOT3];
	if (obj->loaded)
	{
		obj->initialise = InitialiseGenSlot3;
		obj->collision = HybridCollision;
		obj->control = AnimatingControl;
		obj->saveAnim = true;
		obj->saveFlags = true;
	}

	obj = &Objects[ID_GEN_SLOT4];
	if (obj->loaded)
	{
		//obj->initialise = InitialiseGenSlot4;
		//obj->control = GenSlot4Control;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->savePosition = true;
	}

	obj = &Objects[ID_EXPLOSION];
	if (obj->loaded)
	{
		obj->initialise = InitialiseExplosion;
		obj->control = ExplosionControl;
		obj->drawRoutine = nullptr;
		obj->saveFlags = true;
		obj->usingDrawAnimatingItem = false;
	}
}

static void StartSwitch(OBJECT_INFO *obj)
{
	obj = &Objects[ID_COG_SWITCH];
	if (obj->loaded)
	{
		obj->collision = CogSwitchCollision;
		obj->control = CogSwitchControl;
		obj->saveFlags = true;
		obj->saveAnim = true;
	}

	obj = &Objects[ID_RAISING_COG];
	if (obj->loaded)
	{
		obj->initialise = InitialiseRaisingCog;
		obj->control = RaisingCogControl;
		obj->saveFlags = true;
		obj->savePosition = true;
		obj->saveAnim = true;
	}
}

static void StartShatter(OBJECT_INFO *obj)
{
	for (int i = ID_SMASH_OBJECT1; i <= ID_SMASH_OBJECT16; i++)
		InitSmashObject(obj, i);
}

static void StartProjectiles(OBJECT_INFO *obj)
{
	InitProjectile(obj, BubblesControl, ID_ENERGY_BUBBLES, true);
	InitProjectile(obj, MissileControl, ID_BUBBLES, true);
	InitProjectile(obj, MissileControl, ID_IMP_ROCK, true);
	InitProjectile(obj, TorpedoControl, ID_TORPEDO);
	InitProjectile(obj, ControlGrenade, ID_GRENADE);
	InitProjectile(obj, ControlRocket, ID_ROCKET);
	InitProjectile(obj, ControlHarpoonBolt, ID_HARPOON);
	InitProjectile(obj, ControlCrossbowBolt, ID_CROSSBOW_BOLT);
}

static void StartPickup(OBJECT_INFO *obj)
{
	for (int objNumber = ID_PUZZLE_ITEM1; objNumber <= ID_EXAMINE8_COMBO2; objNumber++)
	{
		InitPickup(obj, objNumber);
	}

	InitPickup(obj, ID_HAMMER_ITEM);
	InitPickup(obj, ID_CROWBAR_ITEM);
	InitPickup(obj, ID_PISTOLS_ITEM);
	InitPickup(obj, ID_PISTOLS_AMMO_ITEM);
	InitPickup(obj, ID_UZI_ITEM);
	InitPickup(obj, ID_UZI_AMMO_ITEM);
	InitPickup(obj, ID_SHOTGUN_ITEM);
	InitPickup(obj, ID_SHOTGUN_AMMO1_ITEM);
	InitPickup(obj, ID_SHOTGUN_AMMO2_ITEM);
	InitPickup(obj, ID_CROSSBOW_ITEM);
	InitPickup(obj, ID_CROSSBOW_AMMO1_ITEM);
	InitPickup(obj, ID_CROSSBOW_AMMO2_ITEM);
	InitPickup(obj, ID_CROSSBOW_AMMO3_ITEM);
	InitPickup(obj, ID_GRENADE_GUN_ITEM);
	InitPickup(obj, ID_GRENADE_AMMO1_ITEM);
	InitPickup(obj, ID_GRENADE_AMMO2_ITEM);
	InitPickup(obj, ID_GRENADE_AMMO3_ITEM);
	InitPickup(obj, ID_HARPOON_ITEM);
	InitPickup(obj, ID_HARPOON_AMMO_ITEM);
	InitPickup(obj, ID_ROCKET_LAUNCHER_ITEM);
	InitPickup(obj, ID_ROCKET_LAUNCHER_AMMO_ITEM);
	InitPickup(obj, ID_HK_ITEM);
	InitPickup(obj, ID_HK_AMMO_ITEM);
	InitPickup(obj, ID_REVOLVER_ITEM);
	InitPickup(obj, ID_REVOLVER_AMMO_ITEM);
	InitPickup(obj, ID_BIGMEDI_ITEM);
	InitPickup(obj, ID_SMALLMEDI_ITEM);
	InitPickup(obj, ID_LASERSIGHT_ITEM);
	InitPickup(obj, ID_BINOCULARS_ITEM);
	InitPickup(obj, ID_SILENCER_ITEM);
	InitPickup(obj, ID_FLARE_INV_ITEM);
	InitPickup(obj, ID_WATERSKIN1_EMPTY);
	InitPickup(obj, ID_WATERSKIN2_EMPTY);
	InitPickup(obj, ID_GOLDROSE_ITEM);
}

static OBJECT_INFO *objToInit;
void InitialiseTR5Objects()
{
	StartBaddy(objToInit);
	StartObject(objToInit);
	StartTrap(objToInit);
	StartPickup(objToInit);
	StartSwitch(objToInit);
	StartShatter(objToInit);
	StartProjectiles(objToInit);
}

void AllocTR5Objects()
{
	Bats = game_malloc<BAT_STRUCT>(NUM_BATS);
	ZeroMemory(Bats, NUM_BATS * sizeof(BAT_STRUCT));

	Spiders = game_malloc<SPIDER_STRUCT>(NUM_SPIDERS);
	ZeroMemory(Spiders, NUM_SPIDERS * sizeof(SPIDER_STRUCT));

	Rats = game_malloc<RAT_STRUCT>(NUM_RATS);
	ZeroMemory(Rats, NUM_RATS * sizeof(RAT_STRUCT));
}