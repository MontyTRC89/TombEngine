#pragma once

class EulerAngles;

namespace sol
{
	class state;
}

class Rotation
{
public:
	float x{ 0 };
	float y{ 0 };
	float z{ 0 };

	Rotation() = default;
	Rotation(float x, float y, float z);
	Rotation(EulerAngles orient);

	std::string ToString() const;

	void StoreInPHDPos(EulerAngles& orient);

	static void Register(sol::table& parent);
};
