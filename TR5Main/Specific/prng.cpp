#include "framework.h"
#include "Specific/prng.h"

#include <random>

namespace TEN::Math::Random
{
	static std::mt19937 Engine;

	int32_t GenerateInt(int32_t low, int32_t high)
	{
		return Engine() / (Engine.max() / (high - low + 1) + 1) + low;
	}

	float GenerateFloat(float low, float high)
	{
		return (high - low) * Engine() / Engine.max() + low;
	}
}
