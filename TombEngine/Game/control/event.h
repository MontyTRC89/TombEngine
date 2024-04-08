#pragma once
#include <variant>

struct CAMERA_INFO;
struct MESH_INFO;

namespace TEN::Control::Volumes
{
	constexpr auto NO_CALL_COUNTER = -1;

	using Activator = std::variant<
		std::nullptr_t,
		short,
		MESH_INFO*,
		CAMERA_INFO*>;

	enum class ActivatorFlags
	{
		None = (0 << 0),
		Player = (1 << 0),
		NPC = (1 << 1),
		Moveable = (1 << 2),
		Static = (1 << 3),
		Flyby = (1 << 4),
		PhysicalObject = (1 << 5) // TODO: Future-proofing for Bullet.
	};

	enum class EventMode
	{
		LevelScript,
		Nodes
	};

	enum class EventType
	{
		Enter,
		Inside,
		Leave,
		Loop,
		Load,
		Save,
		Start,
		End,

		Count
	};

	struct Event
	{
		EventMode Mode = EventMode::LevelScript;
		std::string		Function = {};
		std::string		Data = {};

		int CallCounter = NO_CALL_COUNTER;
	};

	struct EventSet
	{
		std::string	Name = {};
		ActivatorFlags Activators = ActivatorFlags::None;
		std::array<Event, int(EventType::Count)> Events = {};
	};
};
