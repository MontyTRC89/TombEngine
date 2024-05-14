#include "framework.h"
#include "Math/Utils.h"

#include "Math/Constants.h"

namespace TEN::Math
{
	float Lerp(float value0, float value1, float alpha)
	{
		alpha = std::clamp(alpha, 0.0f, 1.0f);
		return (((1.0f - alpha) * value0) + (alpha * value1));
	}

	float InterpolateCos(float value0, float value1, float alpha)
	{
		alpha = std::clamp(alpha, 0.0f, 1.0f);
		return Lerp(value0, value1, (1 - cos(alpha * PI)) * 0.5f);
	}

	float InterpolateCubic(float value0, float value1, float value2, float value3, float alpha)
	{
		alpha = std::clamp(alpha, 0.0f, 1.0f);

		float p = (value3 - value2) - (value0 - value1);
		float q = (value0 - value1) - p;
		float r = value2 - value0;
		float s = value1;
		float x = alpha;
		float xSquared = SQUARE(x);
		return ((p * xSquared * x) + (q * xSquared) + (r * x) + s);
	}

	float Smoothstep(float alpha)
	{
		return Smoothstep(0.0f, 1.0f, alpha);
	}

	float Smoothstep(float value0, float value1, float alpha)
	{
		alpha = std::clamp(alpha, value0, value1);

		// Don't process if input value is same as one of the values.
		if (alpha == value0)
		{
			return value0;
		}
		else if (alpha == value1)
		{
			return value1;
		}

		// Scale, bias, and saturate alpha to [0, 1] range.
		alpha = std::clamp((alpha - value0) / (value1 - value0), 0.0f, 1.0f);

		// Evaluate polynomial.
		return (CUBE(alpha) * (alpha * (alpha * 6 - 15) + 10));
	}

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

	float FloorToStep(float value, float step)
	{
		return (floor(value / step) * step);
	}

	float CeilToStep(float value, float step)
	{
		return (ceil(value / step) * step);
	}

	float RoundToStep(float value, float step)
	{
		return (round(value / step) * step);
	}
}
