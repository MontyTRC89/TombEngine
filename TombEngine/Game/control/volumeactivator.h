#pragma once
#include <variant>

struct CAMERA_INFO;
struct MESH_INFO;

namespace TEN::Control::Volumes
{
	constexpr auto NO_CALL_COUNTER = -1;

	using VolumeActivator = std::variant<
		std::nullptr_t,
		short,
		MESH_INFO*,
		CAMERA_INFO*>;

	enum class VolumeEventMode
	{
		LevelScript,
		Nodes
	};

	enum class VolumeActivatorFlags
	{
		None		   = (0 << 0),
		Player		   = (1 << 0),
		NPC			   = (1 << 1),
		Moveable	   = (1 << 2),
		Static		   = (1 << 3),
		Flyby		   = (1 << 4),
		PhysicalObject = (1 << 5) // TODO: Future-proofing for Bullet.
	};

	struct VolumeEvent
	{
		VolumeEventMode Mode	 = VolumeEventMode::LevelScript;
		std::string		Function = {};
		std::string		Data	 = {};

		int CallCounter = NO_CALL_COUNTER;
	};

	struct VolumeEventSet
	{
		std::string Name = {};
		VolumeActivatorFlags Activators = VolumeActivatorFlags::None;

		VolumeEvent OnEnter	 = {};
		VolumeEvent OnLeave	 = {};
		VolumeEvent OnInside = {};
	};
};
