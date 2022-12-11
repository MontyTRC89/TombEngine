#pragma once
#include "Game/control/volumetriggerer.h"
#include "Game/room.h"
#include "Specific/setup.h"
#include "Renderer/Renderer11.h"

constexpr auto NO_EVENT_SET = -1;
constexpr auto NO_CALL_COUNTER = -1;

constexpr auto VOLUME_BUSY_TIMEOUT = 10;
constexpr auto VOLUME_LEAVE_TIMEOUT = 5;

constexpr auto VOLUME_STATE_QUEUE_SIZE = 16;

enum class VolumeStateStatus
{
	Outside,
	Entering,
	Inside,
	Leaving
};

enum class VolumeType
{
	Box,
	Sphere,
	Prism	// TODO: Unsupported as of now.
};

enum VolumeActivatorType
{
	Player = 1,
	NPC = 2,
	Moveable = 4,
	Static = 8,
	Flyby = 16,
	PhysicalObject = 32	// Future-proofing for Bullet.
};

struct VolumeState
{
	VolumeStateStatus Status = VolumeStateStatus::Outside;
	VolumeTriggerer Triggerer = nullptr;
	int Timestamp = 0;
};

struct TriggerVolume
{
	VolumeType Type;
	std::string Name = {};
	int EventSetIndex;

	bool Enabled = true;

	Vector3 Position;
	Quaternion Rotation;
	Vector3 Scale;	// x used as radius if type is VolumeType::Sphere.

	BoundingOrientedBox Box;
	BoundingSphere Sphere;

	std::vector<VolumeState> StateQueue;
};

namespace TEN::Control::Volumes
{
	void TestVolumes(short roomNumber, BoundingOrientedBox bbox, VolumeActivatorType activatorType, VolumeTriggerer triggerer);
	void TestVolumes(short itemNum);
	void TestVolumes(short roomNumber, MESH_INFO* mesh);
	void TestVolumes(CAMERA_INFO* camera);

	void InitialiseNodeScripts();
}
