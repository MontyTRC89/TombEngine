#include "framework.h"
#include "tr2_objects.h"
/// entities

/// objects

/// trap

/// vehicles
#include "boat.h"
#include "snowmobile.h"

/// necessary import
#include "collide.h"
#include "setup.h"
#include "level.h"

static void InitialiseBaddy()
{
	ObjectInfo* obj;

}

static void InitialiseObject()
{
	ObjectInfo* obj;

}

static void InitialiseTrap()
{
	ObjectInfo* obj;

}

// boat, snowmobile, snowmobile gun
static void InitialiseVehicles()
{
	ObjectInfo* obj;

	// TODO: fix BoatControl() not using int BoatControl(void)
	obj = &Objects[ID_SPEEDBOAT];
	if (obj->loaded)
	{
		obj->initialise = InitialiseBoat;
		obj->collision = BoatCollision;
		obj->control = BoatControl;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->savePosition = true;
	}

	obj = &Objects[ID_SNOWMOBILE];
	if (obj->loaded)
	{
		obj->initialise = InitialiseSkidoo;
		obj->collision = SkidooCollision;
		//obj->drawRoutine = DrawSkidoo; // TODO: create a new render for the skidoo. (with track animated)
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->savePosition = true;
	}
}

void InitialiseTR2Objects()
{
	InitialiseBaddy();
	InitialiseObject();
	InitialiseTrap();
	InitialiseVehicles();
}
