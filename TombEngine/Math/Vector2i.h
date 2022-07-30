#pragma once

struct Vector2Int
{
	int x = 0;
	int y = 0;

	static const Vector2Int Zero;

	Vector2Int();
	Vector2Int(int x, int y);

	Vector2 ToVector2();
};
