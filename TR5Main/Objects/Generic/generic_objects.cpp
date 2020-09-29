#include "framework.h"
#include "generic_objects.h"

/// objects
#include "generic_bridge.h"

/// necessary import
#include "setup.h"

static void StartObject()
{
	for (int objNumber = ID_BRIDGE1; objNumber <= ID_BRIDGE8; ++objNumber)
	{
		auto obj = &Objects[objNumber];
		if (obj->loaded)
		{
			obj->initialise = InitialiseBridge;
			obj->floor = BridgeFloor;
			obj->ceiling = BridgeCeiling;
		}
	}
}

void InitialiseGenericObjects()
{
	StartObject();
}
