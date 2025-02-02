#include "framework.h"
#include "Scripting/Internal/TEN/Types/Rotation/Rotation.h"

#include "Math/Math.h"
#include "Scripting/Internal/ReservedScriptNames.h"

using namespace TEN::Math;

namespace TEN::Scripting
{
	/// Represents a degree-based 3D rotation.
	// All angle components are in degrees clamped to the range [0.0, 360.0].
	// @tenprimitive Rotation
	// @pragma nostrip

	void Rotation::Register(sol::table& parent)
	{
		using ctors = sol::constructors<
			Rotation(float, float, float)>;

		// Register type.
		parent.new_usertype<Rotation>(
			ScriptReserved_Rotation,
			ctors(),
			sol::call_constructor, ctors(),

			sol::meta_function::to_string, &Rotation::ToString,

			ScriptReserved_RotationDirection, &Rotation::Direction,

			/// (float) X angle component in degrees.
			// @mem x
			"x", &Rotation::x,

			/// (float) Y angle component in degrees.
			// @mem y
			"y", &Rotation::y,

			/// (float) Z angle component in degrees.
			// @mem z
			"z", &Rotation::z);
	}

	/// Create a Rotation object.
	// @function Rotation
	// @tparam float x X angle component in degrees.
	// @tparam float y Y angle component in degrees.
	// @tparam float z Z angle component in degrees.
	// @treturn Rotation A new Rotation object.
	Rotation::Rotation(float x, float y, float z)
	{
		this->x = x;
		this->y = y;
		this->z = z;
	}

	Rotation::Rotation(const Vector3& vec)
	{
		x = vec.x;
		y = vec.y;
		z = vec.z;
	}

	Rotation::Rotation(const EulerAngles& eulers)
	{
		x = TO_DEGREES(eulers.x);
		y = TO_DEGREES(eulers.y);
		z = TO_DEGREES(eulers.z);
	}

	/// Get the normalized direction vector of this Rotation.
	// @function Direction
	// @treturn Vec3 Normalized direction vector.
	Vec3 Rotation::Direction() const
	{
		auto eulers = ToEulerAngles();
		return Vec3(eulers.ToDirection());
	}

	/// @function __tostring
	// @tparam Rotation rot This Rotation.
	// @treturn string A string showing the X, Y, and Z angle components of this Rotation.
	std::string Rotation::ToString() const
	{
		return ("{ " + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + " }");
	}

	EulerAngles Rotation::ToEulerAngles() const
	{
		return EulerAngles(ANGLE(x), ANGLE(y), ANGLE(z));
	}

	Rotation::operator Vector3() const
	{
		return Vector3(x, y, z);
	};
}
