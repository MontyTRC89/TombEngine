#pragma once

namespace TEN::Traps::TR5
{
	struct LaserBarrierBeam
	{
		static constexpr auto VERTEX_COUNT = 4;

		std::array<Vector3, VERTEX_COUNT> VertexPoints = {};
	};

	struct LaserBarrier
	{
		std::vector<LaserBarrierBeam> Beams		  = {};
		BoundingOrientedBox			  BoundingBox = {};
		Vector4						  Color		  = Vector4::Zero;

		bool IsActive = false; // TODO: Might be unnecessary.
		int	Lethal; // TODO: Rename.
	};

	extern std::vector<LaserBarrier> LaserBarriers;

	void InitializeLaserBarriers(short itemNumber);
	void ControlLaserBarriers(short itemNumber);
	void CollideLaserBarriers(short itemNumber);
	void ClearLaserBarriers();
}
