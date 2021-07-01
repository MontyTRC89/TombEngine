#include "framework.h"
#include "GameScriptPosition.h"
#include <sol.hpp>

void GameScriptPosition::Register(sol::state* state)
{
	state->new_usertype<GameScriptPosition>("Position",
		sol::constructors<GameScriptPosition(int, int, int)>(),
		"X", sol::property(&GameScriptPosition::GetX, &GameScriptPosition::SetX),
		"Y", sol::property(&GameScriptPosition::GetY, &GameScriptPosition::SetY),
		"Z", sol::property(&GameScriptPosition::GetZ, &GameScriptPosition::SetZ)
		);
}

GameScriptPosition::GameScriptPosition(int x, int y, int z)
{
	SetX(x);
	SetY(y);
	SetZ(z);
}

int GameScriptPosition::GetX() const
{
	return x;
}

void GameScriptPosition::SetX(int x)
{
	if (x < INT_MIN || x > INT_MAX)
		return;
	else
		this->x = x;
}

int GameScriptPosition::GetY() const
{
	return y;
}

void GameScriptPosition::SetY(int y)
{
	if (y < INT_MIN || y > INT_MAX)
		return;
	else
		this->y = y;
}

int GameScriptPosition::GetZ() const
{
	return z;
}

void GameScriptPosition::SetZ(int z)
{
	if (z < INT_MIN || z > INT_MAX)
		return;
	else
		this->z = z;
}