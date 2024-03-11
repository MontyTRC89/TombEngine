#pragma once
#include "Math/Math.h"

using namespace TEN::Math;

struct CollisionInfo;
struct ItemInfo;

namespace TEN::Traps::TR5
{
	struct SingleLaserBeam
	{
		static constexpr auto VERTEX_COUNT = 4;
		static constexpr auto HEIGHT	   = CLICK(0.05f);

		std::array<Vector3, VERTEX_COUNT> VertexPoints = {};
	};

	struct SingleLaser
	{
		Vector4						  Color		  = Vector4::Zero;
		BoundingOrientedBox			  BoundingBox = {};
		std::vector<SingleLaserBeam> Beams		  = {};

		bool IsActive		  = false;
		bool IsLethal		  = false;
		bool IsHeavyActivator = false;

		void Initialize(const ItemInfo& item);
		void Update(const ItemInfo& item);
	};

	extern std::unordered_map<int, SingleLaser> LaserBeams;

	void InitializeSingleLaser(short itemNumber);
	void ControlSingleLaser(short itemNumber);
	void CollideSingleLaser(short itemNumber, ItemInfo* playerItem, CollisionInfo* coll);
	void TriggerLaserSpark(const GameVector& pos, short angle, int count, const Vector4& colorStart);
	void SpawnLaserBarrierLight(const ItemInfo& item, float intensity, float amplitude, const GameVector& pos);
	void ClearSingleLaserEffects();
}
