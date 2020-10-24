#include "framework.h"
#include "prng.h"
#include <limits>

namespace T5M::Math::Random
{
	static std::mt19937 Engine;

	int32_t generateInt(int32_t low, int32_t high)
	{
		return std::clamp(static_cast<int32_t>(Engine()), low, high);
	}

	float generateFloat(float low, float high)
	{
		return (high - low) * Engine() / Engine.max() + low;
	}
}
