#pragma once
#include "Math/Constants.h"
#include "Math/Geometry.h"
#include "Math/Legacy.h"
#include "Math/Objects/EulerAngles.h"
#include "Math/Objects/GameBoundingBox.h"
#include "Math/Objects/GameVector.h"
#include "Math/Objects/Pose.h"
#include "Math/Objects/Vector2i.h"
#include "Math/Objects/Vector3i.h"
#include "Math/Random.h"
#include "Math/Solvers.h"

namespace TEN::Math
{
	constexpr inline auto OFFSET_RADIUS = [](auto x) { return ((x * SQRT_2) + 4); };
	constexpr inline auto MESH_BITS		= [](auto x) { return (1 << x); };

	const float Lerp(float value0, float value1, float time);
	const float InterpolateCos(float value0, float value1, float time);
	const float InterpolateCubic(float value0, float value1, float value2, float value3, float time);

	const float Smoothstep(float x);
	const float Smoothstep(float edge0, float edge1, float x);
	const float Luma(const Vector3& color);
	const Vector3 Screen(const Vector3& ambient, const Vector3& tint);
	const Vector4 Screen(const Vector4& ambient, const Vector4& tint);
}
