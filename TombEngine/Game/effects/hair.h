#pragma once
#include "Math/Math.h"

using namespace TEN::Math;

struct AnimFrame;
struct ItemInfo;
struct SPHERE;

namespace TEN::Effects::Hair
{
	class HairUnit
	{
	private:
		// Constants
		static constexpr auto SEGMENT_COUNT_MAX = 6;
		static constexpr auto HAIR_GRAVITY		= 10.0f;

		struct HairSegment
		{
			Vector3		Position	= Vector3::Zero;
			EulerAngles Orientation = EulerAngles::Zero;
			Vector3		Velocity	= Vector3::Zero;
		};

	public:
		// Constants
		static constexpr auto SPHERE_COUNT_MAX = 6;

		// Components
		bool IsInitialized = false;
		bool IsEnabled	   = false;

		std::array <HairSegment, SEGMENT_COUNT_MAX + 1> Segments = {};

		// Utilities
		void Update(const ItemInfo& item, int hairUnitIndex);

	private:
		// Helpers
		AnimFrame*							 GetFramePtr(const ItemInfo& item);
		std::array<SPHERE, SPHERE_COUNT_MAX> GetSpheres(const ItemInfo& item, bool isYoung);
		
		void UpdateSegments(const ItemInfo& item, int hairUnitIndex, bool isYoung);
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
