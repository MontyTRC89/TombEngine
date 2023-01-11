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
		obj->pivotLength = 375;
		obj->radius = 340;
		obj->intelligent = true;
		obj->SetBoneRotationFlags(2, ROT_Y);
		obj->SetupHitEffect();
	}

	obj = &Objects[ID_BEAR];
	if (obj->loaded)
	{
		obj->initialise = InitialiseCreature;
		obj->control = BearControl;
		obj->collision = CreatureCollision;
		obj->shadowType = ShadowMode::All;
		obj->HitPoints = 20;
		obj->pivotLength = 500;
		obj->radius = 340;
		obj->intelligent = true;
		obj->SetBoneRotationFlags(13, ROT_Y);
		obj->SetupHitEffect();
	}

	obj = &Objects[ID_APE];
	if (obj->loaded)
	{
		obj->initialise = InitialiseCreature;
		obj->control = ApeControl;
		obj->collision = CreatureCollision;
		obj->shadowType = ShadowMode::All;
		obj->HitPoints = 22;
		obj->pivotLength = 250;
		obj->radius = 340;
		obj->intelligent = true;
		obj->lotType = LOTType::Ape;
		obj->SetupHitEffect();
	}

	obj = &Objects[ID_BIG_RAT];
	if (obj->loaded)
	{
		obj->initialise = InitialiseBigRat;
		obj->control = BigRatControl;
		obj->collision = CreatureCollision;
		obj->shadowType = ShadowMode::All;
		obj->HitPoints = 5;
		obj->pivotLength = 200;
		obj->radius = 204;
		obj->intelligent = true;
		obj->waterCreature = true;
		obj->lotType = LOTType::Water;
		obj->SetBoneRotationFlags(1, ROT_Y);
		obj->SetupHitEffect();
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
		obj->intelligent = true;
		obj->SetBoneRotationFlags(2, ROT_X | ROT_Z);
		obj->SetupHitEffect();
	}

	obj = &Objects[ID_GIANT_MUTANT];
	if (obj->loaded)
	{
		obj->initialise = InitialiseCreature;
		obj->collision = CreatureCollision;
		obj->control = GiantMutantControl;
		obj->shadowType = ShadowMode::All;
		obj->HitPoints = 500;
		obj->radius = BLOCK(1 / 3.0f);
		obj->intelligent = true;
		obj->lotType = LOTType::Blockable;
		obj->SetBoneRotationFlags(1, ROT_Y);
		obj->SetupHitEffect();
	}

	obj = &Objects[ID_DOPPELGANGER];
	if (obj->loaded)
	{
		obj->animIndex = Objects[ID_LARA].animIndex; // NOTE: lara is obviously loaded by default.
		obj->initialise = InitialiseCreature;
		obj->collision = CreatureCollision;
		obj->control = DoppelgangerControl;
		obj->shadowType = ShadowMode::All;
		obj->HitPoints = 1000;
		obj->radius = 102;
		obj->SetupHitEffect(true);
	}

	obj = &Objects[ID_CENTAUR_MUTANT];
	if (obj->loaded)
	{
		obj->initialise = InitialiseCreature;
		obj->control = CentaurControl;
		obj->collision = CreatureCollision;
		obj->shadowType = ShadowMode::All;
		obj->HitPoints = 120;
		obj->pivotLength = 400;
		obj->radius = BLOCK(1 / 3.0f);
		obj->intelligent = true;
		obj->lotType = LOTType::Blockable;
		obj->SetBoneRotationFlags(10, ROT_X | ROT_Y);
		obj->SetupHitEffect();
	}

	obj = &Objects[ID_WINGED_MUMMY];
	if (obj->loaded)
	{
		obj->initialise = InitialiseWingedMutant;
		obj->control = WingedMutantControl;
		obj->collision = CreatureCollision;
		obj->shadowType = ShadowMode::All;
		obj->hitEffect = HitEffect::Blood;
		obj->pivotLength = 150;
		obj->radius = BLOCK(1 / 3.0f);
		obj->HitPoints = 50;
		obj->intelligent = true;
		obj->lotType = LOTType::Flyer;
		obj->SetBoneRotationFlags(1, ROT_Y);
		obj->SetBoneRotationFlags(2, ROT_Y);
		obj->SetupHitEffect();
	}
}

static void StartObject(ObjectInfo* obj)
{
	obj = &Objects[ID_BACON_REFERENCE];
	if (obj->loaded)
	{
		obj->collision = AIPickupCollision;
		obj->drawRoutine = nullptr;
	}
}

static void StartTrap(ObjectInfo* obj)
{
	obj = &Objects[ID_DAMOCLES_SWORD];
	if (obj->loaded)
	{
		obj->initialise = InitialiseDamoclesSword;
		obj->control = ControlDamoclesSword;
		obj->collision = CollideDamoclesSword;
		obj->shadowType = ShadowMode::All;
		obj->SetupHitEffect(true);
	}
}

static void StartProjectiles(ObjectInfo* obj)
{
	InitProjectile(obj, ControlMissile, ID_PROJ_SHARD);
	InitProjectile(obj, ControlMissile, ID_PROJ_NATLA);
	InitProjectile(obj, ControlMissile, ID_PROJ_BOMB);
}

void InitialiseTR1Objects()
{
	ObjectInfo* objectPtr = nullptr;
	StartEntity(objectPtr);
	StartObject(objectPtr);
	StartTrap(objectPtr);
	StartProjectiles(objectPtr);
}
