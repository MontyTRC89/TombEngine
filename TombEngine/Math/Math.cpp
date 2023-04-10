#include "framework.h"
#include "Math/Math.h"

namespace TEN::Math
{
	float Luma(const Vector3& color)
	{
		constexpr auto RED_COEFF   = 0.2126f;
		constexpr auto GREEN_COEFF = 0.7152f;
		constexpr auto BLUE_COEFF  = 0.0722f;

		// Use Rec.709 trichromat formula to get perceptive luma value.
		return float((color.x * RED_COEFF) + (color.y * GREEN_COEFF) + (color.z * BLUE_COEFF));
	}

	Vector3 Screen(const Vector3& ambient, const Vector3& tint)
	{
		float luma = Luma(tint);
		auto multiplicative = ambient * tint;
		auto additive = ambient + tint;

		return Vector3(
			Lerp(multiplicative.x, additive.x, luma),
			Lerp(multiplicative.y, additive.y, luma),
			Lerp(multiplicative.z, additive.z, luma));
	}

	Vector4 Screen(const Vector4& ambient, const Vector4& tint)
	{
		auto result = Screen(Vector3(ambient), Vector3(tint));
		return Vector4(result.x, result.y, result.z, ambient.w * tint.w);
	}
}
