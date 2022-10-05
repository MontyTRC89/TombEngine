#pragma once
#include "Math/Constants.h"
#include "Math/Containers/BoundingBox.h"
#include "Math/Containers/EulerAngles.h"
#include "Math/Containers/GameVector.h"
#include "Math/Containers/PoseData.h"
#include "Math/Containers/Vector2i.h"
#include "Math/Containers/Vector3i.h"
#include "Math/Geometry.h"
#include "Math/Random.h"
#include "Math/Solvers.h"

#include "Math/Legacy.h"

namespace TEN::Math
{
	constexpr auto MESH_BITS = [](auto x) { return (1 << x); };

	const float Lerp(float value0, float value1, float time);
	const float InterpolateCos(float value0, float value1, float time);
	const float InterpolateCubic(float value0, float value1, float value2, float value3, float time);

	const float Smoothstep(float edge0, float edge1, float x);
	const float Smoothstep(float x);
	const float Luma(const Vector3& color);
	const Vector3 Screen(const Vector3& ambient, const Vector3& tint);
	const Vector4 Screen(const Vector4& ambient, const Vector4& tint);
}
