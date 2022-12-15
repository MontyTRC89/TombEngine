#pragma once
#include "Game/control/volumeactivator.h"
#include "Game/room.h"
#include "Renderer/Renderer11.h"
#include "Specific/setup.h"

struct CollisionSetup;

constexpr auto NO_EVENT_SET	   = -1;

constexpr auto VOLUME_BUSY_TIMEOUT	= 10;
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
	Prism // TODO: Unsupported as of now.
};

struct VolumeState
{
	VolumeStateStatus Status	= VolumeStateStatus::Outside;
	VolumeActivator	  Activator = nullptr;
	int				  Timestamp = 0;
};

struct TriggerVolume
{
	bool Enabled	   = true;
	int	 EventSetIndex = 0;

	VolumeType	Type = VolumeType::Box;
	std::string Name = {};

	BoundingOrientedBox Box	   = BoundingOrientedBox();
	BoundingSphere		Sphere = BoundingSphere();

	std::vector<VolumeState> StateQueue = {};
};

namespace TEN::Control::Volumes
{
	void TestVolumes(short roomNumber, const BoundingOrientedBox& box, VolumeActivatorFlags activatorFlag, VolumeActivator activator);
	void TestVolumes(short itemNumber, const CollisionSetup* coll = nullptr);
	void TestVolumes(short roomNumber, MESH_INFO* mesh);
	void TestVolumes(CAMERA_INFO* camera);

	void InitialiseNodeScripts();
}
