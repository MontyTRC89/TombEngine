#pragma once
#include <variant>

struct MESH_INFO;
struct CAMERA_INFO;

namespace TEN::Control::Volumes
{
	using VolumeTriggerer = std::variant<
		std::nullptr_t,
		short,
		MESH_INFO*,
		CAMERA_INFO*>;

	enum class VolumeEventMode
	{
		LevelScript,
		Constructor
	};

	struct VolumeEvent
	{
		VolumeEventMode Mode;
		std::string Function;
		std::string Argument;

		int CallCounter;
	};

	struct VolumeEventSet
	{
		std::string Name;
		int Activators;

		VolumeEvent OnEnter;
		VolumeEvent OnLeave;
		VolumeEvent OnInside;
	};
};
