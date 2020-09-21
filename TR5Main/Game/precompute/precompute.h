#pragma once
#include <array>
#include <random>
namespace T5M::Precomputed {
	namespace Random {
		constexpr auto randLUT = []
		{
			constexpr auto LUT_Size = 65535;
			std::array<float, LUT_Size> arr = {};
			constexpr auto a = 1103515245U;
			constexpr auto b = 12345U;
			for (int i = 0; i < LUT_Size; ++i)
			{
				long double c = ((a * i + b) % 0xFFFFFFFFU) / static_cast<long double>(0xFFFFFFFFU);
				arr[i] = static_cast<float>(c);
			}
			return arr;
		}();
	}
}