#pragma once
#include "Game/room.h"
#include "Specific/setup.h"
#include "Renderer/Renderer11.h"

enum TriggerStatus
{
	TS_OUTSIDE = 0,
	TS_ENTERING = 1,
	TS_INSIDE = 2,
	TS_LEAVING = 3
};

enum TriggerVolumeType
{
	VOLUME_BOX = 0,
	VOLUME_SPHERE = 1,
	VOLUME_PRISM = 2 // Unsupported as of now
};

enum TriggerVolumeActivators
{
	PLAYER = 1,
	NPC = 2,
	MOVEABLES = 4,
	STATICS = 8,
	FLYBYS = 16,
	PHYSICALOBJECTS = 32 // Future-proofness for Bullet
};

struct TRIGGER_VOLUME
{
	TriggerVolumeType type;

	Vector3 position;
	Quaternion rotation;
	Vector3 scale; // X used as radius if type is VOLUME_SPHERE

	std::string onEnter;
	std::string onInside;
	std::string onLeave;

	int activators;
	bool oneShot;

	TriggerStatus status;
	BoundingOrientedBox box;
	BoundingSphere sphere;
};

namespace TEN::Control::Volumes
{
	extern int CurrentCollidedVolume;

	void TestVolumes(short roomNumber, BoundingOrientedBox bbox, TriggerVolumeActivators activatorType);
	void TestVolumes(ITEM_INFO* item);
	void TestVolumes(short roomNumber, MESH_INFO* mesh);
	void TestVolumes(CAMERA_INFO* camera);
}