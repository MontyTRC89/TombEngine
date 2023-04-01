#pragma once

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
			Vector3	   Position	   = Vector3::Zero;
			Vector3	   Velocity	   = Vector3::Zero;
			Quaternion Orientation = Quaternion::Identity;
		};

	public:
		// Members
		bool IsEnabled	   = false;
		bool IsInitialized = false;
		std::vector<HairSegment> Segments = {};

		// Utilities
		void Update(const ItemInfo& item, int hairUnitIndex);

	private:
		// Helpers
		Vector3						GetRelBaseOffset(int hairUnitIndex, bool isYoung);
		Vector3						GetWaterProbeOffset(const ItemInfo& item);
		Quaternion					GetSegmentOrientation(const Vector3& origin, const Vector3& target, const Quaternion& baseOrient);
		std::vector<BoundingSphere> GetSpheres(const ItemInfo& item, bool isYoung);

		void CollideSegmentWithRoom(HairSegment& segment, int waterHeight, int roomNumber, bool isOnLand);
		void CollideSegmentWithSpheres(HairSegment& segment, const std::vector<BoundingSphere>& spheres);
	};

	class HairEffectController
	{
	private:
		// Constants
		static constexpr auto UNIT_COUNT_MAX = 2;

	public:
		// Members
		std::array<HairUnit, UNIT_COUNT_MAX> Units = {};

		// Utilities
		void Initialize();
		void Update(ItemInfo& item, bool isYoung);
	};

	extern HairEffectController HairEffect;
}
