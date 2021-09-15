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

		// Sky

		if (level->Layer1.Enabled)
		{
			Position1 += level->Layer1.CloudSpeed;
			if (Position1 <= 9728)
			{
				if (Position1 < 0)
					Position1 += 9728;
			}
			else
			{
				Position1 -= 9728;
			}
		}

		if (level->Layer2.Enabled)
		{
			Position2 += level->Layer2.CloudSpeed;
			if (Position2 <= 9728)
			{
				if (Position2 < 0)
					Position2 += 9728;
			}
			else
			{
				Position2 -= 9728;
			}
		}

		auto color = Vector4(level->Layer1.R / 255.0f, level->Layer1.G / 255.0f, level->Layer1.B / 255.0f, 1.0f);

		// Storm

		if (level->Storm)
		{
			if (LightningCount || LightningRand)
			{
				UpdateStorm();
				if (StormTimer > -1)
					StormTimer--;
				if (!StormTimer)
					SoundEffect(SFX_TR4_THUNDER_RUMBLE, NULL, 0);
			}
			else if (!(rand() & 0x7F))
			{
				LightningCount = (rand() & 0x1F) + 16;
				StormTimer = (rand() & 3) + 12;
			}

			auto flashBrightness = SkyStormColor / 255.0f;
			auto r = std::clamp(color.x + flashBrightness, 0.0f, 1.0f);
			auto g = std::clamp(color.y + flashBrightness, 0.0f, 1.0f);
			auto b = std::clamp(color.z + flashBrightness, 0.0f, 1.0f);

			Color = Vector4(r, g, b, color.w);
		}
		else
			Color = color;

		// Wind

		CurrentWind += (GetRandomControl() & 7) - 3;
		if (CurrentWind <= -2)
			CurrentWind++;
		else if (CurrentWind >= 9)
			CurrentWind--;

		DWindAngle = (DWindAngle + 2 * (GetRandomControl() & 63) - 64) & 0x1FFE;

		if (DWindAngle < 1024)
			DWindAngle = 2048 - DWindAngle;
		else if (DWindAngle > 3072)
			DWindAngle += 6144 - 2 * DWindAngle;

		WindAngle = (WindAngle + ((DWindAngle - WindAngle) >> 3)) & 0x1FFE;

		FinalWindX = CurrentWind * phd_sin(WindAngle << 3);
		FinalWindZ = CurrentWind * phd_cos(WindAngle << 3);
	}

	void EnvironmentController::Clear()
	{
		StormTimer = 0;
		SkyStormColor = 1;
		SkyStormColor2 = 1;

		CurrentWind = FinalWindX = FinalWindZ = 0;
		WindAngle = DWindAngle = 2048;
	}

	void EnvironmentController::UpdateStorm()
	{
		LightningCount--;

		if (LightningCount <= 0)
		{
			SkyStormColor = 0;
			LightningRand = 0;
		}
		else if (LightningCount < 5 && SkyStormColor < 50)
		{
			auto newColor =SkyStormColor - LightningCount * 2;
			if (newColor < 0)
				newColor = 0;
			SkyStormColor = newColor;
		}
		else if (LightningCount)
		{
			LightningRand = ((rand() & 0x1FF - LightningRand) >> 1) + LightningRand;
			SkyStormColor2 += LightningRand * SkyStormColor2 >> 8;
			SkyStormColor = SkyStormColor2;
			if (SkyStormColor > 255)
				SkyStormColor = 255;
		}
	}

}}}