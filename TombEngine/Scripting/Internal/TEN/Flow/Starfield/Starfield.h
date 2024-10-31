#pragma once

namespace sol { class state; }

namespace TEN::Scripting
{
	class Starfield
	{
	private:
		// Constants
		static const auto STAR_COUNT_MAX  = 6000;
		static const auto METEOR_COUNT_MAX = 100;

		// Members
		int	  _starCount		  = 0;
		int	  _meteorCount		  = 0;
		int	  _meteorSpawnDensity = 0;
		float _meteorVelocity	  = 0;

	public:
		static void Register(sol::table& table);

		// Constructors
		Starfield() = default;
		Starfield(int starCount);
		Starfield(int starCount, int meteorCount, int meteorSpawnDensity, float meteorVel);

		// Getters
		bool  GetStarsEnabled() const;
		bool  GetMeteorsEnabled() const;
		int	  GetStarCount() const;
		int	  GetMeteorCount() const;
		int	  GetMeteorSpawnDensity() const;
		float GetMeteorVelocity() const;

		// Setters
		void SetStarCount(int count);
		void SetMeteorCount(int count);
		void SetMeteorSpawnDensity(int spawnDensity);
		void SetMeteorVelocity(float vel);
	};
}
