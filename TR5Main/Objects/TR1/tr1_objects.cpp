#include "framework.h"
#include "Objects/TR1/tr1_objects.h"
/// entities
#include "tr1_ape.h" // OK
#include "tr1_bear.h" // OK
#include "tr1_doppelganger.h" // OK
#include "tr1_natla.h" // OK
#include "tr1_giant_mutant.h" // OK
#include "tr1_wolf.h" // OK
#include "tr1_bigrat.h" // OK
#include "tr1_centaur.h"
/// objects

/// traps

/// necessary import
#include "Game/control/box.h"
#include "Game/collision/collide_item.h"
#include "Specific/setup.h"
#include "Specific/level.h"
#include "Objects/Utils/object_helper.h"
#include "Game/itemdata/creature_info.h"

static void StartBaddy(OBJECT_INFO* obj)
{
	obj = &Objects[ID_WOLF];
	if (obj->loaded)
	{
		obj->initialise = InitialiseWolf;
		obj->control = WolfControl;
		obj->collision = CreatureCollision;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->hitPoints = 6;
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
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->hitPoints = 20;
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
		obj->hitPoints = 22;
		obj->hitEffect = HIT_BLOOD;
		obj->shadowSize = 128;
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
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->hitPoints = 5;
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
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->hitPoints = 400;
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
		obj->control = AbortionControl;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->hitPoints = 500;
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
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->hitPoints = 1000;
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
		obj->shadowSize = UNIT_SHADOW / 3;
		obj->hitPoints = 120;
		obj->hitEffect = HIT_BLOOD;
		obj->pivotLength = 400;
		obj->radius = WALL_SIZE / 3;
		obj->intelligent = 1;
		obj->savePosition = obj->saveHitpoints = obj->saveAnim = obj->saveFlags = 1;
		g_Level.Bones[obj->boneIndex + 10 * 4] |= ROT_Y | ROT_X;
	}
}

static void StartObject(OBJECT_INFO* obj)
{
	obj = &Objects[ID_BACON_REFERENCE];
	if (obj->loaded)
	{
		obj->drawRoutine = nullptr;
		obj->collision = AIPickupCollision;
		obj->hitPoints = 0;
	}
}

static void StartTrap(OBJECT_INFO* obj)
{

}

static void StartProjectiles(OBJECT_INFO* obj)
{
	InitProjectile(obj, ControlCentaurBomb, ID_PROJ_BOMB);
}

static OBJECT_INFO* objToInit;
void InitialiseTR1Objects()
{
	StartBaddy(objToInit);
	StartObject(objToInit);
	StartTrap(objToInit);
	StartProjectiles(objToInit);
}
