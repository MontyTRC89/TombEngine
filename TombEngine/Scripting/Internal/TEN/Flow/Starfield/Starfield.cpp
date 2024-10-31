#include "Scripting/Internal/TEN/Flow/Starfield/Starfield.h"

#include "Game/effects/weather.h"
#include "Specific/level.h"

using namespace TEN::Effects::Environment;

/// Represents a starfield.
//
// @tenclass Flow.Starfield
// @pragma nostrip

namespace TEN::Scripting
{
	void Starfield::Register(sol::table& parent)
	{
		using ctors = sol::constructors<
			Starfield(int starCount),
			Starfield(int starCount, int meteorCount, int meteorSpawnDensity, float meteorVel)>;
		
		// Register type.
		parent.new_usertype<Starfield>(
			"Starfield", ctors(), sol::call_constructor, ctors(),
			"GetStarsEnabled", &Starfield::GetStarsEnabled,
			"GetMeteorsEnabled", &Starfield::GetMeteorsEnabled,
			"GetStarCount", &Starfield::GetStarCount,
			"GetMeteorCount", &Starfield::GetMeteorCount,
			"GetMeteorSpawnDensity", &Starfield::GetMeteorSpawnDensity,
			"GetMeteorVelocity", &Starfield::GetMeteorVelocity,
			"SetStarCount", &Starfield::SetStarCount,
			"SetMeteorCount", &Starfield::SetMeteorCount,
			"SetMeteorSpawnDensity", &Starfield::SetMeteorSpawnDensity,
			"SetMeteorVelocity", &Starfield::SetMeteorVelocity);
	}

	/// Create a starfield object with only stars.
	// @function Starfield()
	// @tparam int starCount Star count.
	// @treturn Starfield A new Starfield object.
	Starfield::Starfield(int starCount)
	{
		_starCount = starCount;
	}

	/// Create a starfield object with stars and meteors.
	// @function Starfield()
	// @tparam int starCount Star count (6000 max).
	// @tparam int meteorCount Meteor count (100 max).
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

	/// Get the starfield's enabled status of stars.
	// @function Starfield:GetStarsEnabled()
	// @treturn bool Stars enabled status.
	bool Starfield::GetStarsEnabled() const
	{
		return (_starCount > 0);
	}

	/// Get the starfield's enabled status of meteors.
	// @function Starfield:GetMeteorsEnabled()
	// @treturn bool Meteors enabled status.
	bool Starfield::GetMeteorsEnabled() const
	{
		return (_meteorCount > 0);
	}

	/// Get the starfield's number of stars.
	// @function Starfield:GetStarCount()
	// @treturn int Count.
	int Starfield::GetStarCount() const
	{
		return _starCount;
	}

	/// Get the starfield's number of meteors.
	// @function Starfield:GetMeteorCount()
	// @treturn int Count.
	int Starfield::GetMeteorCount() const
	{
		return _meteorCount;
	}

	/// Get the starfield's meteor spawn density.
	// @function Starfield:GetMeteorSpawnDensity()
	// @treturn int Spawn density.
	int Starfield::GetMeteorSpawnDensity() const
	{
		return _meteorSpawnDensity;
	}

	/// Get the starfield's meteor velocity.
	// @function Starfield:GetMeteorVelocity()
	// @treturn float Velocity.
	float Starfield::GetMeteorVelocity() const
	{
		return _meteorVelocity;
	}

	/// Set the starfield's number of stars (6000 max).
	// @function Starfield:SetStarCount(int)
	// @tparam int New count.
	void Starfield::SetStarCount(int count)
	{
		if (count < 0 || count > STAR_COUNT_MAX)
			TENLog("Star count must be in range [0, " + std::to_string(STAR_COUNT_MAX) + "].", LogLevel::Warning);

		_starCount = std::clamp(count, 0, STAR_COUNT_MAX);
	}

	/// Set the starfield's number of meteors (100 max).
	// @function Starfield:SetMeteorCount(int)
	// @tparam int New count.
	void Starfield::SetMeteorCount(int count)
	{
		if (count < 0 || count > METEOR_COUNT_MAX)
			TENLog("Meteor count must be in range [0, " + std::to_string(METEOR_COUNT_MAX) + "].", LogLevel::Warning);

		_meteorCount = std::clamp(count, 0, METEOR_COUNT_MAX);
	}

	/// Set the starfield's meteor spawn density.
	// @function Starfield:SetMeteorSpawnDensity(int)
	// @tparam int New spawn density.
	void Starfield::SetMeteorSpawnDensity(int spawnDensity)
	{
		_meteorSpawnDensity = spawnDensity;
	}

	/// Set the starfield's meteor velocity.
	// @function Starfield:SetMeteorVelocity(float)
	// @tparam int New velocity.
	void Starfield::SetMeteorVelocity(float vel)
	{
		_meteorVelocity = vel;
	}
}
