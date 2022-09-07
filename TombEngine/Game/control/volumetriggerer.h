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
		Nodes
	};

	struct VolumeEvent
	{
		VolumeEventMode Mode;
		std::string Function;
		std::string Data;

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
