#pragma once
#include "Math/Objects/EulerAngles.h"

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
	Rotation(const Vector3& vec);

	operator Vector3() const;

	void StoreInPHDPos(Pose& pose) const;

	std::string ToString() const;

	static void Register(sol::table& parent);
};
