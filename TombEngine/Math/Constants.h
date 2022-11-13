#pragma once
#include <climits>

//namespace TEN::Math
//{
	// Math constants
	constexpr inline auto PI	   = 3.14159265358979323846264338327950288419716939937510f;
	constexpr inline auto PI_MUL_2 = PI * 2;
	constexpr inline auto PI_DIV_2 = PI / 2;
	constexpr inline auto PI_DIV_4 = PI / 4;
	constexpr inline auto RADIAN   = PI / 180;
	constexpr inline auto SQRT_2   = 1.41421356237309504880168872420969807856967187537694f;

	constexpr inline auto SQUARE = [](auto x) { return (x * x); };
	constexpr inline auto CUBE	 = [](auto x) { return (x * x * x); };

	// World constants
	constexpr inline auto BLOCK_UNIT = 1024;
	constexpr inline auto NO_HEIGHT	 = INT_MIN + UCHAR_MAX;
	constexpr inline auto MAX_HEIGHT = INT_MIN + 1; // NOTE: +1 prevents issues with sign change.
	constexpr inline auto DEEP_WATER = INT_MAX - 1; // NOTE: -1 prevents issues with sign change.

	constexpr inline auto BLOCK	 = [](auto x) { return (BLOCK_UNIT * x); };
	constexpr inline auto SECTOR = [](auto x) { return BLOCK(x); }; // TODO: Replace with BLOCK() at some point.
	constexpr inline auto CLICK	 = [](auto x) { return ((BLOCK(1) / 4) * x); };

	constexpr inline auto STEP_SIZE		 = CLICK(1);
	constexpr inline auto STOP_SIZE		 = CLICK(2);
	constexpr inline auto WALL_MASK		 = BLOCK(1) - 1; // TODO: Rename to BLOCK_MASK?
	constexpr inline auto GRID_SNAP_SIZE = (int)BLOCK(1.0f / 8);
//}
