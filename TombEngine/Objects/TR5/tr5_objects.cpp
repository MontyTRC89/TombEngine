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
#include "Objects/TR5/Entity/AutoGun.h"			 // OK
#include "Objects/TR5/Entity/tr5_brownbeast.h"	 // OK
#include "Objects/TR5/Entity/tr5_chef.h"		 // OK
#include "Objects/TR5/Entity/tr5_cyborg.h"		 // OK
#include "Objects/TR5/Entity/tr5_doberman.h"	 // OK
#include "Objects/TR5/Entity/tr5_dog.h"			 // OK
#include "Objects/TR5/Entity/tr5_ghost.h"		 // OK
#include "Objects/TR5/Entity/tr5_gladiator.h"	 // OK
#include "Objects/TR5/Entity/tr5_guard.h"		 // OK
#include "Objects/TR5/Entity/tr5_gunship.h"		 // OK
#include "Objects/TR5/Entity/tr5_hydra.h"		 // OK
#include "Objects/TR5/Entity/tr5_imp.h"			 // OK
#include "Objects/TR5/Entity/tr5_lagoon_witch.h" // OK
#include "Objects/TR5/Entity/tr5_larson.h"		 // OK
#include "Objects/TR5/Entity/tr5_laser_head.h"	 // OK
#include "Objects/TR5/Entity/tr5_lion.h"		 // OK
#include "Objects/TR5/Entity/tr5_reaper.h"		 // OK
#include "Objects/TR5/Entity/tr5_roman_statue.h" // OK
#include "Objects/TR5/Entity/tr5_submarine.h"	 // OK
#include "Objects/TR5/Entity/tr5_twogun.h"		 // OK
#include "Objects/TR5/Entity/tr5_willowwisp.h"	 // OK

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
		obj->radius = 102;
		obj->pivotLength = 50;
		obj->intelligent = true;
		obj->ZoneType = ZoneType::Human;
		obj->SetBoneRotationFlags(6, ROT_X | ROT_Y);
		obj->SetBoneRotationFlags(13, ROT_X | ROT_Y);
		obj->SetupHitEffect();
	}

	obj = &Objects[ID_SWAT];
	if (obj->loaded)
	{
		obj->initialise = InitialiseGuard;
		obj->collision = CreatureCollision;
		obj->control = GuardControl;
		obj->shadowType = ShadowMode::All;
		obj->biteOffset = 0;
		obj->HitPoints = 24;
		obj->pivotLength = 50;
		obj->radius = 102;
		obj->intelligent = true;
		obj->explodableMeshbits = 0x4000;
		obj->ZoneType = ZoneType::HumanJump;
		obj->SetBoneRotationFlags(6, ROT_X | ROT_Y);
		obj->SetBoneRotationFlags(13, ROT_X | ROT_Y);
		obj->SetupHitEffect();
	}

	obj = &Objects[ID_GUARD1];
	if (obj->loaded)
	{
		// Object required.
		if (Objects[ID_SWAT].loaded)
			obj->animIndex = Objects[ID_SWAT].animIndex;

		obj->initialise = InitialiseGuard;
		obj->collision = CreatureCollision;
		obj->control = GuardControl;
		obj->shadowType = ShadowMode::All;
		obj->biteOffset = 4;
		obj->HitPoints = 24;
		obj->radius = 102;
		obj->pivotLength = 50;
		obj->explodableMeshbits = 0x4000;
		obj->intelligent = true;
		obj->ZoneType = ZoneType::HumanJump;
		obj->SetBoneRotationFlags(6, ROT_X | ROT_Y);
		obj->SetBoneRotationFlags(13, ROT_X | ROT_Y);
		obj->SetupHitEffect();
	}

	obj = &Objects[ID_SWAT_PLUS];
	if (obj->loaded)
	{
		if (!Objects[ID_SWAT].loaded)
			obj->animIndex = Objects[ID_GUARD1].animIndex;
		else
			obj->animIndex = Objects[ID_SWAT].animIndex;

		obj->initialise = InitialiseGuard;
		obj->collision = CreatureCollision;
		obj->control = GuardControl;
		obj->shadowType = ShadowMode::All;
		obj->biteOffset = 0;
		obj->HitPoints = 24;
		obj->pivotLength = 50;
		obj->radius = 102;
		obj->intelligent = true;
		obj->ZoneType = ZoneType::HumanJump;
		obj->SetBoneRotationFlags(6, ROT_X | ROT_Y);
		obj->SetBoneRotationFlags(13, ROT_X | ROT_Y);
		obj->SetupHitEffect();
	}

	obj = &Objects[ID_MAFIA];
	if (obj->loaded)
	{
		if (!Objects[ID_SWAT].loaded)
			obj->animIndex = Objects[ID_GUARD1].animIndex;
		else
			obj->animIndex = Objects[ID_SWAT].animIndex;

		obj->initialise = InitialiseGuard;
		obj->collision = CreatureCollision;
		obj->control = GuardControl;
		obj->shadowType = ShadowMode::All;
		obj->biteOffset = 0;
		obj->HitPoints = 24;
		obj->pivotLength = 50;
		obj->radius = 102;
		obj->explodableMeshbits = 0x4000;
		obj->intelligent = true;
		obj->ZoneType = ZoneType::HumanJump;
		obj->SetBoneRotationFlags(6, ROT_X | ROT_Y);
		obj->SetBoneRotationFlags(13, ROT_X | ROT_Y);
		obj->SetupHitEffect();
	}

	obj = &Objects[ID_SCIENTIST];
	if (obj->loaded)
	{
		if (!Objects[ID_SWAT].loaded)
			obj->animIndex = Objects[ID_GUARD1].animIndex;
		else
			obj->animIndex = Objects[ID_SWAT].animIndex;

		obj->initialise = InitialiseGuard;
		obj->control = GuardControl;
		obj->collision = CreatureCollision;
		obj->shadowType = ShadowMode::All;
		obj->HitPoints = 24;
		obj->pivotLength = 50;
		obj->radius = 102;
		obj->intelligent = true;
		obj->ZoneType = ZoneType::HumanJump;
		obj->SetBoneRotationFlags(6, ROT_X | ROT_Y);
		obj->SetBoneRotationFlags(13, ROT_X | ROT_Y);
		obj->SetupHitEffect();
	}

	obj = &Objects[ID_GUARD2];
	if (obj->loaded)
	{
		if (!Objects[ID_SWAT].loaded)
			obj->animIndex = Objects[ID_GUARD1].animIndex;
		else
			obj->animIndex = Objects[ID_SWAT].animIndex;

		obj->initialise = InitialiseGuard;
		obj->control = GuardControl;
		obj->collision = CreatureCollision;
		obj->shadowType = ShadowMode::All;
		obj->biteOffset = 4;
		obj->HitPoints = 24;
		obj->pivotLength = 50;
		obj->radius = 102;
		obj->explodableMeshbits = 0x4000;
		obj->intelligent = true;
		obj->ZoneType = ZoneType::Human;
		obj->SetBoneRotationFlags(6, ROT_X | ROT_Y);
		obj->SetBoneRotationFlags(13, ROT_X | ROT_Y);
		obj->SetupHitEffect();
	}

	obj = &Objects[ID_GUARD3];
	if (obj->loaded)
	{
		if (!Objects[ID_SWAT].loaded)
			obj->animIndex = Objects[ID_GUARD1].animIndex;
		else
			obj->animIndex = Objects[ID_SWAT].animIndex;

		obj->initialise = InitialiseGuard;
		obj->control = GuardControl;
		obj->collision = CreatureCollision;
		obj->shadowType = ShadowMode::All;
		obj->biteOffset = 4;
		obj->HitPoints = 24;
		obj->pivotLength = 50;
		obj->radius = 102;
		obj->explodableMeshbits = 0x4000;
		obj->intelligent = true;
		obj->ZoneType = ZoneType::Human;
		obj->SetBoneRotationFlags(6, ROT_X | ROT_Y);
		obj->SetBoneRotationFlags(13, ROT_X | ROT_Y);
		obj->SetupHitEffect();
	}

	obj = &Objects[ID_ATTACK_SUB];
	if (obj->loaded)
	{
		obj->initialise = InitialiseSubmarine;
		obj->collision = CreatureCollision;
		obj->control = SubmarineControl;
		obj->shadowType = ShadowMode::All;
		obj->HitPoints = 100;
		obj->pivotLength = 200;
		obj->radius = 512;
		obj->intelligent = true;
		obj->waterCreature = true;
		obj->undead = true;
		obj->ZoneType = ZoneType::Water;
		obj->SetBoneRotationFlags(0, ROT_X);
		obj->SetBoneRotationFlags(1, ROT_X);
	}

	obj = &Objects[ID_CHEF];
	if (obj->loaded)
	{
		obj->initialise = InitialiseChef;
		obj->control = ControlChef;
		obj->collision = CreatureCollision;
		obj->shadowType = ShadowMode::All;
		obj->HitPoints = 35;
		obj->pivotLength = 50;
		obj->radius = 102;
		obj->biteOffset = 0;
		obj->intelligent = true;
		obj->ZoneType = ZoneType::Human;
		obj->SetBoneRotationFlags(6, ROT_X | ROT_Y);
		obj->SetBoneRotationFlags(13, ROT_X | ROT_Y);
		obj->SetupHitEffect();
	}

	obj = &Objects[ID_LION];
	if (obj->loaded)
	{
		obj->initialise = InitialiseLion;
		obj->collision = CreatureCollision;
		obj->control = LionControl;
		obj->shadowType = ShadowMode::All;
		obj->HitPoints = 40;
		obj->pivotLength = 50;
		obj->radius = 341;
		obj->intelligent = true;
		obj->SetBoneRotationFlags(6, ROT_Y);
		obj->SetBoneRotationFlags(19, ROT_Y);
		obj->SetupHitEffect();
	}

	obj = &Objects[ID_DOBERMAN];
	if (obj->loaded)
	{
		obj->initialise = InitialiseDoberman;
		obj->collision = CreatureCollision;
		obj->control = DobermanControl;
		obj->shadowType = ShadowMode::All;
		obj->HitPoints = 18;
		obj->pivotLength = 50;
		obj->radius = 256;
		obj->intelligent = true;
		obj->SetBoneRotationFlags(19, ROT_Y);
		obj->SetupHitEffect();
	}

	obj = &Objects[ID_HUSKIE];
	if (obj->loaded)
	{
		obj->initialise = InitialiseTr5Dog;
		obj->collision = CreatureCollision;
		obj->control = Tr5DogControl;
		obj->shadowType = ShadowMode::All;
		obj->HitPoints = 24;
		obj->pivotLength = 50;
		obj->radius = 256;
		obj->intelligent = true;
		obj->SetBoneRotationFlags(19, ROT_Y);
		obj->SetupHitEffect();
	}

	obj = &Objects[ID_REAPER];
	if (obj->loaded)
	{
		obj->initialise = InitialiseReaper;
		obj->collision = CreatureCollision;
		obj->control = ReaperControl;
		obj->shadowType = ShadowMode::All;
		obj->HitPoints = 10;
		obj->pivotLength = 50;
		obj->radius = 102;
		obj->intelligent = true;
		obj->waterCreature = true;
		obj->ZoneType = ZoneType::Water;
		obj->SetupHitEffect();
	}

	obj = &Objects[ID_MAFIA2];
	if (obj->loaded)
	{
		obj->initialise = InitialiseMafia2;
		obj->collision = CreatureCollision;
		obj->control = Mafia2Control;
		obj->shadowType = ShadowMode::All;
		obj->biteOffset = 7;
		obj->HitPoints = 26;
		obj->pivotLength = 50;
		obj->radius = 102;
		obj->explodableMeshbits = 0x4000;
		obj->intelligent = true;
		obj->ZoneType = ZoneType::Human;
		obj->meshSwapSlot = ID_MESHSWAP_MAFIA2;
		obj->SetBoneRotationFlags(6, ROT_X | ROT_Y);
		obj->SetBoneRotationFlags(13, ROT_X | ROT_Y);
		obj->SetupHitEffect();
	}

	obj = &Objects[ID_PIERRE];
	if (obj->loaded)
	{
		obj->initialise = InitialiseLarson;
		obj->collision = CreatureCollision;
		obj->control = LarsonControl;
		obj->shadowType = ShadowMode::All;
		obj->biteOffset = 1;
		obj->HitPoints = 60;
		obj->pivotLength = 50;
		obj->radius = 102;
		obj->intelligent = true;
		obj->ZoneType = ZoneType::Human;
		obj->SetBoneRotationFlags(6, ROT_X | ROT_Y);
		obj->SetBoneRotationFlags(7, ROT_X | ROT_Y);
		obj->SetupHitEffect();
	}

	obj = &Objects[ID_LARSON];
	if (obj->loaded)
	{
		obj->initialise = InitialiseLarson;
		obj->collision = CreatureCollision;
		obj->control = LarsonControl;
		obj->shadowType = ShadowMode::All;
		obj->biteOffset = 3;
		obj->HitPoints = 60;
		obj->pivotLength = 50;
		obj->radius = 102;
		obj->intelligent = true;
		obj->ZoneType = ZoneType::Human;
		obj->SetBoneRotationFlags(6, ROT_X | ROT_Y);
		obj->SetBoneRotationFlags(7, ROT_X | ROT_Y);
		obj->SetupHitEffect();
	}

	obj = &Objects[ID_CYBORG];
	if (obj->loaded)
	{
		obj->initialise = InitialiseCyborg;
		obj->collision = CreatureCollision;
		obj->control = CyborgControl;
		obj->shadowType = ShadowMode::All;
		obj->biteOffset = 5;
		obj->HitPoints = 50;
		obj->pivotLength = 50;
		obj->radius = 102;
		obj->intelligent = true;
		obj->undead = true;
		obj->ZoneType = ZoneType::Human;
		obj->meshSwapSlot = ID_MESHSWAP_HITMAN;
		obj->SetBoneRotationFlags(6, ROT_X | ROT_Y);
		obj->SetBoneRotationFlags(13, ROT_X | ROT_Y);
		obj->SetupHitEffect(true);
	}

	obj = &Objects[ID_SNIPER];
	if (obj->loaded)
	{
		obj->initialise = InitialiseSniper;
		obj->collision = CreatureCollision;
		obj->control = SniperControl;
		obj->shadowType = ShadowMode::All;
		obj->biteOffset = 6;
		obj->HitPoints = 35;
		obj->pivotLength = 50;
		obj->radius = 102;
		obj->explodableMeshbits = 0x4000;
		obj->intelligent = true;
		obj->SetBoneRotationFlags(6, ROT_X | ROT_Y);
		obj->SetBoneRotationFlags(13, ROT_X | ROT_Y);
		obj->SetupHitEffect();
	}

	obj = &Objects[ID_GUARD_LASER];
	if (obj->loaded)
	{
		obj->initialise = InitialiseHeavyGuard;
		obj->collision = CreatureCollision;
		obj->control = HeavyGuardControl;
		obj->shadowType = ShadowMode::All;
		obj->HitRoutine = HeavyGuardHit;
		obj->biteOffset = 0;
		obj->HitPoints = 24;
		obj->pivotLength = 50;
		obj->radius = 128;
		obj->intelligent = true;
		obj->undead = true;
		obj->SetBoneRotationFlags(0, ROT_X | ROT_Y);
		obj->SetBoneRotationFlags(1, ROT_X | ROT_Y);
		obj->SetupHitEffect(true);
	}

	obj = &Objects[ID_HYDRA];
	if (obj->loaded)
	{
		obj->initialise = InitialiseHydra;
		obj->collision = CreatureCollision;
		obj->control = HydraControl;
		obj->shadowType = ShadowMode::All;
		obj->HitPoints = 30;
		obj->pivotLength = 50;
		obj->radius = 102;
		obj->biteOffset = 1024;
		obj->intelligent = true;
		obj->undead = true;
		obj->SetBoneRotationFlags(0, ROT_X | ROT_Y);
		obj->SetBoneRotationFlags(8, ROT_X | ROT_Y);
		obj->SetupHitEffect();
	}

	obj = &Objects[ID_IMP];
	if (obj->loaded)
	{
		obj->initialise = InitialiseImp;
		obj->collision = CreatureCollision;
		obj->control = ImpControl;
		obj->shadowType = ShadowMode::All;
		obj->biteOffset = 256;
		obj->HitPoints = 12;
		obj->pivotLength = 20;
		obj->radius = 102;
		obj->intelligent = true;
		obj->meshSwapSlot = ID_MESHSWAP_IMP;
		obj->SetBoneRotationFlags(4, ROT_X | ROT_Z);
		obj->SetBoneRotationFlags(9, ROT_X | ROT_Z);
		obj->SetupHitEffect();
	}

	obj = &Objects[ID_WILLOWISP];
	if (obj->loaded)
	{
		obj->initialise = InitialiseLightingGuide;
		//obj->control = ControlLightingGuide;
		obj->drawRoutine = nullptr;
		obj->shadowType = ShadowMode::All;
		obj->biteOffset = 256;
		obj->radius = 256;
		obj->HitPoints = NOT_TARGETABLE;
		obj->pivotLength = 20;
		obj->intelligent = true;
		obj->ZoneType = ZoneType::Flyer;
		obj->SetBoneRotationFlags(4, ROT_X | ROT_Z);
		obj->SetBoneRotationFlags(9, ROT_X | ROT_Z);
		obj->SetupHitEffect();
	}

	obj = &Objects[ID_BROWN_BEAST];
	if (obj->loaded)
	{
		obj->initialise = InitialiseBrownBeast;
		obj->collision = CreatureCollision;
		obj->control = ControlBrowsBeast;
		obj->shadowType = ShadowMode::All;
		obj->biteOffset = 256;
		obj->HitPoints = 100;
		obj->pivotLength = 20;
		obj->radius = 341;
		obj->intelligent = true;
		obj->SetBoneRotationFlags(4, ROT_X | ROT_Z);
		obj->SetBoneRotationFlags(9, ROT_X | ROT_Z);
		obj->SetupHitEffect();
	}

	obj = &Objects[ID_LAGOON_WITCH];
	if (obj->loaded)
	{
		obj->initialise = InitialiseLagoonWitch;
		obj->collision = CreatureCollision;
		obj->control = LagoonWitchControl;
		obj->shadowType = ShadowMode::All;
		obj->biteOffset = 256;
		obj->HitPoints = 100;
		obj->pivotLength = 20;
		obj->radius = 256;
		obj->intelligent = true;
		obj->waterCreature = true;
		obj->ZoneType = ZoneType::Water;
		obj->SetBoneRotationFlags(4, ROT_X | ROT_Z);
		obj->SetBoneRotationFlags(9, ROT_X | ROT_Z);
		obj->SetupHitEffect();
	}

	obj = &Objects[ID_INVISIBLE_GHOST];
	if (obj->loaded)
	{
		obj->initialise = InitialiseInvisibleGhost;
		obj->collision = CreatureCollision;
		obj->control = InvisibleGhostControl;
		obj->shadowType = ShadowMode::All;
		obj->biteOffset = 256;
		obj->HitPoints = NOT_TARGETABLE;
		obj->pivotLength = 20;
		obj->radius = 256;
		obj->intelligent = true;
		obj->ZoneType = ZoneType::Human;
		obj->SetBoneRotationFlags(6, ROT_X | ROT_Y);
		obj->SetBoneRotationFlags(8, ROT_X | ROT_Y);
		obj->SetupHitEffect();
	}

	obj = &Objects[ID_RATS_EMITTER];
	if (obj->loaded)
	{
		obj->drawRoutine = nullptr;
		obj->initialise = InitialiseLittleRats;
		obj->control = LittleRatsControl;
		obj->usingDrawAnimatingItem = false;
	}

	obj = &Objects[ID_BATS_EMITTER];
	if (obj->loaded)
	{
		obj->drawRoutine = nullptr;
		obj->initialise = InitialiseLittleBats;
		obj->control = LittleBatsControl;
		obj->usingDrawAnimatingItem = false;
	}

	obj = &Objects[ID_SPIDERS_EMITTER];
	if (obj->loaded)
	{
		obj->drawRoutine = nullptr;
		obj->initialise = InitialiseSpiders;
		obj->control = SpidersEmitterControl;
		obj->usingDrawAnimatingItem = false;
	}

	obj = &Objects[ID_GLADIATOR];
	if (obj->loaded)
	{
		obj->initialise = InitialiseGladiator;
		obj->control = ControlGladiator;
		obj->collision = CreatureCollision;
		obj->shadowType = ShadowMode::All;
		obj->biteOffset = 0;
		obj->HitPoints = 20;
		obj->pivotLength = 50;
		obj->radius = 102;
		obj->intelligent = true;
		obj->ZoneType = ZoneType::Human;
		obj->SetBoneRotationFlags(6, ROT_X | ROT_Y);
		obj->SetBoneRotationFlags(13, ROT_X | ROT_Y);
		obj->SetupHitEffect();
	}

	for (int i = 0; i < 2; i++)
	{
		obj = &Objects[ID_ROMAN_GOD1 + i];
		if (obj->loaded)
		{
			obj->initialise = InitialiseRomanStatue;
			obj->collision = CreatureCollision;
			obj->control = RomanStatueControl;
			obj->HitRoutine = RomanStatueHit;
			obj->shadowType = ShadowMode::All;
			obj->biteOffset = 0;
			obj->HitPoints = 300;
			obj->pivotLength = 50;
			obj->radius = 256;
			obj->intelligent = true;
			obj->ZoneType = ZoneType::Human;
			obj->meshSwapSlot = ID_MESHSWAP_ROMAN_GOD1 + i;
			obj->SetBoneRotationFlags(6, ROT_X | ROT_Y);
			obj->SetBoneRotationFlags(13, ROT_X | ROT_Y);
			obj->SetupHitEffect(true);
		}
	}

	obj = &Objects[ID_LASERHEAD];
	if (obj->loaded)
	{
		obj->initialise = InitialiseGuardian;
		obj->collision = CreatureCollision;
		obj->control = ControlGuardian;
		obj->explodableMeshbits = 6;
		obj->usingDrawAnimatingItem = false;
		obj->undead = true;
		obj->nonLot = true;
		obj->SetupHitEffect(true);
	}

	obj = &Objects[ID_AUTOGUN];
	if (obj->loaded)
	{
		obj->initialise = InitialiseAutoGuns;
		obj->control = ControlAutoGun;
		obj->intelligent = true;
		obj->undead = true;
		obj->SetBoneRotationFlags(6, ROT_X | ROT_Y);
		obj->SetBoneRotationFlags(8, ROT_Y);
		obj->SetupHitEffect(true);
	}

	obj = &Objects[ID_GUNSHIP];
	if (obj->loaded)
	{
		obj->control = ControlGunShip;
		obj->SetBoneRotationFlags(0, ROT_Y);
		obj->SetBoneRotationFlags(1, ROT_X);
		obj->SetupHitEffect(true);
	}
}

static void StartObject(ObjectInfo *obj)
{
	InitFlare(obj, ID_FLARE_ITEM);

	for (int objectNumber = ID_SEARCH_OBJECT1; objectNumber <= ID_SEARCH_OBJECT4; objectNumber++)
		InitSearchObject(obj, objectNumber);

	for (int objectNumber = ID_PUSHABLE_OBJECT1; objectNumber <= ID_PUSHABLE_OBJECT10; objectNumber++)
		InitPushableObject(obj, objectNumber);

	obj = &Objects[ID_TWOBLOCK_PLATFORM];
	if (obj->loaded)
	{
		obj->initialise = InitialiseTwoBlocksPlatform;
		obj->control = TwoBlocksPlatformControl;
		obj->floor = TwoBlocksPlatformFloor;
		obj->ceiling = TwoBlocksPlatformCeiling;
		obj->floorBorder = TwoBlocksPlatformFloorBorder;
		obj->ceilingBorder = TwoBlocksPlatformCeilingBorder;
		obj->SetupHitEffect(true);
	}

	for (int objectNumber = ID_RAISING_BLOCK1; objectNumber <= ID_RAISING_BLOCK4; objectNumber++)
	{
		obj = &Objects[objectNumber];
		if (obj->loaded)
		{
			obj->initialise = InitialiseRaisingBlock;
			obj->control = ControlRaisingBlock;
			obj->floor = RaisingBlockFloor;
			obj->ceiling = RaisingBlockCeiling;
			obj->floorBorder = RaisingBlockFloorBorder;
			obj->ceilingBorder = RaisingBlockCeilingBorder;
			obj->SetupHitEffect(true);
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
		obj->SetupHitEffect(true);
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
		obj->SetupHitEffect(true);
	}

	obj = &Objects[ID_PROPELLER_H];
	if (obj->loaded)
	{
		obj->initialise = InitialiseVentilator;
		obj->control = VentilatorControl;
		obj->SetupHitEffect(true);
	}

	obj = &Objects[ID_PROPELLER_V];
	if (obj->loaded)
	{
		obj->initialise = InitialiseVentilator;
		obj->control = VentilatorControl;
		obj->SetupHitEffect(true);
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
		obj->SetupHitEffect(true);
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
		obj->shadowType = ShadowMode::All;
		obj->SetupHitEffect(true);
	}
	
	obj = &Objects[ID_CLASSIC_ROLLING_BALL];
	if (obj->loaded)
	{
		obj->initialise = InitialiseClassicRollingBall;
		obj->control = ClassicRollingBallControl;
		obj->collision = ClassicRollingBallCollision;
		obj->SetupHitEffect(true);
	}

	obj = &Objects[ID_BIG_ROLLING_BALL];
	if (obj->loaded)
	{
		obj->collision = ClassicRollingBallCollision;
		obj->control = ClassicRollingBallControl;
		obj->initialise = InitialiseClassicRollingBall;
		obj->SetupHitEffect(true);
	}

	obj = &Objects[ID_GEN_SLOT3];
	if (obj->loaded)
	{
		obj->initialise = InitialiseGenSlot3;
		obj->collision = HybridCollision;
		obj->control = AnimatingControl;
	}

	// TODO: Why commented? -- TokyoSU, 2022.12.24
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
		obj->SetupHitEffect(true);
	}

	obj = &Objects[ID_CROWDOVE_SWITCH];
	if (obj->loaded)
	{
		obj->initialise = InitialiseCrowDoveSwitch;
		obj->collision = CrowDoveSwitchCollision;
		obj->control = CrowDoveSwitchControl;
		obj->SetupHitEffect(true);
	}

	obj = &Objects[ID_WRECKING_BALL];
	if (obj->loaded)
	{
		obj->initialise = InitialiseWreckingBall;
		obj->collision = WreckingBallCollision;
		obj->control = WreckingBallControl;
		obj->SetupHitEffect(true);
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

void InitialiseTR5Objects()
{
	ObjectInfo* objectPtr = nullptr;
	StartEntity(objectPtr);
	StartObject(objectPtr);
	StartTrap(objectPtr);
	StartPickup(objectPtr);
	StartSwitch(objectPtr);
	StartShatter(objectPtr);
	StartProjectiles(objectPtr);
}

void AllocTR5Objects()
{
	ZeroMemory(Bats, NUM_BATS * sizeof(BatData));
	ZeroMemory(Spiders, NUM_SPIDERS * sizeof(SpiderData));
	ZeroMemory(Rats, NUM_RATS * sizeof(RatData));
}
