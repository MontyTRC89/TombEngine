#pragma once

//namespace TEN::Math
//{
	constexpr inline auto PI		= 3.14159265358979323846264338327950288419716939937510L;
	constexpr inline auto PI_MULT_2 = PI * 2;
	constexpr inline auto PI_DIV_2	= PI / 2;
	constexpr inline auto PI_DIV_4	= PI / 4;
	constexpr inline auto RADIAN	= PI / 180;
	constexpr inline auto SQRT_2	= 1.41421356237309504880168872420969807856967187537694L;

	constexpr inline auto BLOCK_UNIT	 = 1024;
	constexpr inline auto QTR_BLOCK_UNIT = BLOCK_UNIT / 4;
	constexpr inline auto BLOCK			 = [](auto x) { return int(BLOCK_UNIT * x); };
	constexpr inline auto QTR_BLOCK		 = [](auto x) { return int(QTR_BLOCK_UNIT * x); };
//}
