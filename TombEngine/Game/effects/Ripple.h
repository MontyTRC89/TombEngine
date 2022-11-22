#pragma once
#include "Specific/BitField.h"

using namespace TEN::Utils;

namespace TEN::Effects::Ripple
{
	constexpr auto RIPPLE_NUM_MAX = 256;

	enum RippleFlags
	{
		ShortInit  = 1,
		LowOpacity = 2,
		Blood	   = 3,
		NoRandom   = 4,

		Count
	};

	struct Ripple
	{
		bool		 IsActive	 = false;
		unsigned int SpriteIndex = 0;

		Vector3 Position = Vector3::Zero;
		Vector4 Color	 = Vector4::Zero;

		float Life	  = 0.0f;
		float Scale	  = 0.0f;
		float Opacity = 0.0f;
		float Init	  = 0.0f;

		BitField Flags = BitField(RippleFlags::Count);
	};

	extern std::array<Ripple, RIPPLE_NUM_MAX> Ripples;

	Ripple& GetFreeRipple();

	void SpawnRipple(const Vector3& pos, float scale, const std::vector<unsigned int>& flags = {});
	void SpawnUnderwaterBlood(const Vector3& pos, float scale);

	void UpdateRipples();
	void ClearRipples();
}
