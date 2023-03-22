#pragma once
#include "Math/Math.h"

using namespace TEN::Math;

struct AnimFrame;
struct ItemInfo;

namespace TEN::Effects::Hair
{
	class HairUnit
	{
	private:
		// Constants
		static constexpr auto HAIR_GRAVITY = 10.0f;

		struct HairSegment
		{
			Vector3		Position	= Vector3::Zero;
			EulerAngles Orientation = EulerAngles::Zero;
			Vector3		Velocity	= Vector3::Zero;
		};

	public:
		// Components
		bool IsEnabled	   = false;
		bool IsInitialized = false;
		std::vector<HairSegment> Segments = {};

		// Utilities
		void Update(const ItemInfo& item, int hairUnitIndex);

	private:
		// Helpers
		Vector3						GetRelBaseOffset(int hairUnitIndex, bool isYoung);
		std::vector<BoundingSphere> GetSpheres(const ItemInfo& item, bool isYoung);
		EulerAngles					GetOrientation(const Vector3& origin, const Vector3& target);
		Vector3						GetWaterProbeOffset(const ItemInfo& item);

		void CollideSegmentWithRoom(HairSegment& segment, int waterHeight, int roomNumber, bool isOnLand);
		void CollideSegmentWithSpheres(HairSegment& segment, const std::vector<BoundingSphere>& spheres);
	};

	class HairEffectController
	{
	private:
		// Constants
		static constexpr auto UNIT_COUNT_MAX = 2;

	public:
		// Components
		std::array<HairUnit, UNIT_COUNT_MAX> Units = {};

		// Utilities
		void Initialize();
		void Update(ItemInfo& item, bool isYoung);
	};

	extern HairEffectController HairEffect;
}
