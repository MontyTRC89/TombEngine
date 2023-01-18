#include "framework.h"
#include "Math/Math.h"

namespace TEN::Math
{
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
