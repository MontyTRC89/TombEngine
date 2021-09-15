#include "framework.h"
#include "savegame.h"
#include "weather.h"
#include "Sound\sound.h"
#include "Scripting\GameScriptLevel.h"

using namespace TEN::Effects::Environment;

namespace TEN {
namespace Effects {
namespace Environment 
{
	EnvironmentController Weather;

	void EnvironmentController::Update()
	{
		GameScriptLevel* level = g_GameFlow->GetLevel(CurrentLevel);

		UpdateSky(level);
		UpdateStorm(level);
		UpdateWind(level);
		UpdateFlash(level);
	}

	void EnvironmentController::Clear()
	{
		// Clear storm vars
		StormTimer     = 0;
		StormSkyColor  = 1;
		StormSkyColor2 = 1;

		// Clear wind vars
		WindCurrent = WindFinalX = WindFinalZ = 0;
		WindAngle = WindDAngle = 2048;

		// Clear flash vars
		FlashProgress = 0.0f;
		FlashColorBase = Vector4::Zero;
	}

	void EnvironmentController::Flash(int r, int g, int b, float speed)
	{
		FlashProgress = 1.0f;
		FlashSpeed = std::clamp(speed, 0.005f, 1.0f);
		FlashColorBase = Vector4(std::clamp(r, 0, UCHAR_MAX) / (float)UCHAR_MAX,
							     std::clamp(g, 0, UCHAR_MAX) / (float)UCHAR_MAX,
								 std::clamp(b, 0, UCHAR_MAX) / (float)UCHAR_MAX,
								 1.0f);
	}

	void EnvironmentController::UpdateSky(GameScriptLevel* level)
	{
		if (level->Layer1.Enabled)
		{
			SkyPosition1 += level->Layer1.CloudSpeed;
			if (SkyPosition1 <= 9728)
			{
				if (SkyPosition1 < 0)
					SkyPosition1 += 9728;
			}
			else
			{
				SkyPosition1 -= 9728;
			}
		}

		if (level->Layer2.Enabled)
		{
			SkyPosition2 += level->Layer2.CloudSpeed;
			if (SkyPosition2 <= 9728)
			{
				if (SkyPosition2 < 0)
					SkyPosition2 += 9728;
			}
			else
			{
				SkyPosition2 -= 9728;
			}
		}
	}

	void EnvironmentController::UpdateStorm(GameScriptLevel* level)
	{
		auto color = Vector4(level->Layer1.R / 255.0f, level->Layer1.G / 255.0f, level->Layer1.B / 255.0f, 1.0f);

		if (level->Storm)
		{
			if (StormCount || StormRand)
			{
				UpdateLightning();
				if (StormTimer > -1)
					StormTimer--;
				if (!StormTimer)
					SoundEffect(SFX_TR4_THUNDER_RUMBLE, NULL, 0);
			}
			else if (!(rand() & 0x7F))
			{
				StormCount = (rand() & 0x1F) + 16;
				StormTimer = (rand() & 3) + 12;
			}

			auto flashBrightness = StormSkyColor / 255.0f;
			auto r = std::clamp(color.x + flashBrightness, 0.0f, 1.0f);
			auto g = std::clamp(color.y + flashBrightness, 0.0f, 1.0f);
			auto b = std::clamp(color.z + flashBrightness, 0.0f, 1.0f);

			SkyCurrentColor = Vector4(r, g, b, color.w);
		}
		else
			SkyCurrentColor = color;
	}

	void EnvironmentController::UpdateLightning()
	{
		StormCount--;

		if (StormCount <= 0)
		{
			StormSkyColor = 0;
			StormRand = 0;
		}
		else if (StormCount < 5 && StormSkyColor < 50)
		{
			auto newColor = StormSkyColor - StormCount * 2;
			if (newColor < 0)
				newColor = 0;
			StormSkyColor = newColor;
		}
		else if (StormCount)
		{
			StormRand = ((rand() & 0x1FF - StormRand) >> 1) + StormRand;
			StormSkyColor2 += StormRand * StormSkyColor2 >> 8;
			StormSkyColor = StormSkyColor2;
			if (StormSkyColor > 255)
				StormSkyColor = 255;
		}
	}

	void EnvironmentController::UpdateWind(GameScriptLevel* level)
	{
		WindCurrent += (GetRandomControl() & 7) - 3;
		if (WindCurrent <= -2)
			WindCurrent++;
		else if (WindCurrent >= 9)
			WindCurrent--;

		WindDAngle = (WindDAngle + 2 * (GetRandomControl() & 63) - 64) & 0x1FFE;

		if (WindDAngle < 1024)
			WindDAngle = 2048 - WindDAngle;
		else if (WindDAngle > 3072)
			WindDAngle += 6144 - 2 * WindDAngle;

		WindAngle = (WindAngle + ((WindDAngle - WindAngle) >> 3)) & 0x1FFE;

		WindFinalX = WindCurrent * phd_sin(WindAngle << 3);
		WindFinalZ = WindCurrent * phd_cos(WindAngle << 3);
	}

	void EnvironmentController::UpdateFlash(GameScriptLevel* level)
	{
		if (FlashProgress > 0.0f)
		{
			FlashProgress -= FlashSpeed;
			if (FlashProgress < 0.0f)
				FlashProgress = 0.0f;
		}
	}
}}}