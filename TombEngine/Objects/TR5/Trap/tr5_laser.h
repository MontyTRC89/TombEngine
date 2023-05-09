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
		int lethal;
		BoundingOrientedBox				  BoundingBox = {};
		std::vector<LaserBarrierBeam> Beams = {};
		Vector4 Color = Vector4::Zero;
		bool On = false;
	};

	extern std::vector<LaserBarrier> LaserBarriers;

	void InitializeLaserBarriers(short itemNumber);
	void ControlLaserBarriers(short itemNumber);
	void CollideLaserBarriers(short itemNumber);
	void ClearLaserBarriers();
	void LaserBarrierLight(short itemNumber, int lightIntensity, int amplitude);
}
