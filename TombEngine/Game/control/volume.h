#pragma once
#include "Game/control/volumeactivator.h"
#include "Game/room.h"
#include "Renderer/Renderer11.h"
#include "Specific/setup.h"

struct CollisionSetup;

namespace TEN::Control::Volumes
{
	constexpr auto NO_EVENT_SET = -1;

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

		int Timestamp = 0;
	};

	void TestVolumes(short roomNumber, const BoundingOrientedBox& box, VolumeActivatorFlags activatorFlag, VolumeActivator activator);
	void TestVolumes(short itemNumber, const CollisionSetup* coll = nullptr);
	void TestVolumes(short roomNumber, MESH_INFO* mesh);
	void TestVolumes(CAMERA_INFO* camera);

	void InitialiseNodeScripts();
}

// TODO: Move into namespace and deal with errors.
struct TriggerVolume
{
	bool Enabled	   = true;
	int	 EventSetIndex = 0;

	std::string Name = {};
	VolumeType	Type = VolumeType::Box;

	BoundingOrientedBox Box	   = BoundingOrientedBox();
	BoundingSphere		Sphere = BoundingSphere();

	std::vector<VolumeState> StateQueue = {};
};