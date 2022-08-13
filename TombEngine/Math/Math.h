#pragma once
#include "Math/Constants.h"
#include "Math/Containers/GameVector.h"
#include "Math/Containers/PoseData.h"
#include "Math/Containers/Vector2i.h"
#include "Math/Containers/Vector3i.h"
#include "Math/Containers/Vector3s.h"
#include "Math/Geometry.h"
#include "Math/Random.h"

//namespace TEN::Math
//{
	const float Lerp(float value0, float value1, float time);
	const float Smoothstep(float edge0, float edge1, float x);
	const float Smoothstep(float x);
	const float Luma(Vector3& color);
	const Vector3 Screen(Vector3& ambient, Vector3& tint);
	const Vector4 Screen(Vector4& ambient, Vector4& tint);
//}
