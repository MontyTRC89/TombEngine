#include "framework.h"
#include "Math/Containers/Vector2i.h"

//namespace TEN::Math
//{
	Vector2Int const Vector2Int::Zero = Vector2Int(0, 0);

	Vector2Int::Vector2Int()
	{
	}

	Vector2Int::Vector2Int(int x, int y)
	{
		this->x = x;
		this->y = y;
	}

	Vector2Int::Vector2Int(Vector2 vector)
	{
		this->x = (int)round(vector.x);
		this->y = (int)round(vector.y);
	}

	float Vector2Int::Distance(Vector2Int origin, Vector2Int target)
	{
		return Vector2::Distance(origin.ToVector2(), target.ToVector2());
	}

	Vector2 Vector2Int::ToVector2() const
	{
		return Vector2(x, y);
	}

	bool Vector2Int::operator ==(Vector2Int vector)
	{
		return (x == vector.x && y == vector.y);
	}

	bool Vector2Int::operator !=(Vector2Int vector)
	{
		return (x != vector.x || y != vector.y);
	}

	Vector2Int Vector2Int::operator +(Vector2Int vector)
	{
		return Vector2Int(x + vector.x, y + vector.y);
	}

	Vector2Int Vector2Int::operator -(Vector2Int vector)
	{
		return Vector2Int(x - vector.x, y - vector.y);
	}

	Vector2Int Vector2Int::operator *(Vector2Int vector)
	{
		return Vector2Int(x * vector.x, y * vector.y);
	}

	Vector2Int Vector2Int::operator *(float value)
	{
		return Vector2Int((int)round(x * value), (int)round(y * value));
	}

	Vector2Int Vector2Int::operator /(float value)
	{
		return Vector2Int((int)round(x / value), (int)round(y / value));
	}

	Vector2Int& Vector2Int::operator =(Vector2Int vector)
	{
		this->x = vector.x;
		this->y = vector.y;
		return *this;
	}

	Vector2Int& Vector2Int::operator +=(Vector2Int vector)
	{
		this->x += vector.x;
		this->y += vector.y;
		return *this;
	}

	Vector2Int& Vector2Int::operator -=(Vector2Int vector)
	{
		this->x -= vector.x;
		this->y -= vector.y;
		return *this;
	}

	Vector2Int& Vector2Int::operator *=(Vector2Int vector)
	{
		this->x *= vector.x;
		this->y *= vector.y;
		return *this;
	}

	Vector2Int& Vector2Int::operator *=(float value)
	{
		this->x *= value;
		this->y *= value;
		return *this;
	}

	Vector2Int& Vector2Int::operator /=(float value)
	{
		this->x /= value;
		this->y /= value;
		return *this;
	}
//}
