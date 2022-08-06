#include "framework.h"
#include "Objects/TR1/tr1_objects.h"

/// necessary import
#include "Game/control/box.h"
#include "Game/collision/collide_item.h"
#include "Game/itemdata/creature_info.h"
#include "Specific/setup.h"
#include "Specific/level.h"

/// entities
#include "Objects/TR1/Entity/tr1_ape.h" // OK
#include "Objects/TR1/Entity/tr1_bear.h" // OK
#include "Objects/TR1/Entity/tr1_doppelganger.h" // OK
#include "Objects/TR1/Entity/tr1_natla.h" // OK
#include "Objects/TR1/Entity/tr1_giant_mutant.h" // OK
#include "Objects/TR1/Entity/tr1_wolf.h" // OK
#include "Objects/TR1/Entity/tr1_big_rat.h" // OK
#include "Objects/TR1/Entity/tr1_centaur.h"
#include "Objects/Utils/object_helper.h"

using namespace TEN::Entities::TR1;

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
		obj->savePosition = true;
		obj->saveHitpoints = true;
		obj->saveAnim = true;
		obj->saveFlags = true;

		g_Level.Bones[obj->boneIndex + 2 * 4] |= ROT_Y;
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
		obj->savePosition = true;
		obj->saveHitpoints = true;
		obj->saveAnim = true;
		obj->saveFlags = true;

		g_Level.Bones[obj->boneIndex + 13 * 4] |= ROT_Y;
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
		obj->savePosition = true;
		obj->saveHitpoints = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->zoneType = ZONE_APE;
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
		obj->savePosition = true;
		obj->saveHitpoints = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->waterCreature = true;
		obj->zoneType = ZONE_WATER;
		g_Level.Bones[obj->boneIndex + 4] |= ROT_Y;
	}

	obj = &Objects[ID_NATLA];
	if (obj->loaded)
	{
		obj->initialise = InitialiseCreature;
		obj->collision = CreatureCollision;
		obj->hitEffect = HIT_BLOOD;
		obj->control = NatlaControl;
		obj->shadowType = ShadowMode::All;
		obj->HitPoints = 400;
		obj->radius = 204;
		obj->intelligent = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->savePosition = true;
		obj->saveHitpoints = true;
		g_Level.Bones[obj->boneIndex + 2 * 4] |= (ROT_Z | ROT_X);
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
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->savePosition = true;
		obj->saveHitpoints = true;
		g_Level.Bones[obj->boneIndex + 1 * 4] |= ROT_Y;
	}

	obj = &Objects[ID_LARA_DOPPELGANGER];
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
		obj->saveFlags = true;
		obj->savePosition = true;
		obj->saveHitpoints = true;
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
		obj->radius = WALL_SIZE / 3;
		obj->intelligent = 1;
		obj->savePosition = obj->saveHitpoints = obj->saveAnim = obj->saveFlags = 1;
		g_Level.Bones[obj->boneIndex + 10 * 4] |= ROT_Y | ROT_X;
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

}

static void StartProjectiles(ObjectInfo* obj)
{
	InitProjectile(obj, ControlCentaurBomb, ID_PROJ_BOMB);
}

static ObjectInfo* objToInit;
void InitialiseTR1Objects()
{
	StartEntity(objToInit);
	StartObject(objToInit);
	StartTrap(objToInit);
	StartProjectiles(objToInit);
}
