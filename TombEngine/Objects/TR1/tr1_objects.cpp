#include "framework.h"
#include "Objects/TR1/tr1_objects.h"

#include "Game/control/box.h"
#include "Game/collision/collide_item.h"
#include "Game/itemdata/creature_info.h"
#include "Game/missile.h"
#include "Specific/setup.h"
#include "Specific/level.h"

// Creatures
#include "Objects/TR1/Entity/tr1_ape.h" // OK
#include "Objects/TR1/Entity/tr1_bear.h" // OK
#include "Objects/TR1/Entity/tr1_doppelganger.h" // OK
#include "Objects/TR1/Entity/tr1_natla.h" // OK
#include "Objects/TR1/Entity/tr1_giant_mutant.h" // OK
#include "Objects/TR1/Entity/tr1_wolf.h" // OK
#include "Objects/TR1/Entity/tr1_big_rat.h" // OK
#include "Objects/TR1/Entity/tr1_centaur.h"
#include "Objects/TR1/Entity/tr1_winged_mutant.h"
#include "Objects/Utils/object_helper.h"

// Traps
#include "Objects/TR1/Trap/DamoclesSword.h"

using namespace TEN::Entities::Creatures::TR1;
using namespace TEN::Entities::Traps::TR1;

static void StartEntity(ObjectInfo* obj)
{
	obj = &Objects[ID_WOLF];
	if (obj->loaded)
	{
		obj->initialise = InitialiseWolf;
		obj->control = WolfControl;
		obj->collision = CreatureCollision;
		obj->shadowType = ShadowMode::All;
		obj->HitPoints = 6;
		obj->hitEffect = HIT_BLOOD;
		obj->pivotLength = 375;
		obj->radius = 340;
		obj->intelligent = true;
		obj->SetBoneRotation(2, ROT_Y); // head
	}

	obj = &Objects[ID_BEAR];
	if (obj->loaded)
	{
		obj->initialise = InitialiseCreature;
		obj->control = BearControl;
		obj->collision = CreatureCollision;
		obj->shadowType = ShadowMode::All;
		obj->HitPoints = 20;
		obj->hitEffect = HIT_BLOOD;
		obj->pivotLength = 500;
		obj->radius = 340;
		obj->intelligent = true;
		obj->SetBoneRotation(13, ROT_Y); // head
	}

	obj = &Objects[ID_APE];
	if (obj->loaded)
	{
		obj->control = ApeControl;
		obj->collision = CreatureCollision;
		obj->HitPoints = 22;
		obj->hitEffect = HIT_BLOOD;
		obj->shadowType = ShadowMode::All;
		obj->pivotLength = 250;
		obj->radius = 340;
		obj->intelligent = true;
		obj->ZoneType = ZoneType::Ape;
	}

	obj = &Objects[ID_BIG_RAT];
	if (obj->loaded)
	{
		obj->initialise = InitialiseBigRat;
		obj->control = BigRatControl;
		obj->collision = CreatureCollision;
		obj->shadowType = ShadowMode::All;
		obj->HitPoints = 5;
		obj->hitEffect = HIT_BLOOD;
		obj->pivotLength = 200;
		obj->radius = 204;
		obj->intelligent = true;
		obj->waterCreature = true;
		obj->ZoneType = ZoneType::Water;
		obj->SetBoneRotation(1, ROT_Y); // head
	}

	obj = &Objects[ID_NATLA];
	if (obj->loaded)
	{
		obj->initialise = InitialiseCreature;
		obj->collision = CreatureCollision;
		obj->control = NatlaControl;
		obj->shadowType = ShadowMode::All;
		obj->HitPoints = 400;
		obj->radius = 204;
		obj->hitEffect = HIT_BLOOD;
		obj->intelligent = true;
		obj->SetBoneRotation(2, ROT_X | ROT_Z);
	}

	obj = &Objects[ID_GIANT_MUTANT];
	if (obj->loaded)
	{
		obj->initialise = InitialiseCreature;
		obj->collision = CreatureCollision;
		obj->control = GiantMutantControl;
		obj->shadowType = ShadowMode::All;
		obj->HitPoints = 500;
		obj->hitEffect = HIT_BLOOD;
		obj->radius = 341;
		obj->intelligent = true;
		obj->SetBoneRotation(1, ROT_Y);
	}

	obj = &Objects[ID_DOPPELGANGER];
	if (obj->loaded)
	{
		if (Objects[ID_LARA].loaded)
			obj->animIndex = Objects[ID_LARA].animIndex;

		obj->initialise = InitialiseDoppelganger;
		obj->collision = CreatureCollision;
		obj->control = DoppelgangerControl;
		//obj->drawRoutine = DrawEvilLara;
		obj->shadowType = ShadowMode::All;
		obj->HitPoints = 1000;
		obj->hitEffect = HIT_BLOOD;
		obj->radius = 102;
		//obj->intelligent = true;
	}

	obj = &Objects[ID_CENTAUR_MUTANT];
	if (obj->loaded)
	{
		obj->initialise = InitialiseCreature;
		obj->control = CentaurControl;
		obj->collision = CreatureCollision;
		obj->shadowType = ShadowMode::All;
		obj->HitPoints = 120;
		obj->hitEffect = HIT_BLOOD;
		obj->pivotLength = 400;
		obj->radius = BLOCK(1.0f / 3);
		obj->intelligent = true;
		obj->ZoneType = ZoneType::Blockable;
		obj->SetBoneRotation(10, ROT_X | ROT_Y);
	}

	obj = &Objects[ID_WINGED_MUMMY];
	if (obj->loaded)
	{
		obj->initialise = InitialiseWingedMutant;
		obj->control = WingedMutantControl;
		obj->collision = CreatureCollision;
		obj->shadowType = ShadowMode::All;
		obj->hitEffect = HIT_BLOOD;
		obj->pivotLength = 150;
		obj->radius = BLOCK(1.0f / 3);
		obj->HitPoints = 50;
		obj->intelligent = true;
		obj->ZoneType = ZoneType::Flyer;
		obj->SetBoneRotation(1, ROT_Y); // torso
		obj->SetBoneRotation(2, ROT_Y); // head
	}
}

static void StartObject(ObjectInfo* obj)
{
	obj = &Objects[ID_BACON_REFERENCE];
	if (obj->loaded)
	{
		obj->drawRoutine = nullptr;
		obj->collision = AIPickupCollision;
		obj->HitPoints = 0;
	}
}

static void StartTrap(ObjectInfo* obj)
{
	obj = &Objects[ID_DAMOCLES_SWORD];
	if (obj->loaded)
		SetupDamoclesSword(*obj);
}

static void StartProjectiles(ObjectInfo* obj)
{
	InitProjectile(obj, ControlMissile, ID_PROJ_SHARD);
	InitProjectile(obj, ControlMissile, ID_PROJ_NATLA);
	InitProjectile(obj, ControlMissile, ID_PROJ_BOMB);
}

static ObjectInfo* objToInit;
void InitialiseTR1Objects()
{
	StartEntity(objToInit);
	StartObject(objToInit);
	StartTrap(objToInit);
	StartProjectiles(objToInit);
}
