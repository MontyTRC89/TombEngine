#pragma once

namespace TEN::Traps::TR5
{
	struct LaserBarrierBeam
	{
		static constexpr auto VERTEX_COUNT = 4;

		//Vector4							  Color		   = Vector4::Zero;
		BoundingOrientedBox				  BoundingBox  = {};
		std::array<Vector3, VERTEX_COUNT> VertexPoints = {};
	};

	struct LaserBarrier
	{
		std::vector<LaserBarrierBeam> Beams = {};
		Vector4 Color = Vector4::Zero;
	};

	extern std::vector<LaserBarrier> LaserBarriers;

	void InitializeLaserBarriers(short itemNumber);
	void ControlLaserBarriers(short itemNumber);
	void CollideLaserBarriers();
	void UpdateLaserBarriers();
	void ClearLaserBarriers();
}
