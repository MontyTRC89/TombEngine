#include "framework.h"
#include "Math/Interpolation.h"

#include "Math/Constants.h"

namespace TEN::Math
{
	float Lerp(float value0, float value1, float alpha)
	{
		return (((1.0f - alpha) * value0) + (alpha * value1));
	}

	float InterpolateCos(float value0, float value1, float alpha)
	{
		return Lerp(value0, value1, (1 - cos(alpha * PI)) * 0.5f);
	}

	float InterpolateCubic(float value0, float value1, float value2, float value3, float alpha)
	{
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
}
