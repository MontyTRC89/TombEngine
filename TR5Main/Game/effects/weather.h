#pragma once
#include <SimpleMath.h>
#include "Scripting/GameScriptLevel.h"

namespace TEN {
namespace Effects {
namespace Environment 
{
	class EnvironmentController
	{
	public:
		Vector3 Wind() { return Vector3(WindFinalX / 2.0f, 0, WindFinalZ / 2.0f); }
		Vector3 FlashColor() { return FlashColorBase * sin(FlashProgress * PI / 2.0f); }
		Vector4 SkyColor() { return SkyCurrentColor; }
		short   SkyLayer1Position() { return SkyPosition1; }
		short   SkyLayer2Position() { return SkyPosition2; }

		void Flash(int r, int g, int b, float speed);
		void Update();
		void Clear();

	private:
		// Sky
		Vector4 SkyCurrentColor;
		short   SkyPosition1;
		short   SkyPosition2;

		// Wind
		int WindFinalX;
		int WindFinalZ;
		int WindAngle;
		int WindDAngle;
		int WindCurrent;

		// Flash fader
		Vector3 FlashColorBase = Vector3::Zero;
		float   FlashSpeed = 1.0f;
		float   FlashProgress = 0.0f;

		// Lightning
		int  StormCount;
		int  StormRand;
		int  StormTimer;
		byte StormSkyColor = 1;
		byte StormSkyColor2 = 1;

		void UpdateSky(GameScriptLevel* level);
		void UpdateStorm(GameScriptLevel* level);
		void UpdateWind(GameScriptLevel* level);
		void UpdateFlash(GameScriptLevel* level);
		void UpdateLightning();
	};

	extern EnvironmentController Weather;
}}}