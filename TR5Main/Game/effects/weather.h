#pragma once
#include <SimpleMath.h>

using namespace DirectX::SimpleMath;

namespace TEN {
namespace Effects {
namespace Environment {

	class EnvironmentController
	{
	public:
		Vector3 Wind() { return Vector3(FinalWindX / 2.0f, 0, FinalWindZ / 2.0f); }
		Vector4 SkyColor() { return Color; }
		short   SkyLayer1Position() { return Position1; }
		short   SkyLayer2Position() { return Position2; }

		void Update();
		void Clear();

	private:
		// Sky
		Vector4 Color;
		short Position1;
		short Position2;

		// Wind
		int FinalWindX;
		int FinalWindZ;
		int WindAngle;
		int DWindAngle;
		int CurrentWind;

		// Lightning
		int LightningCount;
		int LightningRand;
		int StormTimer;
		byte SkyStormColor = 1;
		byte SkyStormColor2 = 1;

		void UpdateStorm();
	};

	extern EnvironmentController Weather;
}}}