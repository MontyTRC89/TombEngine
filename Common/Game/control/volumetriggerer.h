#pragma once
#include <variant>

struct MESH_INFO;
struct CAMERA_INFO;

namespace TEN::Control::Volumes
{
	using VolumeTriggerer = std::variant<
		short,
		MESH_INFO*,
		CAMERA_INFO *>;
};
