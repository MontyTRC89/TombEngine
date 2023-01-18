#pragma once
#include "Math/Constants.h"
#include "Math/Containers/EulerAngles.h"
#include "Math/Containers/GameBoundingBox.h"
#include "Math/Containers/GameVector.h"
#include "Math/Containers/Pose.h"
#include "Math/Containers/Vector2i.h"
#include "Math/Containers/Vector3i.h"
#include "Math/Geometry.h"
#include "Math/Interpolation.h"
#include "Math/Random.h"
#include "Math/Solvers.h"

#include "Math/Legacy.h"

namespace TEN::Math
{
	constexpr inline auto OFFSET_RADIUS = [](auto x) { return ((x * SQRT_2) + 4); };
	constexpr inline auto MESH_BITS		= [](auto x) { return (1 << x); };

	const float	  Luma(const Vector3& color);
	const Vector3 Screen(const Vector3& ambient, const Vector3& tint);
	const Vector4 Screen(const Vector4& ambient, const Vector4& tint);
}
