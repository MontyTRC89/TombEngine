#pragma once

namespace TEN::Traps::TR5
{
	constexpr auto LASER_BEAM_COUNT = 3;

	struct LaserStructInfo
	{
		std::array<Vector3, LASER_BEAM_COUNT> vert1 = {};
		std::array<Vector3, LASER_BEAM_COUNT> vert2 = {};
		std::array<Vector3, LASER_BEAM_COUNT> vert3 = {};
		std::array<Vector3, LASER_BEAM_COUNT> vert4 = {};

		Vector4 Color = Vector4::Zero;
		short Rand[18];
		int life = INFINITY;
	};

	extern std::vector<LaserStructInfo> Lasers;

	void InitializeLasers(short itemNumber);
	void ControlLasers(short itemNumber);
	void UpdateLasers();
	void ClearLasers();
}
