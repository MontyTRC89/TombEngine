#pragma once
#include "Math/Math.h"
#include "Objects/Effects/LensFlare.h"
#include "Scripting/Include/ScriptInterfaceLevel.h"

using namespace TEN::Entities::Effects;

namespace TEN::Effects::Environment 
{
	constexpr auto WEATHER_PARTICLES_SPAWN_DENSITY = 32;
	constexpr auto WEATHER_PARTICLES_MAX_COUNT = 2048;
	constexpr auto WEATHER_PARTICLES_MAX_COLL_CHECK_DELAY = 5.0f;

	constexpr auto MAX_DUST_SIZE = 25.0f;
	constexpr auto MAX_SNOW_SIZE = 32.0f;
	constexpr auto MAX_RAIN_SIZE = 128.0f;

	constexpr auto WEATHER_PARTICLE_HORIZONTAL_SPEED = 8.0f;
	constexpr auto MAX_SNOW_SPEED = 128.0f;
	constexpr auto MAX_RAIN_SPEED = 256.0f;
	constexpr auto MAX_DUST_SPEED = 1.0f;

	constexpr auto WEATHER_PARTICLES_TRANSPARENCY = 0.8f;
	constexpr auto WEATHER_PARTICLES_NEAR_DEATH_LIFE_VALUE = 20.0f;
	constexpr auto WEATHER_PARTICLES_NEAR_DEATH_MELT_FACTOR = 1.0f - (1.0f / (WEATHER_PARTICLES_NEAR_DEATH_LIFE_VALUE * 2));

	constexpr auto DUST_SPAWN_DENSITY = 300;
	constexpr auto DUST_LIFE = 40;
	constexpr auto DUST_SPAWN_RADIUS = (10 * 1024);

	constexpr auto METEOR_PARTICLE_COUNT_MAX = 10;
	constexpr auto METEOR_PARTICLE_LIFE_MAX = 150;
	constexpr auto METEOR_PARTICLE_VELOCITY = 32.0f;
	constexpr auto METEOR_PARTICLE_SPAWN_DENSITY = 4;
	constexpr auto METEOR_PARTICLE_FADE_TIME = 30.0f;

	struct StarParticle
	{
		Vector3 Position  = Vector3::Zero;
		Vector3 Direction = Vector3::Zero;
		Vector3 Color	  = Vector3::Zero;

		float Extinction = 1.0f;
		float Scale		 = 1.0f;
		float Blinking	 = 1.0f;
	};

	struct MeteorParticle
	{
		Vector3 Position	  = Vector3::Zero;
		Vector3 Direction	  = Vector3::Zero;
		Vector3 StartPosition = Vector3::Zero;
		Vector3 Color		  = Vector3::Zero;

		bool  Active = false;
		float Life	 = 0.0f;
		float Fade	 = 0.0f;

		Vector3 PrevPosition = Vector3::Zero;
		float	PrevFade	 = 0.0f;

		void StoreInterpolationData()
		{
			PrevPosition = Position;
			PrevFade = Fade;
		}
	};

	struct WeatherParticle
	{
		WeatherType Type = WeatherType::None;

		int Room = NO_VALUE;
		Vector3 Position = Vector3::Zero;
		Vector3 Velocity = Vector3::Zero;

		float StartLife = 0.0f;
		float Life = 0.0f;
		float CollisionCheckDelay = 0.0f;
		float Size = 0.0f;

		bool Enabled = false;
		bool Stopped = false;

		float Transparency() const;

		Vector3 OldPosition = Vector3::Zero;
		Vector3 OldVelocity = Vector3::Zero;
		float OldSize = 0.0f;
		float OldLife = 0.0f;

		void StoreInterpolationData()
		{
			OldPosition = Position;
			OldVelocity = Velocity;
			OldSize = Size;
			OldLife = Life;
		}
	};

	class EnvironmentController
	{
	public:
		EnvironmentController();

		Vector3 Wind() { return Vector3(WindX / 2.0f, 0, WindZ / 2.0f); }
		Vector3 FlashColor() { return FlashColorBase * sin(FlashProgress * PI / 2.0f); }
		Vector4 SkyColor(int index) { return SkyCurrentColor[std::clamp(index, 0, 1)]; }
		short   SkyPosition(int index) { return SkyCurrentPosition[std::clamp(index, 0, 1)]; }

		void Flash(int r, int g, int b, float speed);
		void Update();
		void Clear();

		const std::vector<WeatherParticle>& GetParticles() const { return Particles; }
		const std::vector<StarParticle>& GetStars() const { return Stars; }
		const std::vector<MeteorParticle>& GetMeteors() { return Meteors; }

	private:
		// Weather
		std::vector<WeatherParticle> Particles;

		// Sky
		Vector4 SkyCurrentColor[2] = {};
		short   SkyCurrentPosition[2] = {};

		// Wind
		int WindX = 0;
		int WindZ = 0;
		int WindAngle = 0;
		int WindDAngle = 0;
		int WindCurrent = 0;

		// Flash fader
		Vector3 FlashColorBase = Vector3::Zero;
		float   FlashSpeed = 1.0f;
		float   FlashProgress = 0.0f;

		// Lightning
		int  StormCount = 0;
		int  StormRand = 0;
		int  StormTimer = 0;
		byte StormSkyColor = 1;
		byte StormSkyColor2 = 1;

		// Starfield
		std::vector<StarParticle>	Stars	= {};
		std::vector<MeteorParticle> Meteors = {};
		bool ResetStarField = true;

		// Lens flare
		LensFlare GlobalLensFlare = {};

		void UpdateStarfield(ScriptInterfaceLevel* level);
		void UpdateSky(ScriptInterfaceLevel* level);
		void UpdateStorm(ScriptInterfaceLevel* level);
		void UpdateWind(ScriptInterfaceLevel* level);
		void UpdateFlash(ScriptInterfaceLevel* level);
		void UpdateWeather(ScriptInterfaceLevel* level);
		void UpdateLightning();

		void SpawnDustParticles(ScriptInterfaceLevel* level);
		void SpawnWeatherParticles(ScriptInterfaceLevel* level);
		void SpawnMeteorParticles(ScriptInterfaceLevel* level);
	};

	extern EnvironmentController Weather;
}
