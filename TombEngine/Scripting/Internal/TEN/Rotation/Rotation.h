#pragma once
#include "Math/Objects/EulerAngles.h"

class EulerAngles;
class Pose;
namespace sol { class state; }

class Rotation
{
public:
	static void Register(sol::table& parent);

	// Members
	float x = 0;
	float y = 0;
	float z = 0;

	// Constructors
	Rotation() = default;
	Rotation(float x, float y, float z);
	Rotation(const EulerAngles& eulers);
	Rotation(const Pose& pose);
	Rotation(const Vector3& vector);

	void StoreInPHDPos(Pose& pose) const;

	// Converters
	std::string ToString() const;
	EulerAngles ToEulerAngles() const;

	// Operators
	operator Vector3() const;
};
