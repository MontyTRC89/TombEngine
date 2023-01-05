#pragma once
#include "Game/camera.h"

static const std::unordered_map<std::string, CameraType> kCameraType
{
	{"Chase", CameraType::Chase},
	{"Fixed", CameraType::Fixed},
	{"Look", CameraType::Look},
	{"Combat", CameraType::Combat},
	{"Heavy", CameraType::Heavy},
	{"Object", CameraType::Object},
};

namespace sol {
	class state;
};

namespace Misc {
	void Register(sol::state* lua, sol::table &);
};
