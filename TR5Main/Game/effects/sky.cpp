#include "framework.h"
#include "savegame.h"
#include "sky.h"
#include "Sound\sound.h"
#include "Scripting\GameScriptLevel.h"

using namespace TEN::Effects::Sky;

namespace TEN {
namespace Effects {
namespace Sky 
{
	SkyController Sky;

	void SkyController::UpdateSky()
	{
		GameScriptLevel* level = g_GameFlow->GetLevel(CurrentLevel);

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
	}

	void SkyController::UpdateStorm()
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