#include "framework.h"
#include "Scripting/Internal/TEN/Flow/Starfield/Starfield.h"

#include "Game/effects/weather.h"
#include "Specific/level.h"

using namespace TEN::Effects::Environment;

/// Represents a starfield in the sky.
//
// @tenprimitive Flow.Starfield
// @pragma nostrip

namespace TEN::Scripting
{
	void Starfield::Register(sol::table& parent)
	{
		using ctors = sol::constructors<
			Starfield(int),
			Starfield(int, int, int, float)>;
		
		// Register type.
		parent.new_usertype<Starfield>(
			"Starfield",
			ctors(), sol::call_constructor, ctors(),

			"GetStarCount", &Starfield::GetStarCount,
			"GetMeteorCount", &Starfield::GetMeteorCount,
			"GetMeteorSpawnDensity", &Starfield::GetMeteorSpawnDensity,
			"GetMeteorVelocity", &Starfield::GetMeteorVelocity,
			"GetStarsEnabled", &Starfield::GetStarsEnabledStatus,
			"GetMeteorsEnabled", &Starfield::GetMeteorsEnabledStatus,

			"SetStarCount", &Starfield::SetStarCount,
			"SetMeteorCount", &Starfield::SetMeteorCount,
			"SetMeteorSpawnDensity", &Starfield::SetMeteorSpawnDensity,
			"SetMeteorVelocity", &Starfield::SetMeteorVelocity);
	}

	/// Create a starfield object with only stars.
	// @function Starfield
	// @tparam int starCount Star count.
	// @treturn Starfield A new Starfield object.
	Starfield::Starfield(int starCount)
	{
		_starCount = starCount;
	}

	/// Create a starfield object with stars and meteors.
	// @function Starfield
	// @tparam int starCount Star count. __Max: 6000__
	// @tparam int meteorCount Meteor count. __Max: 100__
	// @tparam int meteorSpawnDensity Meteor spawn density.
	// @tparam int meteorVel Meteor velocity.
	// @treturn Starfield A new Starfield object.
	Starfield::Starfield(int starCount, int meteorCount, int meteorSpawnDensity, float meteorVel)
	{
		if (starCount < 0 || starCount > STAR_COUNT_MAX)
			TENLog("Star count must be in range [0, " + std::to_string(STAR_COUNT_MAX) + "].", LogLevel::Warning);

		if (meteorCount < 0 || meteorCount > METEOR_COUNT_MAX)
			TENLog("Meteor count must be in range [0, " + std::to_string(METEOR_COUNT_MAX) + "].", LogLevel::Warning);

		_starCount = std::clamp(starCount, 0, STAR_COUNT_MAX);
		_meteorCount = std::clamp(meteorCount, 0, METEOR_COUNT_MAX);
		_meteorSpawnDensity = meteorSpawnDensity;
		_meteorVelocity = meteorVel;
	}

	/// Get this starfield's number of stars.
	// @function Starfield:GetStarCount
	// @treturn int Count.
	int Starfield::GetStarCount() const
	{
		return _starCount;
	}

	/// Get this starfield's number of meteors.
	// @function Starfield:GetMeteorCount
	// @treturn int Count.
	int Starfield::GetMeteorCount() const
	{
		return _meteorCount;
	}

	/// Get this starfield's meteor spawn density.
	// @function Starfield:GetMeteorSpawnDensity
	// @treturn int Spawn density.
	int Starfield::GetMeteorSpawnDensity() const
	{
		return _meteorSpawnDensity;
	}

	/// Get this starfield's meteor velocity.
	// @function Starfield:GetMeteorVelocity
	// @treturn float Velocity.
	float Starfield::GetMeteorVelocity() const
	{
		return _meteorVelocity;
	}

	/// Get this starfield's stars enabled status.
	// @function Starfield:GetStarsEnabled
	// @treturn bool Stars enabled status. __true: enabled__, __false: disabled__
	bool Starfield::GetStarsEnabledStatus() const
	{
		return (_starCount > 0);
	}

	/// Get this starfield's meteors enabled status.
	// @function Starfield:GetMeteorsEnabled
	// @treturn bool Meteors enabled status. __true: enabled__, __false: disabled__
	bool Starfield::GetMeteorsEnabledStatus() const
	{
		return (_meteorCount > 0);
	}

	/// Set this starfield's number of stars.
	// @function Starfield:SetStarCount
	// @tparam int count New star count.
	void Starfield::SetStarCount(int count)
	{
		if (count < 0 || count > STAR_COUNT_MAX)
			TENLog("Star count must be in range [0, " + std::to_string(STAR_COUNT_MAX) + "].", LogLevel::Warning);

		_starCount = std::clamp(count, 0, STAR_COUNT_MAX);
	}

	/// Set this starfield's number of meteors.
	// @function Starfield:SetMeteorCount
	// @tparam int count New meteor count.
	void Starfield::SetMeteorCount(int count)
	{
		if (count < 0 || count > METEOR_COUNT_MAX)
			TENLog("Meteor count must be in range [0, " + std::to_string(METEOR_COUNT_MAX) + "].", LogLevel::Warning);

		_meteorCount = std::clamp(count, 0, METEOR_COUNT_MAX);
	}

	/// Set this starfield's meteor spawn density.
	// @function Starfield:SetMeteorSpawnDensity
	// @tparam int density New meteor spawn density.
	void Starfield::SetMeteorSpawnDensity(int spawnDensity)
	{
		_meteorSpawnDensity = spawnDensity;
	}

	/// Set this starfield's meteor velocity.
	// @function Starfield:SetMeteorVelocity
	// @tparam float velocity New meteor velocity.
	void Starfield::SetMeteorVelocity(float vel)
	{
		_meteorVelocity = vel;
	}
}
