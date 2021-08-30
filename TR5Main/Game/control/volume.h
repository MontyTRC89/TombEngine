#pragma once

#include "room.h"
#include "setup.h"
#include "Renderer11.h"

namespace ten::Control::Volumes
{
	extern int CurrentCollidedVolume;

	void TestVolumes(short roomNumber, BoundingOrientedBox bbox, TriggerVolumeActivators activatorType);
	void TestVolumes(ITEM_INFO* item);
	void TestVolumes(short roomNumber, MESH_INFO* mesh);
}