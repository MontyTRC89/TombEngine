#include "framework.h"
#include "GameScriptPosition.h"

GameScriptPosition::GameScriptPosition(int x, int y, int z)
{
	SetX(x);
	SetY(y);
	SetZ(z);
}

int GameScriptPosition::GetX()
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

int GameScriptPosition::GetY()
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

int GameScriptPosition::GetZ()
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