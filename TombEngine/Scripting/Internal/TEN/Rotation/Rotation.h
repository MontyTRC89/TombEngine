#pragma once
#include "Math/Containers/EulerAngles.h"

namespace sol
{
	class state;
}

class Pose;

class Rotation
{
public:
	float x { 0 };
	float y { 0 };
	float z { 0 };

	Rotation() = default;
	Rotation(float aX, float aY, float aZ);
	Rotation(const EulerAngles& eulers);
	Rotation(const Pose& pose);

	void StoreInPHDPos(Pose& pose) const;

	std::string ToString() const;

	static void Register(sol::table& parent);
};
