#pragma once
#include <SimpleMath.h>
#include "Scripting/GameScriptLevel.h"

namespace TEN {
namespace Effects {
namespace Environment 
{
	constexpr auto WEATHER_PARTICLES_MAX_COUNT = 1024;
	constexpr auto SNOW_PARTICLES_COUNT_DIVIDER = 2;

	constexpr auto SNOW_SPAWN_DENSITY = 16;
	constexpr auto RAIN_SPAWN_DENSITY = 32;

	constexpr auto MAX_SNOW_SIZE = 32;
	constexpr auto MAX_RAIN_SIZE = 128;

	constexpr auto WEATHER_PARTICLES_TRANSPARENCY = 0.8f;
	constexpr auto WEATHER_PARTICLES_NEAR_DEATH_LIFE_VALUE = 16;

	constexpr auto SKY_POSITION_LIMIT = 9728;

	struct WeatherParticle
	{
		WeatherType Type;

		int Room;
		Vector3 Position;
		Vector3 Velocity;

		float StartLife;
		float Life;
		float Size;

		bool Enabled;
		bool Stopped;

		float Transparency() const;
	};

	class EnvironmentController
	{
	public:
		EnvironmentController();

		Vector3 Wind() { return Vector3(WindFinalX / 2.0f, 0, WindFinalZ / 2.0f); }
		Vector3 FlashColor() { return FlashColorBase * sin(FlashProgress * PI / 2.0f); }
		Vector4 SkyColor() { return SkyCurrentColor; }
		short   SkyLayer1Position() { return SkyPosition1; }
		short   SkyLayer2Position() { return SkyPosition2; }

		void Flash(int r, int g, int b, float speed);
		void Update();
		void Clear();

		const std::vector<WeatherParticle>& GetParticles() const { return Particles; }

	private:
		// Weather
		std::vector<WeatherParticle> Particles;

		// Sky
		Vector4 SkyCurrentColor = Vector4::Zero;
		short   SkyPosition1 = 0;
		short   SkyPosition2 = 0;

		// Wind
		int WindFinalX = 0;
		int WindFinalZ = 0;
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

		void UpdateSky(GameScriptLevel* level);
		void UpdateStorm(GameScriptLevel* level);
		void UpdateWind(GameScriptLevel* level);
		void UpdateFlash(GameScriptLevel* level);
		void UpdateWeather(GameScriptLevel* level);
		void UpdateLightning();

		void SpawnWeatherParticles(GameScriptLevel* level);
	};

	extern EnvironmentController Weather;
}}}