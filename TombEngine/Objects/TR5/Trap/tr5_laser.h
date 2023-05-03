#pragma once

namespace TEN::Traps::TR5
{
	constexpr auto LASER_BEAM_COUNT = 3;

	struct LaserBarrier
	{
		std::array<Vector3, LASER_BEAM_COUNT> vert1 = {};
		std::array<Vector3, LASER_BEAM_COUNT> vert2 = {};
		std::array<Vector3, LASER_BEAM_COUNT> vert3 = {};
		std::array<Vector3, LASER_BEAM_COUNT> vert4 = {};

		Vector4 Color = Vector4::Zero;
		short Rand[18];
	};

	extern std::vector<LaserBarrier> LaserBarriers;

	void InitializeLaserBarriers(short itemNumber);
	void ControlLaserBarriers(short itemNumber);
	void CollideLaserBarriers();
	void UpdateLaserBarriers();
	void ClearLaserBarriers();
}
