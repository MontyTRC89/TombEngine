#include "framework.h"
#include "camera.h"
#include "savegame.h"
#include "weather.h"
#include "collide.h"
#include "effects/effects.h"
#include "Sound/sound.h"
#include "Specific/prng.h"
#include "Specific/setup.h"
#include "Scripting/GameScriptLevel.h"

using namespace TEN::Math::Random;

namespace TEN {
namespace Effects {
namespace Environment 
{
	EnvironmentController Weather;

	float WeatherParticle::Transparency() const
	{
		float result = WEATHER_PARTICLES_TRANSPARENCY;

		if (Life <= WEATHER_PARTICLES_NEAR_DEATH_LIFE_VALUE)
			result *= Life / (float)WEATHER_PARTICLES_NEAR_DEATH_LIFE_VALUE;

		if ((StartLife - Life) < (float)WEATHER_PARTICLES_NEAR_DEATH_LIFE_VALUE)
			result *= (StartLife - Life) / (float)WEATHER_PARTICLES_NEAR_DEATH_LIFE_VALUE;

		if (Type == WeatherType::Rain)
			result *= 0.45f;

		return result;
	}

	EnvironmentController::EnvironmentController()
	{
		Particles.reserve(WEATHER_PARTICLES_MAX_COUNT);
	}

	void EnvironmentController::Update()
	{
		GameScriptLevel* level = g_GameFlow->GetLevel(CurrentLevel);

		UpdateSky(level);
		UpdateStorm(level);
		UpdateWind(level);
		UpdateFlash(level);
		UpdateWeather(level);
		SpawnWeatherParticles(level);
	}

	void EnvironmentController::Clear()
	{
		// Clear storm vars
		StormTimer     = 0;
		StormSkyColor  = 1;
		StormSkyColor2 = 1;

		// Clear wind vars
		WindCurrent = WindX = WindZ = 0;
		WindAngle = WindDAngle = 2048;

		// Clear flash vars
		FlashProgress = 0.0f;
		FlashColorBase = Vector3::Zero;

		// Clear weather
		Particles.clear();
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
			if (SkyPosition1 <= SKY_POSITION_LIMIT)
			{
				if (SkyPosition1 < 0)
					SkyPosition1 += SKY_POSITION_LIMIT;
			}
			else
			{
				SkyPosition1 -= SKY_POSITION_LIMIT;
			}
		}

		if (level->Layer2.Enabled)
		{
			SkyPosition2 += level->Layer2.CloudSpeed;
			if (SkyPosition2 <= SKY_POSITION_LIMIT)
			{
				if (SkyPosition2 < 0)
					SkyPosition2 += SKY_POSITION_LIMIT;
			}
			else
			{
				SkyPosition2 -= SKY_POSITION_LIMIT;
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
			if (StormSkyColor > UCHAR_MAX)
				StormSkyColor = UCHAR_MAX;
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

		WindX = WindCurrent * phd_sin(WindAngle << 3);
		WindZ = WindCurrent * phd_cos(WindAngle << 3);
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
		for (auto& p : Particles)
		{
			p.Life -= 2;

			// Disable particle if it is dead. It will be cleaned on next call of
			// SpawnWeatherParticles().

			if (p.Life <= 0)
			{
				p.Enabled = false;
				continue;
			}

			// Check if particle got out of collision check radius, and fade it out if it did.

			if (abs(Camera.pos.x - p.Position.x) > COLLISION_CHECK_DISTANCE ||
				abs(Camera.pos.z - p.Position.z) > COLLISION_CHECK_DISTANCE)
			{
				p.Life = std::clamp(p.Life, 0.0f, WEATHER_PARTICLES_NEAR_DEATH_LIFE_VALUE);
			}

			// If particle was locked (after landing or stucking in substance such as water or swamp),
			// fade it out and bypass any collision checks and movement updates.

			if (p.Stopped)
			{
				if (p.Type == WeatherType::Snow)
					p.Size *= WEATHER_PARTICLES_NEAR_DEATH_MELT_FACTOR;

				continue;
			}

			// Backup old position and progress new position according to velocity.

			auto oldPos = p.Position;
			p.Position.x += p.Velocity.x;
			p.Position.y += ((int)p.Velocity.y & (~7)) >> 1;
			p.Position.z += p.Velocity.z;

			COLL_RESULT coll;
			bool collisionCalculated = false;

			if (p.CollisionCheckDelay <= 0)
			{
				coll = GetCollisionResult(p.Position.x, p.Position.y, p.Position.z, p.Room);

				// Determine collision checking frequency based on nearest floor/ceiling surface position.
				// If floor and ceiling is too far, don't do precise collision checks, instead doing it 
				// every 5th frame. If particle approaches floor or ceiling, make checks more frequent.
				// This allows to avoid unnecessary thousands of calls to GetCollisionResult for every particle.
				
				auto coeff = std::min(std::max(0.0f, (coll.Position.Floor - p.Position.y)), std::max(0.0f, (p.Position.y - coll.Position.Ceiling)));
				p.CollisionCheckDelay = std::min(floor(coeff / std::max(std::numeric_limits<float>::denorm_min(), p.Velocity.y)), WEATHER_PARTICLES_MAX_COLL_CHECK_DELAY);
				collisionCalculated = true;
			}
			else
				p.CollisionCheckDelay--;

			auto& r = g_Level.Rooms[p.Room];

			// Check if particle got out of room bounds

			if (p.Position.y <= (r.maxceiling - STEP_SIZE) || p.Position.y >= (r.minfloor + STEP_SIZE) ||
				p.Position.z <= (r.z + WALL_SIZE) || p.Position.z >= (r.z + ((r.zSize - 1) << 10)) ||
				p.Position.x <= (r.x + WALL_SIZE) || p.Position.x >= (r.x + ((r.xSize - 1) << 10)))
			{
				if (!collisionCalculated)
				{
					coll = GetCollisionResult(p.Position.x, p.Position.y, p.Position.z, p.Room);
					collisionCalculated = true;
				}

				if (coll.RoomNumber == p.Room)
				{
					p.Enabled = false; // Not landed on door, so out of room bounds - delete
					continue;
				}
				else
					p.Room = coll.RoomNumber;
			}

			// If collision was updated, process with position checks.

			if (collisionCalculated)
			{
				// If particle is inside water or swamp, count it as "inSubstance".
				// If particle got below floor or above ceiling, count it as "landed".

				bool inSubstance = g_Level.Rooms[coll.RoomNumber].flags & (ENV_FLAG_WATER | ENV_FLAG_SWAMP);
				bool landed = (coll.Position.Floor <= p.Position.y) || (coll.Position.Ceiling >= p.Position.y);

				if (inSubstance || landed)
				{
					p.Stopped = true;
					p.Position = oldPos;
					p.Life = std::clamp(p.Life, 0.0f, WEATHER_PARTICLES_NEAR_DEATH_LIFE_VALUE);

					// Produce ripples if particle got into substance (water or swamp).

					if (inSubstance)
					{
						SetupRipple(p.Position.x, p.Position.y, p.Position.z, GenerateFloat(16, 24),
							RIPPLE_FLAG_SHORT_LIFE | RIPPLE_FLAG_RAND_ROT | RIPPLE_FLAG_LOW_OPACITY,
							Objects[ID_DEFAULT_SPRITES].meshIndex + SPR_RIPPLES);
					}

					// Immediately disable rain particle because it doesn't need fading out.

					if (p.Type == WeatherType::Rain)
						p.Enabled = false;
				}
			}

			// Update velocities for every particle type.

			switch (p.Type)
			{
			case WeatherType::Snow:

				if (p.Velocity.x < (WindX << 2))
					p.Velocity.x += GenerateFloat(0.5f, 2.5f);
				else if (p.Velocity.x > (WindX << 2))
					p.Velocity.x -= GenerateFloat(0.5f, 2.5f);

				if (p.Velocity.z < (WindZ << 2))
					p.Velocity.z += GenerateFloat(0.5f, 2.5f);
				else if (p.Velocity.z > (WindZ << 2))
					p.Velocity.z -= GenerateFloat(0.5f, 2.5f);

				if (p.Velocity.y < p.Size / 2)
					p.Velocity.y += p.Size / 5.0f;

				break;

			case WeatherType::Rain:

				auto random = GenerateInt();
				if ((random & 3) != 3)
				{
					p.Velocity.x += (float)((random & 3) - 1);
					if (p.Velocity.x < -4)
						p.Velocity.x = -4;
					else if (p.Velocity.x > 4)
						p.Velocity.x = 4;
				}

				random = (random >> 2) & 3;
				if (random != 3)
				{
					p.Velocity.z += random - 1;
					if (p.Velocity.z < -4)
						p.Velocity.z = -4;
					else if (p.Velocity.z > 4)
						p.Velocity.z = 4;
				}

				if (p.Velocity.y < p.Size * 2 * std::clamp(level->WeatherStrength, 0.6f, 1.0f))
					p.Velocity.y += p.Size / 5.0f;

				break;
			}
		}
	}

	void EnvironmentController::SpawnWeatherParticles(GameScriptLevel* level)
	{
		// Clean up dead particles
		if (Particles.size() > 0)
			Particles.erase(std::remove_if(Particles.begin(), Particles.end(), [](const WeatherParticle& part) { return !part.Enabled; }), Particles.end());

		if (level->Weather == WeatherType::None || level->WeatherStrength == 0.0f)
			return;

		int newParticlesCount = 0;
		int density = WEATHER_PARTICLES_SPAWN_DENSITY * level->WeatherStrength;

		// Snow is falling twice as fast, and must be spawned accordingly fast
		if (level->Weather == WeatherType::Snow)
			density *= 2;

		if (density > 0.0f && level->Weather != WeatherType::None)
		{
			while (Particles.size() < WEATHER_PARTICLES_MAX_COUNT)
			{
				if (newParticlesCount > density)
					break;

				newParticlesCount++;

				auto distance = level->Weather == WeatherType::Snow ? COLLISION_CHECK_DISTANCE : COLLISION_CHECK_DISTANCE / 2;
				auto radius = GenerateInt(0, distance);
				short angle = GenerateInt(ANGLE(0), ANGLE(180));

				auto xPos = Camera.pos.x + ((int)(phd_cos(angle) * radius));
				auto zPos = Camera.pos.z + ((int)(phd_sin(angle) * radius));
				auto yPos = Camera.pos.y - (WALL_SIZE * 4 + GenerateInt() & (WALL_SIZE * 4 - 1));
				
				auto outsideRoom = IsRoomOutside(xPos, yPos, zPos);

				if (outsideRoom == NO_ROOM)
					continue;

				if (g_Level.Rooms[outsideRoom].flags & (ENV_FLAG_WATER | ENV_FLAG_SWAMP))
					continue;

				auto coll = GetCollisionResult(xPos, yPos, zPos, outsideRoom);

				if (!(coll.Position.Ceiling < yPos || coll.Block->RoomAbove(xPos, yPos, zPos).value_or(NO_ROOM) != NO_ROOM))
					continue;

				auto part = WeatherParticle();

				switch (level->Weather)
				{
				case WeatherType::Snow:
					part.Size = GenerateFloat(MAX_SNOW_SIZE / 3, MAX_SNOW_SIZE);
					part.Velocity.y = GenerateFloat(SNOW_SPEED / 4, SNOW_SPEED) * (part.Size / MAX_SNOW_SIZE);
					part.Life = (SNOW_SPEED / 3) + ((SNOW_SPEED / 2) - ((int)part.Velocity.y >> 2));
					break;

				case WeatherType::Rain:
					part.Size = GenerateFloat(MAX_RAIN_SIZE / 2, MAX_RAIN_SIZE);
					part.Velocity.y = GenerateFloat(RAIN_SPEED / 2, RAIN_SPEED) * (part.Size / MAX_RAIN_SIZE) * std::clamp(level->WeatherStrength, 0.6f, 1.0f);
					part.Life = (RAIN_SPEED * 2) - part.Velocity.y;
					break;
				}

				part.Velocity.x = GenerateFloat(WEATHER_PARTICLE_HORIZONTAL_SPEED / 2, WEATHER_PARTICLE_HORIZONTAL_SPEED);
				part.Velocity.z = GenerateFloat(WEATHER_PARTICLE_HORIZONTAL_SPEED / 2, WEATHER_PARTICLE_HORIZONTAL_SPEED);

				part.Type = level->Weather;
				part.Room = outsideRoom;
				part.Position.x = xPos;
				part.Position.y = yPos;
				part.Position.z = zPos;
				part.Stopped = false;
				part.Enabled = true;
				part.CollisionCheckDelay = 0;
				part.StartLife = part.Life;

				Particles.push_back(part);
			}
		}
	}
}}}