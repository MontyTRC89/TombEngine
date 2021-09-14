#pragma once
#include <SimpleMath.h>

using namespace DirectX::SimpleMath;

namespace TEN {
namespace Effects {
namespace Sky {

	class SkyController
	{
	public:
		short Position1;
		short Position2;
		Vector4 Color;

		void UpdateSky();

	private:

		int LightningCount;
		int LightningRand;

		int StormTimer;
		byte SkyStormColor = 1;
		byte SkyStormColor2 = 1;

		void UpdateStorm();
	};

	extern SkyController Sky;
}}}