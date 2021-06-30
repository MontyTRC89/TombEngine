#include "framework.h"
#include "GameScriptRotation.h"

GameScriptRotation::GameScriptRotation(int x, int y, int z)
{
	SetX(x);
	SetY(y);
	SetZ(z);
}

int GameScriptRotation::GetX() const
{
	return x;
}

void GameScriptRotation::SetX(int x)
{
	this->x = std::clamp(x, -360, 360);
}

int GameScriptRotation::GetY() const
{
	return y;
}

void GameScriptRotation::SetY(int y)
{
	this->y = std::clamp(y, -360, 360);
}

int GameScriptRotation::GetZ() const
{
	return z;
}

void GameScriptRotation::SetZ(int z)
{
	this->z = std::clamp(z, -360, 360);
}