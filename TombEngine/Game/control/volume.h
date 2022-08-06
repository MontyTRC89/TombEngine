#pragma once
#include "Game/control/volumetriggerer.h"
#include "Game/room.h"
#include "Specific/setup.h"
#include "Renderer/Renderer11.h"

constexpr auto NO_EVENT_SET = -1;
constexpr auto NO_CALL_COUNTER = -1;

constexpr auto VOLUME_BUSY_TIMEOUT = 10;

enum class TriggerStatus
{
	Outside,
	Entering,
	Inside,
	Leaving,
};

enum class TriggerVolumeType
{
	Box,
	Sphere,
	Prism	// TODO: Unsupported as of now.
};

enum TriggerVolumeActivators
{
	Player = 1,
	NPC = 2,
	Movable = 4,
	Static = 8,
	Flyby = 16,
	PhysicalObject = 32	// Future-proofing for Bullet.
};

struct TriggerVolume
{
	TriggerVolumeType Type;
	int EventSetIndex;

	Vector3 Position;
	Quaternion Rotation;
	Vector3 Scale;	// x used as radius if type is TriggerVolumeType::Sphere.

	BoundingOrientedBox Box;
	BoundingSphere Sphere;

	TriggerStatus Status = TriggerStatus::Outside;
	VolumeTriggerer Triggerer = nullptr;
	int Timeout = 0;
};

namespace TEN::Control::Volumes
{
	extern int CurrentCollidedVolume;

	void TestVolumes(short roomNumber, BoundingOrientedBox bbox, TriggerVolumeActivators activatorType, VolumeTriggerer triggerer);
	void TestVolumes(short itemNum);
	void TestVolumes(short roomNumber, MESH_INFO* mesh);
	void TestVolumes(CAMERA_INFO* camera);
}
