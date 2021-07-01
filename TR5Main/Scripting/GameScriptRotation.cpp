#include "framework.h"
#include "GameScriptRotation.h"

void GameScriptRotation::Register(sol::state* state)
{
	state->new_usertype<GameScriptRotation>("Rotation",
		sol::constructors<GameScriptRotation(int, int, int)>(),
		"X", sol::property(&GameScriptRotation::GetX, &GameScriptRotation::SetX),
		"Y", sol::property(&GameScriptRotation::GetY, &GameScriptRotation::SetY),
		"Z", sol::property(&GameScriptRotation::GetZ, &GameScriptRotation::SetZ)
		);
}


GameScriptRotation::GameScriptRotation(int x, int y, int z)
{
	SetX(x);
	SetY(y);
	SetZ(z);
}

int GameScriptRotation::ConvertRotation(int a)
{
	short component = std::clamp(a, -359, 359);
	component = static_cast<int>(lround((component/360.0f) * std::numeric_limits<unsigned short>::max()));
	component = component - std::numeric_limits<short>::max();
	return component;
}

int GameScriptRotation::GetX() const
{
	return x;
}

void GameScriptRotation::SetX(int x)
{
	this->x = ConvertRotation(x);
}

int GameScriptRotation::GetY() const
{
	return y;
}

void GameScriptRotation::SetY(int y)
{
	this->y = ConvertRotation(y);
}

int GameScriptRotation::GetZ() const
{
	return z;
}

void GameScriptRotation::SetZ(int z)
{
	this->z = ConvertRotation(z);
}