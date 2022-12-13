#pragma once

#include "Math/Containers/EulerAngles.h"

namespace sol {
	class state;
}
class Pose;

class Rotation {
public:
	float x { 0 };
	float y { 0 };
	float z { 0 };

	Rotation() = default;
	Rotation(float aX, float aY, float aZ);
	Rotation(EulerAngles const& ang);
	Rotation(Pose const& pos);

	std::string ToString() const;

	void StoreInPHDPos(Pose& pos) const;

	static void Register(sol::table & parent);
};
