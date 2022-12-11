#include "framework.h"
#include "Objects/TR5/tr5_objects.h"

#include "Game/collision/collide_item.h"
#include "Game/control/box.h"
#include "Game/itemdata/creature_info.h"
#include "Game/Lara/lara_initialise.h"
#include "Game/Lara/lara_one_gun.h"
#include "Game/pickup/pickup.h"
#include "Objects/Generic/Object/objects.h"
#include "Objects/Generic/Switches/switch.h"
#include "Objects/Utils/object_helper.h"
#include "Specific/level.h"
#include "Specific/setup.h"

// Creatures
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

// Emitters
#include "Objects/TR5/Emitter/tr5_rats_emitter.h"
#include "Objects/TR5/Emitter/tr5_bats_emitter.h"
#include "Objects/TR5/Emitter/tr5_spider_emitter.h"
#include "tr5_smoke_emitter.h"

// Objects
#include "Objects/TR5/Object/tr5_pushableblock.h"
#include "tr5_twoblockplatform.h"
#include "tr5_raisingcog.h"
#include "tr5_raisingblock.h"
#include "tr5_expandingplatform.h"
#include "tr5_light.h"
#include "tr5_bodypart.h"
#include "tr5_teleporter.h"
#include "tr5_highobject.h"
#include "tr5_missile.h"
#include "tr5_genslot.h"

// Traps
#include "tr5_ventilator.h"
#include "tr5_zip_line.h"
#include "Objects/Effects/tr5_electricity.h"
#include "tr5_romehammer.h"
#include "tr5_fallingceiling.h"
#include "tr5_rollingball.h"
#include "tr5_explosion.h"
#include "tr5_wreckingball.h"

// Switches
#include "tr5_crowdove_switch.h"

// Shatters
#include "Objects/TR5/Shatter/tr5_smashobject.h"

using namespace TEN::Entities::Creatures::TR5;
using namespace TEN::Entities::Switches;

static void StartEntity(ObjectInfo *obj)
{
	obj = &Objects[ID_LARA];
	if (obj->loaded)
	{
		obj->initialise = InitialiseLaraLoad;
		obj->shadowType = ShadowMode::Lara;
		obj->HitPoints = 1000;
		obj->usingDrawAnimatingItem = false;
	}

	obj = &Objects[ID_SAS];
	if (obj->loaded)
	{
		obj->initialise = InitialiseGuard;
		obj->control = GuardControl;
		obj->collision = CreatureCollision;
		obj->shadowType = ShadowMode::All;
		obj->HitPoints = 40;
		obj->hitEffect = HIT_BLOOD;
		obj->radius = 102;
		obj->pivotLength = 50;
		obj->intelligent = true;
		obj->ZoneType = ZoneType::HumanClassic;
		
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
		obj->shadowType = ShadowMode::All;
		obj->HitPoints = 24;
		obj->hitEffect = HIT_BLOOD;
		obj->pivotLength = 50;
		obj->radius = 102;
		obj->explodableMeshbits = 0x4000;
		obj->intelligent = true;
		obj->ZoneType = ZoneType::HumanClassic;
		
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
		obj->shadowType = ShadowMode::All;
		obj->HitPoints = 24;
		obj->hitEffect = HIT_BLOOD;
		obj->radius = 102;
		obj->explodableMeshbits = 0x4000;
		obj->intelligent = true;
		obj->ZoneType = ZoneType::HumanClassic;
		
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
		obj->shadowType = ShadowMode::All;
		obj->HitPoints = 24;
		obj->hitEffect = HIT_BLOOD;
		obj->pivotLength = 50;
		obj->radius = 102;
		obj->intelligent = true;
		obj->ZoneType = ZoneType::HumanClassic;
		
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
		obj->shadowType = ShadowMode::All;
		obj->HitPoints = 24;
		obj->hitEffect = HIT_BLOOD;
		obj->pivotLength = 50;
		obj->radius = 102;
		obj->explodableMeshbits = 0x4000;
		obj->intelligent = true;
		obj->ZoneType = ZoneType::HumanClassic;
		
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
		obj->shadowType = ShadowMode::All;
		obj->HitPoints = 24;
		obj->hitEffect = HIT_BLOOD;
		obj->pivotLength = 50;
		obj->radius = 102;
		obj->intelligent = true;
		obj->ZoneType = ZoneType::HumanClassic;
		
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
		obj->shadowType = ShadowMode::All;
		obj->HitPoints = 24;
		obj->hitEffect = HIT_BLOOD;
		obj->pivotLength = 50;
		obj->radius = 102;
		obj->explodableMeshbits = 0x4000;
		obj->intelligent = true;
		obj->ZoneType = ZoneType::HumanClassic;
		
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
		obj->shadowType = ShadowMode::All;
		obj->HitPoints = 24;
		obj->hitEffect = HIT_BLOOD;
		obj->pivotLength = 50;
		obj->radius = 102;
		obj->explodableMeshbits = 0x4000;
		obj->intelligent = true;
		obj->ZoneType = ZoneType::HumanClassic;
		
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
		obj->shadowType = ShadowMode::All;
		obj->HitPoints = 100;
		obj->hitEffect = HIT_RICOCHET;
		obj->pivotLength = 200;
		obj->radius = 512;
		obj->intelligent = true;
		obj->waterCreature = true;
		obj->ZoneType = ZoneType::Flyer;
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
		obj->shadowType = ShadowMode::All;
		obj->HitPoints = 35;
		obj->hitEffect = HIT_BLOOD;
		obj->pivotLength = 50;
		obj->radius = 102;
		obj->biteOffset = 0;
		obj->intelligent = true;
		obj->ZoneType = ZoneType::HumanClassic;
		
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
		obj->shadowType = ShadowMode::All;
		obj->HitPoints = 40;
		obj->hitEffect = HIT_BLOOD;
		obj->pivotLength = 50;
		obj->radius = 341;
		obj->intelligent = true;
		obj->ZoneType = ZoneType::Basic;
		
		g_Level.Bones[obj->boneIndex + 6 * 4] |= ROT_Y;
		g_Level.Bones[obj->boneIndex + 19 * 4] |= ROT_Y;
	}

	obj = &Objects[ID_DOBERMAN];
	if (obj->loaded)
	{
		obj->initialise = InitialiseDoberman;
		obj->collision = CreatureCollision;
		obj->control = DobermanControl;
		obj->shadowType = ShadowMode::All;
		obj->HitPoints = 18;
		obj->hitEffect = HIT_BLOOD;
		obj->pivotLength = 50;
		obj->radius = 256;
		obj->intelligent = true;
		obj->ZoneType = ZoneType::Basic;
		
		g_Level.Bones[obj->boneIndex + 19 * 4] |= ROT_Y;
	}

	obj = &Objects[ID_HUSKIE];
	if (obj->loaded)
	{
		obj->initialise = InitialiseTr5Dog;
		obj->collision = CreatureCollision;
		obj->control = Tr5DogControl;
		obj->shadowType = ShadowMode::All;
		obj->HitPoints = 24;
		obj->hitEffect = HIT_BLOOD;
		obj->pivotLength = 50;
		obj->radius = 256;
		obj->intelligent = true;
		obj->ZoneType = ZoneType::Basic;
		
		g_Level.Bones[obj->boneIndex + 19 * 4] |= ROT_Y;
	}

	obj = &Objects[ID_REAPER];
	if (obj->loaded)
	{
		obj->initialise = InitialiseReaper;
		obj->collision = CreatureCollision;
		obj->control = ReaperControl;
		obj->shadowType = ShadowMode::All;
		obj->HitPoints = 10;
		obj->hitEffect = HIT_BLOOD;
		obj->pivotLength = 50;
		obj->radius = 102;
		obj->intelligent = true;
		obj->waterCreature = true;
		obj->ZoneType = ZoneType::Flyer;
	}

	obj = &Objects[ID_MAFIA2];
	if (obj->loaded)
	{
		obj->biteOffset = 7;
		obj->initialise = InitialiseMafia2;
		obj->collision = CreatureCollision;
		obj->control = Mafia2Control;
		obj->shadowType = ShadowMode::All;
		obj->HitPoints = 26;
		obj->hitEffect = HIT_BLOOD;
		obj->pivotLength = 50;
		obj->radius = 102;
		obj->explodableMeshbits = 0x4000;
		obj->intelligent = true;
		obj->ZoneType = ZoneType::HumanClassic;
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
		obj->shadowType = ShadowMode::All;
		obj->HitPoints = 60;
		obj->hitEffect = HIT_BLOOD;
		obj->pivotLength = 50;
		obj->radius = 102;
		obj->intelligent = true;
		obj->ZoneType = ZoneType::HumanClassic;
		
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
		obj->shadowType = ShadowMode::All;
		obj->HitPoints = 60;
		obj->hitEffect = HIT_BLOOD;
		obj->pivotLength = 50;
		obj->radius = 102;
		obj->intelligent = true;
		obj->ZoneType = ZoneType::HumanClassic;
		
		g_Level.Bones[obj->boneIndex + 6 * 4] |= ROT_Y;
		g_Level.Bones[obj->boneIndex + 6 * 4] |= ROT_X;
		g_Level.Bones[obj->boneIndex + 7 * 4] |= ROT_Y;
		g_Level.Bones[obj->boneIndex + 7 * 4] |= ROT_X;
	}

	obj = &Objects[ID_CYBORG];
	if (obj->loaded)
	{
		obj->biteOffset = 5;
		obj->initialise = InitialiseCyborg;
		obj->collision = CreatureCollision;
		obj->control = CyborgControl;
		obj->shadowType = ShadowMode::All;
		obj->HitPoints = 50;
		obj->hitEffect = HIT_RICOCHET;
		obj->pivotLength = 50;
		obj->radius = 102;
		obj->intelligent = true;
		obj->undead = true;
		obj->ZoneType = ZoneType::HumanClassic;
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
		obj->shadowType = ShadowMode::All;
		obj->HitPoints = 35;
		obj->hitEffect = HIT_BLOOD;
		obj->pivotLength = 50;
		obj->radius = 102;
		obj->explodableMeshbits = 0x4000;
		obj->intelligent = true;
		
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
		obj->shadowType = ShadowMode::All;
		obj->HitPoints = 24;
		obj->hitEffect = HIT_RICOCHET;
		obj->pivotLength = 50;
		obj->radius = 128;
		obj->explodableMeshbits = 4;
		obj->intelligent = true;
		obj->undead = true;
		obj->ZoneType = ZoneType::HumanClassic;
		
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
		obj->shadowType = ShadowMode::All;
		obj->HitPoints = 30;
		obj->hitEffect = HIT_RICOCHET;
		obj->pivotLength = 50;
		obj->radius = 102;
		obj->biteOffset = 1024;
		obj->intelligent = true;
		obj->undead = true;
		obj->ZoneType = ZoneType::Basic;
		
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
		obj->shadowType = ShadowMode::All;
		obj->HitPoints = 12;
		obj->hitEffect = HIT_BLOOD;
		obj->pivotLength = 20;
		obj->radius = 102;
		obj->intelligent = true;
		obj->meshSwapSlot = ID_MESHSWAP_IMP;
		obj->ZoneType = ZoneType::Basic;
		
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
		obj->shadowType = ShadowMode::All;
		obj->radius = 256;
		obj->HitPoints = 16;
		obj->pivotLength = 20;
		obj->intelligent = true;
		obj->ZoneType = ZoneType::Flyer;
		
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
		obj->shadowType = ShadowMode::All;
		obj->HitPoints = 100;
		obj->hitEffect = HIT_BLOOD;
		obj->pivotLength = 20;
		obj->radius = 341;
		obj->intelligent = true;
		obj->ZoneType = ZoneType::Basic;
		
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
		obj->shadowType = ShadowMode::All;
		obj->HitPoints = 100;
		obj->hitEffect = HIT_BLOOD;
		obj->pivotLength = 20;
		obj->radius = 256;
		obj->intelligent = true;
		obj->waterCreature = true;
		obj->ZoneType = ZoneType::Basic;
		
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
		obj->shadowType = ShadowMode::All;
		obj->HitPoints = 100;
		obj->hitEffect = HIT_SMOKE;
		obj->pivotLength = 20;
		obj->radius = 256;
		obj->intelligent = true;
		obj->ZoneType = ZoneType::HumanClassic;
		
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
		obj->shadowType = ShadowMode::All;
		obj->HitPoints = 20;
		obj->hitEffect = HIT_BLOOD;
		obj->pivotLength = 50;
		obj->radius = 102;
		obj->intelligent = true;
		obj->ZoneType = ZoneType::HumanClassic;
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
			obj->shadowType = ShadowMode::All;
			obj->HitPoints = 300;
			obj->hitEffect = HIT_SMOKE;
			obj->pivotLength = 50;
			obj->radius = 256;
			obj->intelligent = true;
			obj->meshSwapSlot = ID_MESHSWAP_ROMAN_GOD1 + i;
			obj->ZoneType = ZoneType::HumanClassic;
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
		obj->usingDrawAnimatingItem = false;
		obj->undead = true;
		obj->hitEffect = HIT_RICOCHET;
	}

	obj = &Objects[ID_AUTOGUN];
	if (obj->loaded)
	{
		obj->initialise = InitialiseAutoGuns;
		obj->control = AutoGunsControl;
		obj->hitEffect = HIT_RICOCHET;
		g_Level.Bones[obj->boneIndex + 6 * 4] |= ROT_Y;
		g_Level.Bones[obj->boneIndex + 6 * 4] |= ROT_X;
		g_Level.Bones[obj->boneIndex + 8 * 4] |= ROT_Y;
	}

	obj = &Objects[ID_GUNSHIP];
	if (obj->loaded)
	{
		obj->control = ControlGunShip;
		obj->hitEffect = HIT_RICOCHET;
		g_Level.Bones[obj->boneIndex + 0] |= ROT_Y;
		g_Level.Bones[obj->boneIndex + 4] |= ROT_X;
	}

	// TR5 SUBMARINE
	obj = &Objects[ID_ATTACK_SUB];
	if (obj->loaded)
	{
		obj->initialise = InitialiseSubmarine;
		obj->control = SubmarineControl;
		obj->ZoneType = ZoneType::Basic;
		obj->hitEffect = HIT_RICOCHET;
		obj->shadowType = ShadowMode::All;
		obj->HitPoints = 100;
	}
}

static void StartObject(ObjectInfo *obj)
{
	InitFlare(obj, ID_FLARE_ITEM);

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
	}

	obj = &Objects[ID_ELECTRICAL_LIGHT];
	if (obj->loaded)
	{
		obj->control = ElectricalLightControl;
		obj->drawRoutine = nullptr;
		obj->usingDrawAnimatingItem = false;
		
	}

	obj = &Objects[ID_PULSE_LIGHT];
	if (obj->loaded)
	{
		obj->control = PulseLightControl;
		obj->drawRoutine = nullptr;
		obj->usingDrawAnimatingItem = false;
	}

	obj = &Objects[ID_STROBE_LIGHT];
	if (obj->loaded)
	{
		obj->control = StrobeLightControl;
	}

	obj = &Objects[ID_COLOR_LIGHT];
	if (obj->loaded)
	{
		obj->control = ColorLightControl;
		obj->drawRoutine = nullptr;
		obj->usingDrawAnimatingItem = false;
	}

	obj = &Objects[ID_BLINKING_LIGHT];
	if (obj->loaded)
	{
		obj->control = BlinkingLightControl;
		obj->drawRoutine = nullptr;
		
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
		
	}

	obj = &Objects[ID_SMOKE_EMITTER_WHITE];
	if (obj->loaded)
	{
		obj->initialise = InitialiseSmokeEmitter;
		obj->control = SmokeEmitterControl;
		obj->drawRoutine = nullptr;
		obj->usingDrawAnimatingItem = false;
	}

	obj = &Objects[ID_SMOKE_EMITTER];
	if (obj->loaded)
	{
		obj->initialise = InitialiseSmokeEmitter;
		obj->control = SmokeEmitterControl;
		obj->drawRoutine = nullptr;
		obj->usingDrawAnimatingItem = false;
	}

	obj = &Objects[ID_TELEPORTER];
	if (obj->loaded)
	{
		obj->initialise = InitialiseTeleporter;
		obj->control = ControlTeleporter;
		obj->drawRoutine = nullptr;
	}

	obj = &Objects[ID_LARA_START_POS];
	if (obj->loaded)
	{
		obj->drawRoutine = nullptr;
	}

	obj = &Objects[ID_HIGH_OBJECT1];
	if (obj->loaded)
	{
		obj->initialise = InitialiseHighObject1;
		obj->control = ControlHighObject1;
		obj->collision = ObjectCollision;
	}

	obj = &Objects[ID_GEN_SLOT1];
	if (obj->loaded)
	{
		obj->control = GenSlot1Control;
	}

	for (int objectNumber = ID_AI_GUARD; objectNumber <= ID_AI_X2; objectNumber++)
	{
		obj = &Objects[objectNumber];
		if (obj->loaded)
		{
			obj->drawRoutine = nullptr;
			obj->collision = AIPickupCollision;
			obj->HitPoints = 0;
		}
	}
}

static void StartTrap(ObjectInfo *obj)
{
	obj = &Objects[ID_ZIPLINE_HANDLE];
	if (obj->loaded)
	{
		obj->initialise = InitialiseZipLine;
		obj->collision = ZipLineCollision;
		obj->control = ControlZipLine;
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

	obj = &Objects[ID_ELECTRICAL_CABLES];
	if (obj->loaded)
	{
		obj->control = ElectricityWiresControl;
	}

	obj = &Objects[ID_ROME_HAMMER];
	if (obj->loaded)
	{
		obj->initialise = InitialiseRomeHammer;
		obj->collision = GenericSphereBoxCollision;
		obj->control = AnimatingControl;
		obj->hitEffect = HIT_RICOCHET;
	}

	obj = &Objects[ID_FALLING_CEILING];
	if (obj->loaded)
	{
		obj->collision = TrapCollision;
		obj->control = FallingCeilingControl;
	}

	obj = &Objects[ID_ROLLINGBALL];
	if (obj->loaded)
	{
		obj->collision = RollingBallCollision;
		obj->control = RollingBallControl;
		obj->hitEffect = HIT_RICOCHET;
		obj->shadowType = ShadowMode::All;
	}
	
	obj = &Objects[ID_CLASSIC_ROLLING_BALL];
	if (obj->loaded)
	{
		obj->initialise = InitialiseClassicRollingBall;
		obj->control = ClassicRollingBallControl;
		obj->collision = ClassicRollingBallCollision;
		obj->hitEffect = HIT_RICOCHET;
	}

	obj = &Objects[ID_BIG_ROLLING_BALL];
	if (obj->loaded)
	{
		obj->collision = ClassicRollingBallCollision;
		obj->control = ClassicRollingBallControl;
		obj->initialise = InitialiseClassicRollingBall;
		obj->hitEffect = HIT_RICOCHET;
	}

	obj = &Objects[ID_GEN_SLOT3];
	if (obj->loaded)
	{
		obj->initialise = InitialiseGenSlot3;
		obj->collision = HybridCollision;
		obj->control = AnimatingControl;
	}

	obj = &Objects[ID_GEN_SLOT4];
	if (obj->loaded)
	{
		//obj->initialise = InitialiseGenSlot4;
		//obj->control = GenSlot4Control;
	}

	obj = &Objects[ID_EXPLOSION];
	if (obj->loaded)
	{
		obj->initialise = InitialiseExplosion;
		obj->control = ExplosionControl;
		obj->drawRoutine = nullptr;
		obj->usingDrawAnimatingItem = false;
	}
}

static void StartSwitch(ObjectInfo *obj)
{
	obj = &Objects[ID_RAISING_COG];
	if (obj->loaded)
	{
		obj->initialise = InitialiseRaisingCog;
		obj->control = RaisingCogControl;
		obj->hitEffect = HIT_RICOCHET;
	}

	obj = &Objects[ID_CROWDOVE_SWITCH];
	if (obj->loaded)
	{
		obj->initialise = InitialiseCrowDoveSwitch;
		obj->collision = CrowDoveSwitchCollision;
		obj->control = CrowDoveSwitchControl;
		obj->hitEffect = HIT_RICOCHET;
	}

	obj = &Objects[ID_WRECKING_BALL];
	if (obj->loaded)
	{
		obj->initialise = InitialiseWreckingBall;
		obj->collision = WreckingBallCollision;
		obj->control = WreckingBallControl;
	}
}

static void StartShatter(ObjectInfo *obj)
{
	for (int i = ID_SMASH_OBJECT1; i <= ID_SMASH_OBJECT16; i++)
		InitSmashObject(obj, i);
}

static void StartProjectiles(ObjectInfo *obj)
{
	InitProjectile(obj, MissileControl, ID_BUBBLES, true);
	InitProjectile(obj, MissileControl, ID_IMP_ROCK, true);
	InitProjectile(obj, TorpedoControl, ID_TORPEDO);
	InitProjectile(obj, GrenadeControl, ID_GRENADE);
	InitProjectile(obj, RocketControl, ID_ROCKET);
	InitProjectile(obj, HarpoonBoltControl, ID_HARPOON);
	InitProjectile(obj, CrossbowBoltControl, ID_CROSSBOW_BOLT);
}

static void StartPickup(ObjectInfo *obj)
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
	InitPickup(obj, ID_DIARY_ITEM);
}

static ObjectInfo *objToInit;
void InitialiseTR5Objects()
{
	StartEntity(objToInit);
	StartObject(objToInit);
	StartTrap(objToInit);
	StartPickup(objToInit);
	StartSwitch(objToInit);
	StartShatter(objToInit);
	StartProjectiles(objToInit);
}

void AllocTR5Objects()
{
	ZeroMemory(Bats, NUM_BATS * sizeof(BatData));
	ZeroMemory(Spiders, NUM_SPIDERS * sizeof(SpiderData));
	ZeroMemory(Rats, NUM_RATS * sizeof(RatData));
}
