#include "framework.h"
#include "Starfield.h"
#include "Specific/level.h"
#include "Game/effects/weather.h"

using namespace TEN::Effects::Environment;

/***
Starfield

@tenclass Flow.Starfield
@pragma nostrip
*/

void Starfield::Register(sol::table& parent)
{
	using ctors = sol::constructors<Starfield(int, int, int, int), Starfield(int)>;
	parent.new_usertype<Starfield>("Starfield",
		ctors(),
		sol::call_constructor, ctors(),

		/*** (int) Stars count.

		Values can be between [0, 6000], 0 resulting in no stars being rendered, and 6000 resulting in the maximum number of stars being rendered.

		@mem starsCount*/
		"starsCount", sol::property(&Starfield::GetStarsCount, &Starfield::SetStarsCount),

		/*** (int) Meteors count.

		Values can be between [0, 100], 0 resulting in no meteors being rendered, and 100 resulting in the maximum number of meteors being rendered.

		@mem meteorsCount*/
		"meteorsCount", sol::property(&Starfield::GetMeteorsCount, &Starfield::SetMeteorsCount),

		/*** (int) Meteors spawn density.

		@mem meteorsSpawnDensity*/
		"meteorsSpawnDensity", sol::property(&Starfield::GetMeteorsSpawnDensity, &Starfield::SetMeteorsSpawnDensity),

		/*** (float) Meteors speed.

		@mem meteorsSpeed*/
		"meteorsSpeed", sol::property(&Starfield::GetMeteorsSpeed, &Starfield::SetMeteorsSpeed)
	);
}

/***
@tparam int starsCount Stars count
@treturn Starfield A starfield object with only stars enabled.
@function Starfield
*/
Starfield::Starfield(int starsCount)
{
	SetStarsCount(starsCount);
	SetMeteorsCount(0);
}

/***
@tparam int starsCount Stars count
@tparam int meteorsCount Stars count
@treturn Starfield A starfield object with boths stars and meteors enabled.
@function Starfield
*/
Starfield::Starfield(int starsCount, int meteorsCount, int meteorsSpawnDensity, int meteorsSpawnSpeed)
{
	SetStarsCount(starsCount);
	SetMeteorsCount(meteorsCount);
	SetMeteorsSpawnDensity(meteorsSpawnDensity);
	SetMeteorsSpeed(meteorsSpawnSpeed);
}

void Starfield::SetStarsCount(int const& starsCount)
{
	assertion(starsCount >= 0 && starsCount <= 6000, "Stars count must be in the range 0 ... 6000");
	StarsCount = starsCount;
}


int Starfield::GetStarsCount() const
{
	return StarsCount;
}

void Starfield::SetMeteorsCount(int const& meteorsCount)
{
	assertion(meteorsCount >= 0 && meteorsCount <= 100, "Stars count must be in the range 0 ... 100");
	MeteorsCount = meteorsCount;
}


int Starfield::GetMeteorsCount() const
{
	return MeteorsCount;
}

void Starfield::SetMeteorsSpawnDensity(int const& meteorsSpawnDensity)
{
	MeteorsSpawnDensity = meteorsSpawnDensity;
}


int Starfield::GetMeteorsSpawnDensity() const
{
	return MeteorsSpawnDensity;
}

void Starfield::SetMeteorsSpeed(float const& meteorsSpeed)
{
	MeteorsSpeed = meteorsSpeed;
}


float Starfield::GetMeteorsSpeed() const
{
	return MeteorsSpeed;
}

bool Starfield::GetEnabled() const
{
	return (StarsCount > 0);
}

bool Starfield::GetMeteorsEnabled() const
{
	return (MeteorsCount > 0);
}