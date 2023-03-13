#include "framework.h"
#include "Math/Math.h"

namespace TEN::Math
{
	const float Lerp(float value0, float value1, float time)
	{
		return (((1.0f - time) * value0) + (time * value1));
	}

	const float InterpolateCos(float value0, float value1, float time)
	{
		return Lerp(value0, value1, (1 - cos(time * PI)) * 0.5f);
	}

	const float InterpolateCubic(float value0, float value1, float value2, float value3, float time)
	{
		float p = (value3 - value2) - (value0 - value1);
		float q = (value0 - value1) - p;
		float r = value2 - value0;
		float s = value1;
		float x = time;
		float xSquared = SQUARE(x);
		return ((p * xSquared * x) + (q * xSquared) + (r * x) + s);
	}

	const float Smoothstep(float x)
	{
		return Smoothstep(0.0f, 1.0f, x);
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
		return (CUBE(x) * (x * (x * 6 - 15) + 10));
	}

	const float Luma(const Vector3& color)
	{
		// Use Rec.709 trichromat formula to get perceptive luma value.
		return float((color.x * 0.2126f) + (color.y * 0.7152f) + (color.z * 0.0722f));
	}

	const Vector3 Screen(const Vector3& ambient, const Vector3& tint)
	{
		float luma = Luma(tint);

		auto multiplicative = ambient * tint;
		auto additive = ambient + tint;

		float r = Lerp(multiplicative.x, additive.x, luma);
		float g = Lerp(multiplicative.y, additive.y, luma);
		float b = Lerp(multiplicative.z, additive.z, luma);
		return Vector3(r, g, b);
	}

	const Vector4 Screen(const Vector4& ambient, const Vector4& tint)
	{
		auto result = Screen(Vector3(ambient), Vector3(tint));
		return Vector4(result.x, result.y, result.z, ambient.w * tint.w);
	}
}
