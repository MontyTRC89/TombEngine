#include "framework.h"
#include "Math/Containers/Vector3i.h"

//namespace TEN::Math
//{
	Vector3i const Vector3i::Zero = Vector3i(0, 0, 0);

	Vector3i::Vector3i()
	{
	}

	Vector3i::Vector3i(int x, int y, int z)
	{
		this->x = x;
		this->y = y;
		this->z = z;
	}

	Vector3i::Vector3i(const Vector3& vector)
	{
		this->x = (int)round(vector.x);
		this->y = (int)round(vector.y);
		this->z = (int)round(vector.z);
	}

	float Vector3i::Distance(const Vector3i& origin, const Vector3i& target)
	{
		return Vector3::Distance(origin.ToVector3(), target.ToVector3());
	}

	Vector3 Vector3i::ToVector3() const
	{
		return Vector3(x, y, z);
	}

	bool Vector3i::operator ==(const Vector3i& vector)
	{
		return (x == vector.x && y == vector.y && z == vector.z);
	}

	bool Vector3i::operator !=(const Vector3i& vector)
	{
		return (x != vector.x || y != vector.y || z != vector.z);
	}

	Vector3i Vector3i::operator +(const Vector3i& vector)
	{
		return Vector3i(x + vector.x, y + vector.y, z + vector.z);
	}

	Vector3i Vector3i::operator -(const Vector3i& vector)
	{
		return Vector3i(x - vector.x, y - vector.y, z - vector.z);
	}

	Vector3i Vector3i::operator *(const Vector3i& vector)
	{
		return Vector3i(x * vector.x, y * vector.y, z * vector.z);
	}

	Vector3i Vector3i::operator *(float value)
	{
		return Vector3i((int)round(x * value), (int)round(y * value), (int)round(z * value));
	}

	Vector3i Vector3i::operator /(float value)
	{
		return Vector3i((int)round(x / value), (int)round(y / value), (int)round(z / value));
	}

	Vector3i& Vector3i::operator =(const Vector3i& vector)
	{
		this->x = vector.x;
		this->y = vector.y;
		this->z = vector.z;
		return *this;
	}

	Vector3i& Vector3i::operator +=(const Vector3i& vector)
	{
		this->x += vector.x;
		this->y += vector.y;
		this->z += vector.z;
		return *this;
	}

	Vector3i& Vector3i::operator -=(const Vector3i& vector)
	{
		this->x -= vector.x;
		this->y -= vector.y;
		this->z -= vector.z;
		return *this;
	}

	Vector3i& Vector3i::operator *=(const Vector3i& vector)
	{
		this->x *= vector.x;
		this->y *= vector.y;
		this->z *= vector.z;
		return *this;
	}

	Vector3i& Vector3i::operator *=(float value)
	{
		this->x *= value;
		this->y *= value;
		this->z *= value;
		return *this;
	}

	Vector3i& Vector3i::operator /=(float value)
	{
		this->x /= value;
		this->y /= value;
		this->z /= value;
		return *this;
	}
//}
