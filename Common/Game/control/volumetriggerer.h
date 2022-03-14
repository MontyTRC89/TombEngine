#pragma once
#include <variant>

class MESH_INFO;
class CAMERA_INFO;

namespace TEN::Control::Volumes
{
	using VolumeTriggerer = std::variant<
		short,
		MESH_INFO*,
		CAMERA_INFO *>;
};
