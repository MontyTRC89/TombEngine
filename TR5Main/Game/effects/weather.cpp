#include "framework.h"
#include "camera.h"
#include "savegame.h"
#include "weather.h"
#include "collide.h"
#include "Sound\sound.h"
#include "Scripting\GameScriptLevel.h"

namespace TEN {
namespace Effects {
namespace Environment 
{
	EnvironmentController Weather;

	EnvironmentController::EnvironmentController()
	{
		Particles.reserve(WEATHER_PARTICLES_COUNT);
	}

	void EnvironmentController::Update()
	{
		GameScriptLevel* level = g_GameFlow->GetLevel(CurrentLevel);

		UpdateSky(level);
		UpdateStorm(level);
		UpdateWind(level);
		UpdateFlash(level);
		UpdateWeather(level);
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
		FlashColorBase = Vector3::Zero;
	}

	void EnvironmentController::Flash(int r, int g, int b, float speed)
	{
		FlashProgress = 1.0f;
		FlashSpeed = std::clamp(speed, 0.005f, 1.0f);
		FlashColorBase = Vector3(std::clamp(r, 0, UCHAR_MAX) / (float)UCHAR_MAX,
							     std::clamp(g, 0, UCHAR_MAX) / (float)UCHAR_MAX,
								 std::clamp(b, 0, UCHAR_MAX) / (float)UCHAR_MAX);
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

		if (FlashProgress == 0.0f)
			FlashColorBase = Vector3::Zero;
	}

	void EnvironmentController::UpdateWeather(GameScriptLevel* level)
	{
		int newParticlesCount = 0;

		while (Particles.size() < WEATHER_PARTICLES_COUNT)
		{
			if (newParticlesCount >= 16)
				break;

			newParticlesCount++;

			auto radius = (GetRandomDraw() & (WALL_SIZE * 8 - 1)) - WALL_SIZE * 4;
			short angle  = (GetRandomDraw() & 0xFFFF);

			auto xPos = Camera.pos.x + ((int)(phd_cos(angle) * radius));
			auto zPos = Camera.pos.z + ((int)(phd_sin(angle) * radius));
			auto yPos = Camera.pos.y - (WALL_SIZE * 2 + GetRandomDraw() & (WALL_SIZE * 2 - 1));


			if (IsRoomOutside(xPos, yPos, zPos) < 0)
				continue;

			if (g_Level.Rooms[IsRoomOutsideNo].flags & ENV_FLAG_WATER)
				continue;

			auto part = WeatherParticle();

			part.Room = IsRoomOutsideNo;
			part.Position.x = xPos;
			part.Position.y = yPos;
			part.Position.z = zPos;
			part.Stopped = false;
			part.Enabled = true;
			part.Velocity.x =  (GetRandomDraw() & 7) - 4;
			part.Velocity.y = ((GetRandomDraw() & 15) + 8) << 3;
			part.Velocity.z =  (GetRandomDraw() & 7) - 4;
			part.Life = 48 + (64 - ((int)part.Velocity.y >> 2));

			part.Type = WeatherType::Snow;
			part.Size = 16 + (GetRandomDraw() & 7 - 4);

			Particles.push_back(part);
		}

		for (int i = 0; i < Particles.size(); i++)
		{
			auto& p = Particles[i];

			auto oldPos = p.Position;

			if (!p.Stopped)
			{
				p.Position.x += p.Velocity.x;
				p.Position.y += ((int)p.Velocity.y & (~7)) >> 1;
				p.Position.z += p.Velocity.z;
			}

			auto& r = g_Level.Rooms[p.Room];

			if (p.Position.y <= r.maxceiling || p.Position.y >= r.minfloor ||
				p.Position.z <= (r.z + WALL_SIZE) || p.Position.z >= (r.z + ((r.zSize - 1) << 10)) ||
				p.Position.x <= (r.x + WALL_SIZE) || p.Position.x >= (r.x + ((r.xSize - 1) << 10)))
			{
				auto coll = GetCollisionResult(p.Position.x, p.Position.y, p.Position.z, p.Room);
				bool inSubstance = g_Level.Rooms[coll.RoomNumber].flags & (ENV_FLAG_WATER | ENV_FLAG_SWAMP);
				bool landed = coll.Position.Floor < p.Position.y;

				if (coll.RoomNumber == p.Room)
				{
					p.Enabled = false; // Spawned in same room, needs to be on portal
					continue;
				}
				else if (inSubstance || landed)
				{
					p.Stopped = true;
					p.Position = oldPos;
					p.Life = (p.Life > 16) ? 16 : p.Life;
				}
				else
					p.Room = coll.RoomNumber;
			}

			if (!p.Life ||
				abs(Camera.pos.x - p.Position.x) > 6000 ||
				abs(Camera.pos.z - p.Position.z) > 6000)
			{
				if (!p.Life)
				{
					p.Enabled = false;	// Turn it off.
					continue;
				}
				else if (p.Life > 16)
					p.Life = 16;
			}

			if (!p.Stopped)
			{
				if (p.Velocity.x < (WindFinalX << 2))
					p.Velocity.x += 2;
				else if (p.Velocity.x > (WindFinalX << 2))
					p.Velocity.x -= 2;

				if (p.Velocity.z < (WindFinalZ << 2))
					p.Velocity.z += 2;
				else if (p.Velocity.z > (WindFinalZ << 2))
					p.Velocity.z -= 2;

				if (((int)p.Velocity.y & 7) < 7)
					p.Velocity.y++;
			}

			p.Life -= 2;
		}

		// Clean up dead particles
		if (Particles.size() > 0)
			Particles.erase(std::remove_if(Particles.begin(), Particles.end(), [](const WeatherParticle& part) { return !part.Enabled; }), Particles.end());
			// std::erase_if(Particles, [](const WeatherParticle& part) { return !part.Enabled; });
	}
}}}