#pragma once
#include <variant>

struct CAMERA_INFO;
struct MESH_INFO;

namespace TEN::Control::Volumes
{
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

	struct VolumeEvent
	{
		VolumeEventMode Mode	 = VolumeEventMode::LevelScript;
		std::string		Function = {};
		std::string		Data	 = {};

		int CallCounter = 0;
	};

	struct VolumeEventSet
	{
		std::string Name	   = {};
		int			Activators = 0;

		VolumeEvent OnEnter	 = {};
		VolumeEvent OnLeave	 = {};
		VolumeEvent OnInside = {};
	};
};
