#include "framework.h"
#include "Math/Containers/Vector2i.h"

namespace TEN::Math
{
	Vector2i const Vector2i::Zero = Vector2i(0, 0);

	Vector2i::Vector2i()
	{
	}

	Vector2i::Vector2i(int x, int y)
	{
		this->x = x;
		this->y = y;
	}

	Vector2i::Vector2i(const Vector2& vector)
	{
		this->x = (int)round(vector.x);
		this->y = (int)round(vector.y);
	}

	float Vector2i::Distance(const Vector2i& origin, const Vector2i& target)
	{
		return Vector2::Distance(origin.ToVector2(), target.ToVector2());
	}

	Vector2 Vector2i::ToVector2() const
	{
		return Vector2(x, y);
	}

	bool Vector2i::operator ==(const Vector2i& vector)
	{
		return (x == vector.x && y == vector.y);
	}

	bool Vector2i::operator !=(const Vector2i& vector)
	{
		return (x != vector.x || y != vector.y);
	}

	Vector2i Vector2i::operator +(const Vector2i& vector)
	{
		return Vector2i(x + vector.x, y + vector.y);
	}

	Vector2i Vector2i::operator -(const Vector2i& vector)
	{
		return Vector2i(x - vector.x, y - vector.y);
	}

	Vector2i Vector2i::operator *(const Vector2i& vector)
	{
		return Vector2i(x * vector.x, y * vector.y);
	}

	Vector2i Vector2i::operator *(float value)
	{
		return Vector2i((int)round(x * value), (int)round(y * value));
	}

	Vector2i Vector2i::operator /(float value)
	{
		return Vector2i((int)round(x / value), (int)round(y / value));
	}

	Vector2i& Vector2i::operator =(const Vector2i& vector)
	{
		this->x = vector.x;
		this->y = vector.y;
		return *this;
	}

	Vector2i& Vector2i::operator +=(const Vector2i& vector)
	{
		this->x += vector.x;
		this->y += vector.y;
		return *this;
	}

	Vector2i& Vector2i::operator -=(const Vector2i& vector)
	{
		this->x -= vector.x;
		this->y -= vector.y;
		return *this;
	}

	Vector2i& Vector2i::operator *=(const Vector2i& vector)
	{
		this->x *= vector.x;
		this->y *= vector.y;
		return *this;
	}

	Vector2i& Vector2i::operator *=(float value)
	{
		this->x *= value;
		this->y *= value;
		return *this;
	}

	Vector2i& Vector2i::operator /=(float value)
	{
		this->x /= value;
		this->y /= value;
		return *this;
	}
}
