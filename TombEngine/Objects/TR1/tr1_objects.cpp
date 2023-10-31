#include "framework.h"
#include "Objects/TR1/tr1_objects.h"

#include "Game/control/box.h"
#include "Game/collision/collide_item.h"
#include "Game/itemdata/creature_info.h"
#include "Game/Lara/lara.h"
#include "Game/missile.h"
#include "Game/Setup.h"
#include "Specific/level.h"

// Creatures
#include "Objects/TR1/Entity/Cowboy.h" // OK
#include "Objects/TR1/Entity/Kold.h" // OK
#include "Objects/TR1/Entity/tr1_ape.h" // OK
#include "Objects/TR1/Entity/tr1_bear.h" // OK
#include "Objects/TR1/Entity/tr1_doppelganger.h" // OK
#include "Objects/TR1/Entity/tr1_natla.h" // OK
#include "Objects/TR1/Entity/tr1_giant_mutant.h" // OK
#include "Objects/TR1/Entity/tr1_wolf.h" // OK
#include "Objects/TR1/Entity/tr1_big_rat.h" // OK
#include "Objects/TR1/Entity/tr1_centaur.h" // OK
#include "Objects/TR1/Entity/tr1_winged_mutant.h" // OK
#include "Objects/TR1/Entity/SkateboardKid.h" // OK
#include "Objects/Utils/object_helper.h"

// Traps
#include "Objects/TR1/Trap/DamoclesSword.h"
#include "Objects/TR1/Trap/SlammingDoors.h"
#include "Objects/TR1/Trap/SwingingBlade.h"

using namespace TEN::Entities::Creatures::TR1;
using namespace TEN::Entities::Traps::TR1;

static void StartEntity(ObjectInfo* obj)
{
	obj = &Objects[ID_WOLF];
	if (obj->loaded)
	{
		obj->Initialize = InitializeWolf;
		obj->control = WolfControl;
		obj->collision = CreatureCollision;
		obj->shadowType = ShadowMode::All;
		obj->HitPoints = 6;
		obj->pivotLength = 375;
		obj->radius = 340;
		obj->intelligent = true;
		obj->SetBoneRotationFlags(2, ROT_Y);
		obj->SetHitEffect();
	}

	obj = &Objects[ID_BEAR];
	if (obj->loaded)
	{
		obj->Initialize = InitializeCreature;
		obj->control = BearControl;
		obj->collision = CreatureCollision;
		obj->shadowType = ShadowMode::All;
		obj->HitPoints = 20;
		obj->pivotLength = 500;
		obj->radius = 340;
		obj->intelligent = true;
		obj->SetBoneRotationFlags(13, ROT_Y);
		obj->SetHitEffect();
	}

	obj = &Objects[ID_APE];
	if (obj->loaded)
	{
		obj->Initialize = InitializeCreature;
		obj->control = ApeControl;
		obj->collision = CreatureCollision;
		obj->shadowType = ShadowMode::All;
		obj->HitPoints = 22;
		obj->pivotLength = 250;
		obj->radius = 340;
		obj->intelligent = true;
		obj->LotType = LotType::Ape;
		obj->SetHitEffect();
	}

	obj = &Objects[ID_BIG_RAT];
	if (obj->loaded)
	{
		obj->Initialize = InitializeBigRat;
		obj->control = BigRatControl;
		obj->collision = CreatureCollision;
		obj->shadowType = ShadowMode::All;
		obj->HitPoints = 5;
		obj->pivotLength = 200;
		obj->radius = 204;
		obj->intelligent = true;
		obj->waterCreature = true;
		obj->LotType = LotType::Water;
		obj->SetBoneRotationFlags(1, ROT_Y);
		obj->SetHitEffect();
	}

	obj = &Objects[ID_NATLA];
	if (obj->loaded)
	{
		obj->Initialize = InitializeCreature;
		obj->collision = CreatureCollision;
		obj->control = NatlaControl;
		obj->shadowType = ShadowMode::All;
		obj->HitPoints = 400;
		obj->radius = 204;
		obj->intelligent = true;
		obj->LotType = LotType::Flyer;
		obj->SetBoneRotationFlags(0, ROT_X | ROT_Y); // Torso
		obj->SetBoneRotationFlags(1, ROT_X | ROT_Y); // Head
		obj->SetHitEffect();
	}

	obj = &Objects[ID_GIANT_MUTANT];
	if (obj->loaded)
	{
		obj->Initialize = InitializeCreature;
		obj->collision = CreatureCollision;
		obj->control = GiantMutantControl;
		obj->shadowType = ShadowMode::All;
		obj->HitPoints = 500;
		obj->radius = BLOCK(1 / 3.0f);
		obj->intelligent = true;
		obj->LotType = LotType::Blockable;
		obj->SetBoneRotationFlags(1, ROT_Y);
		obj->SetHitEffect();
	}

	obj = &Objects[ID_DOPPELGANGER];
	if (obj->loaded)
	{
		// NOTE: lara is obviously loaded by default.
		auto& laraObj = Objects[ID_LARA];
		obj->animIndex = laraObj.animIndex;
		obj->frameBase = laraObj.frameBase;
		obj->Initialize = InitializeCreature;
		obj->collision = CreatureCollision;
		obj->control = DoppelgangerControl;
		obj->shadowType = ShadowMode::All;
		obj->HitPoints = LARA_HEALTH_MAX;
		obj->radius = 102;
		obj->intelligent = true;
		obj->nonLot = true;
		obj->SetHitEffect();
	}

	obj = &Objects[ID_CENTAUR_MUTANT];
	if (obj->loaded)
	{
		obj->Initialize = InitializeCreature;
		obj->control = CentaurControl;
		obj->collision = CreatureCollision;
		obj->shadowType = ShadowMode::All;
		obj->HitPoints = 120;
		obj->pivotLength = 400;
		obj->radius = BLOCK(1 / 3.0f);
		obj->intelligent = true;
		obj->LotType = LotType::Blockable;
		obj->SetBoneRotationFlags(10, ROT_X | ROT_Y);
		obj->SetHitEffect();
	}

	obj = &Objects[ID_WINGED_MUMMY];
	if (obj->loaded)
	{
		obj->Initialize = InitializeWingedMutant;
		obj->control = WingedMutantControl;
		obj->collision = CreatureCollision;
		obj->shadowType = ShadowMode::All;
		obj->pivotLength = 150;
		obj->radius = BLOCK(1 / 3.0f);
		obj->HitPoints = 50;
		obj->intelligent = true;
		obj->LotType = LotType::Flyer;
		obj->SetBoneRotationFlags(1, ROT_Y);
		obj->SetBoneRotationFlags(2, ROT_Y);
		obj->SetHitEffect();
	}

	obj = &Objects[ID_COWBOY];
	if (obj->loaded)
	{
		obj->Initialize = InitializeCowboy;
		obj->control = CowboyControl;
		obj->collision = CreatureCollision;
		obj->shadowType = ShadowMode::All;
		obj->pivotLength = 0;
		obj->radius = 102;
		obj->HitPoints = 150;
		obj->intelligent = true;
		obj->SetBoneRotationFlags(0, ROT_X | ROT_Y);
		obj->SetBoneRotationFlags(1, ROT_X | ROT_Y);
		obj->SetHitEffect();
	}

	obj = &Objects[ID_KOLD];
	if (obj->loaded)
	{
		obj->Initialize = InitializeKold;
		obj->control = ControlKold;
		obj->collision = CreatureCollision;
		obj->shadowType = ShadowMode::All;
		obj->pivotLength = 0;
		obj->radius = 102;
		obj->HitPoints = 200;
		obj->intelligent = true;
		obj->SetBoneRotationFlags(1, ROT_Y);
		obj->SetBoneRotationFlags(0, ROT_X | ROT_Y);
		obj->SetHitEffect();
	}

	obj = &Objects[ID_SKATEBOARD_KID];
	if (obj->loaded)
	{
		obj->Initialize = InitializeSkateboardKid;
		obj->control = ControlSkateboardKid;
		obj->collision = CreatureCollision;
		obj->shadowType = ShadowMode::All;
		obj->pivotLength = 0;
		obj->radius = 256;
		obj->HitPoints = 125;
		obj->intelligent = true;
		obj->SetBoneRotationFlags(7, ROT_Y); // Head.
		obj->SetBoneRotationFlags(0, ROT_Y | ROT_X); // Torso.
		obj->SetHitEffect();
	}
}

static void StartObject(ObjectInfo* obj)
{
	obj = &Objects[ID_DOPPELGANGER_ORIGIN];
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
		obj->Initialize = InitializeDamoclesSword;
		obj->control = ControlDamoclesSword;
		obj->collision = CollideDamoclesSword;
		obj->shadowType = ShadowMode::All;
		obj->SetHitEffect(true);
	}

	obj = &Objects[ID_SLAMMING_DOORS];
	if (obj->loaded)
	{
		obj->Initialize = InitializeSlammingDoors;
		obj->control = ControlSlammingDoors;
		obj->collision = GenericSphereBoxCollision;
		obj->shadowType = ShadowMode::All;
		obj->SetHitEffect(true);
	}

	obj = &Objects[ID_SWINGING_BLADE];
	if (obj->loaded)
	{
		obj->Initialize = InitializeSwingingBlade;
		obj->control = ControlSwingingBlade;
		obj->collision = GenericSphereBoxCollision;
		obj->shadowType = ShadowMode::All;
		obj->SetHitEffect(true);
	}
}

static void StartProjectiles(ObjectInfo* obj)
{
	InitProjectile(obj, ControlMissile, ID_PROJ_SHARD);
	InitProjectile(obj, ControlMissile, ID_PROJ_BOMB);
	InitProjectile(obj, ControlMissile, ID_PROJ_BOMB);
}

void InitializeTR1Objects()
{
	ObjectInfo* objectPtr = nullptr;
	StartEntity(objectPtr);
	StartObject(objectPtr);
	StartTrap(objectPtr);
	StartProjectiles(objectPtr);
}
