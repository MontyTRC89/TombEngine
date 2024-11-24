#include "framework.h"
#include "Scripting/Internal/TEN/Rotation/Rotation.h"

#include "Math/Math.h"
#include "Scripting/Internal/ReservedScriptNames.h"

using namespace TEN::Math;

/// Represents a degree-based 3D rotation.
// All values are clamped to the range [0.0, 360.0].
// @tenprimitive Rotation
// @pragma nostrip

void Rotation::Register(sol::table& parent)
{
	using ctors = sol::constructors<Rotation(float, float, float)>;
	parent.new_usertype<Rotation>(ScriptReserved_Rotation,
		ctors(),
		sol::call_constructor, ctors(),
		sol::meta_function::to_string, &Rotation::ToString,

		/// (float) X angle component.
		// @mem x
		"x", &Rotation::x,

		/// (float) Y angle component.
		// @mem y
		"y", &Rotation::y,

		/// (float) Z angle component.
		// @mem z
		"z", &Rotation::z);
}

/// @tparam float x X angle component.
// @tparam float y Y angle component.
// @tparam float z Z angle component.
// @treturn Rotation A Rotation.
// @function Rotation
Rotation::Rotation(float x, float y, float z)
{
	this->x = x;
	this->y = y;
	this->z = z;
}

Rotation::Rotation(const EulerAngles& eulers)
{
	x = TO_DEGREES(eulers.x);
	y = TO_DEGREES(eulers.y);
	z = TO_DEGREES(eulers.z);
}

Rotation::Rotation(const Vector3& vec)
{
	x = vec.x;
	y = vec.y;
	z = vec.z;
}

Rotation::Rotation(const Pose& pose)
{
	x = TO_DEGREES(pose.Orientation.x);
	y = TO_DEGREES(pose.Orientation.y);
	z = TO_DEGREES(pose.Orientation.z);
}

void Rotation::StoreInPHDPos(Pose& pose) const
{
	pose.Orientation.x = ANGLE(x);
	pose.Orientation.y = ANGLE(y);
	pose.Orientation.z = ANGLE(z);
}

Rotation::operator Vector3() const
{
	return Vector3(x, y, z);
};

/// Converts rotation to a directional vector or normal.
/// @tparam[opt] Vec3 position if specified, directional vector will be placed in relation to a given position.
/// @tparam[opt] float distance if specified, vector will be scaled to this distance, otherwise its X, Y and Z values will be normalized on a range from -1 to 1.
// @treturn Vec3 resulting directional vector calculated from this rotation.
// @function ToDirection
Vec3 Rotation::ToDirection(sol::optional<Vec3> position, sol::optional<float> distance) const
{
	// Convert degrees to radians.
	float pitch = x * RADIAN;
	float yaw   = y * RADIAN;

	// Calculate the direction vector.
	float dirX = std::cos(yaw) * std::cos(pitch);
	float dirY = std::sin(pitch);
	float dirZ = std::sin(yaw) * std::cos(pitch);

	Vec3  pos  = position.value_or(Vec3(0));
	float dist = distance.value_or(0);

	// Scale by the given distance.
	return pos + Vec3(dirX * dist, dirY * dist, dirZ * dist);
}

/// @tparam Rotation rotation this Rotation.
// @treturn string A string showing the X, Y, and Z angle components of the Rotation.
// @function __tostring
std::string Rotation::ToString() const
{
	return ("{ " + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + " }");
}

EulerAngles Rotation::ToEulerAngles() const
{
	return EulerAngles(ANGLE(x), ANGLE(y), ANGLE(z));
}
