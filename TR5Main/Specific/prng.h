#pragma once
#include <limits>

namespace TEN::Math::Random
{
	int32_t generateInt(int32_t low = 0, int32_t high = std::numeric_limits<int16_t>::max());
	float generateFloat(float low = 0, float high = 1);
}
