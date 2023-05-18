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
		Vector4						  Color		  = Vector4::Zero;
		BoundingOrientedBox			  BoundingBox = {};
		std::vector<LaserBarrierBeam> Beams		  = {};

		bool IsActive		  = false;
		bool IsLethal		  = false;
		bool IsHeavyActivator = false;
	};

	extern std::unordered_map<int, LaserBarrier> LaserBarriers;

	void InitializeLaserBarrier(short itemNumber);
	void ControlLaserBarrier(short itemNumber);
	void CollideLaserBarrier(short itemNumber, ItemInfo* playerItem, CollisionInfo* coll);

	void ClearLaserBarrierEffects();
}
