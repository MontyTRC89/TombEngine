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

	// TODO: Move these constants to lara.h.
	constexpr inline auto STEPUP_HEIGHT		  = (int)CLICK(3.0f / 2);
	constexpr inline auto BAD_JUMP_CEILING	  = (int)CLICK(6.0f / 8);
	constexpr inline auto SHALLOW_WATER_DEPTH = (int)CLICK(1.0f / 2);
	constexpr inline auto WADE_DEPTH		  = STEPUP_HEIGHT;
	constexpr inline auto SWIM_DEPTH		  = CLICK(3) - 38;

	constexpr inline auto GRID_SNAP_SIZE = (int)CLICK(1.0f / 2);
	constexpr inline auto STEP_SIZE		 = CLICK(1);
	constexpr inline auto STOP_SIZE		 = CLICK(2);
	constexpr inline auto WALL_SIZE		 = BLOCK(1);
	constexpr inline auto WALL_MASK		 = BLOCK(1) - 1;

	constexpr inline auto FPS					  = 30;
	constexpr inline auto PREDICTIVE_SCALE_FACTOR = 14;
	constexpr inline auto SLOPE_DIFFERENCE		  = 60;

//}
