#include "framework.h"
#include "generic_objects.h"

/// objects
#include "generic_bridge.h"

/// necessary import
#include "setup.h"

static void StartObject()
{
	OBJECT_INFO* obj;

	obj = &Objects[ID_BRIDGE];
	if (obj->loaded)
	{
		obj->initialise = InitialiseBridge;
		obj->floor = BridgeFloor;
		obj->ceiling = BridgeCeiling;
	}
}

void InitialiseGenericObjects()
{
	StartObject();
}
