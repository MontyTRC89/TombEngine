#include "framework.h"
#include "tr3_objects.h"
/// entities
#include "tr3_tony.h"
/// objects

/// traps

/// switch

/// vehicles
#include "cannon.h"
#include "kayak.h"
#include "minecart.h"
#include "quad.h"
#include "upv.h"
#include "rubberboat.h"
/// necessary import
#include "collide.h"
#include "setup.h"
#include "level.h"

static void InitialiseBaddy(ObjectInfo* obj)
{
	obj = &Objects[ID_TONY_BOSS];
	if (obj->loaded)
	{
		obj->initialise = InitialiseTony;
		obj->collision = CreatureCollision;
		obj->control = TonyControl;
		obj->drawRoutine = S_DrawTonyBoss;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->hitPoints = 100;
		obj->pivotLength = 50;
		obj->radius = 102;
		obj->intelligent = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->saveHitpoints = true;
		obj->savePosition = true;
		Bones[obj->boneIndex + 6 * 4] |= ROT_Y;
		Bones[obj->boneIndex + 6 * 4] |= ROT_X;
		Bones[obj->boneIndex + 13 * 4] |= ROT_Y;
	}

	obj = &Objects[ID_TONY_BOSS_FLAME];
	obj->control = ControlTonyFireBall;
	obj->drawRoutine = NULL;
}

static void InitialiseObject(ObjectInfo* obj)
{
	
}

static void InitialiseTrap(ObjectInfo* obj)
{
	
}

static void InitialiseVehicles(ObjectInfo* obj)
{
	obj = &Objects[ID_QUAD];
	if (obj->loaded)
	{
		obj->initialise = InitialiseQuadBike;
		obj->collision = QuadBikeCollision;
		obj->savePosition = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
	}

	obj = &Objects[ID_KAYAK];
	if (obj->loaded)
	{
		obj->initialise = InitialiseKayak;
		obj->collision = KayakCollision;
		//obj->drawRoutine = DrawKayak;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->savePosition = true;
	}

	obj = &Objects[ID_MINECART];
	if (obj->loaded)
	{
		obj->initialise = InitialiseMineCart;
		obj->collision = MineCartCollision;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->savePosition = true;
	}
}

static ObjectInfo* objToInit;
void InitialiseTR3Objects()
{
	InitialiseBaddy(objToInit);
	InitialiseObject(objToInit);
	InitialiseTrap(objToInit);
	InitialiseVehicles(objToInit);
}
