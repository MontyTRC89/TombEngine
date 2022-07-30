#include "framework.h"
#include "Math/Vector2i.h"

Vector2Int const Vector2Int::Zero = Vector2Int(0, 0);

Vector2Int::Vector2Int()
{
	*this = Vector2Int::Zero;
	this->x = 0;
	this->y = 0;
}

Vector2Int::Vector2Int(int x, int y)
{
	this->x = x;
	this->y = y;
}

Vector2 Vector2Int::ToVector2()
{
	return Vector2(x, y);
}
