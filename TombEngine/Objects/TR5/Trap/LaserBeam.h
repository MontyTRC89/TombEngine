#pragma once
#include "Math/Math.h"

using namespace TEN::Math;

struct CollisionInfo;
struct ItemInfo;

namespace TEN::Traps::TR5
{
	struct LaserBeamEffect
	{
		static constexpr auto SUBDIVISION_COUNT = 8;
		static constexpr auto VERTEX_COUNT = 4;

		Vector4				Color		= Vector4::Zero;
		BoundingOrientedBox BoundingBox = {};
		std::array<Vector3, VERTEX_COUNT> VertexPoints = {};

		bool IsActive		  = false;
		bool IsLethal		  = false;
		bool IsHeavyActivator = false;

		void Initialize(const ItemInfo& item);
		void Update(const ItemInfo& item);
	};

	extern std::unordered_map<int, LaserBeamEffect> LaserBeams;

	void InitializeLaserBeam(short itemNumber);
	void ControlLaserBeam(short itemNumber);
	void CollideLaserBeam(short itemNumber, ItemInfo* playerItem, CollisionInfo* coll);

	void SpawnLaserSpark(const GameVector& pos, short angle, int count, const Vector4& colorStart);
	void SpawnLaserBeamLight(const ItemInfo& item, float intensity, float amplitude, const GameVector& pos);

	void ClearLaserBeamEffects();
}
