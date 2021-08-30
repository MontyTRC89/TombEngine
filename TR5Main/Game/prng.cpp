#include "framework.h"
#include "prng.h"
#include <random>

namespace TEN::Math::Random
{
	static std::mt19937 Engine;

	int32_t generateInt(int32_t low, int32_t high)
	{
		return Engine() / (Engine.max() / (high - low + 1) + 1) + low;
	}

	float generateFloat(float low, float high)
	{
		return (high - low) * Engine() / Engine.max() + low;
	}
}
