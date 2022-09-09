#include "framework.h"
#include "Math/Math.h"

namespace TEN::Math
{
	const float Lerp(float value0, float value1, float time)
	{
		return (((1.0f - time) * value0) + (time * value1));
	}

	const float Smoothstep(float edge0, float edge1, float x)
	{
		x = std::clamp(x, edge0, edge1);

		// Don't process if input value is the same as one of edges.
		if (x == edge0)
			return edge0;
		else if (x == edge1)
			return edge1;

		// Scale, bias, and saturate x to [0, 1] range.
		x = std::clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);

		// Evaluate polynomial.
		return (x * x * x * (x * (x * 6 - 15) + 10));
	}

	const float Smoothstep(float x)
	{
		return Smoothstep(0.0f, 1.0f, x);
	}

	const float Luma(const Vector3& color)
	{
		// Use Rec.709 trichromat formula to get perceptive luma value.
		return float((color.x * 0.2126f) + (color.y * 0.7152f) + (color.z * 0.0722f));
	}

	const Vector4 Screen(const Vector4& ambient, const Vector4& tint)
	{
		auto result = Screen(Vector3(ambient.x, ambient.y, ambient.z), Vector3(tint.x, tint.y, tint.z));
		return Vector4(result.x, result.y, result.z, ambient.w * tint.w);
	}

	const Vector3 Screen(const Vector3& ambient, const Vector3& tint)
	{
		float luma = Luma(tint);

		auto multiplicative = ambient * tint;
		auto additive = ambient + tint;

		float R = Lerp(multiplicative.x, additive.x, luma);
		float G = Lerp(multiplicative.y, additive.y, luma);
		float B = Lerp(multiplicative.z, additive.z, luma);
		return Vector3(R, G, B);
	}
}
