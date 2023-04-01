#pragma once

constexpr auto LASER_BEAM_COUNT = 3;

	struct LaserStructInfo
	{
		std::array<Vector3, LASER_BEAM_COUNT> vert1 = {};
		std::array<Vector3, LASER_BEAM_COUNT> vert2 = {};
		std::array<Vector3, LASER_BEAM_COUNT> vert3 = {};
		std::array<Vector3, LASER_BEAM_COUNT> vert4 = {};
		short Rand[18];
	};
