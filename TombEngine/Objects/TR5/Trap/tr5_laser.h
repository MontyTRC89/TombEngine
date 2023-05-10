#pragma once

struct CollisionInfo;
struct ItemInfo;

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

		bool IsActive = false;
		int	Lethal; // TODO: Rename.
	};

	extern std::unordered_map<int, LaserBarrier> LaserBarriers;

	void InitializeLaserBarrier(short itemNumber);
	void ControlLaserBarrier(short itemNumber);
	void CollideLaserBarrier(short itemNumber, ItemInfo* playerItem, CollisionInfo* coll);

	void ClearLaserBarriers();
}
