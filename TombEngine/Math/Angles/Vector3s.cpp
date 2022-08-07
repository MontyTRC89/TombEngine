#include "framework.h"
#include "Math/Angles/Vector3s.h"

//namespace TEN::Math
//{
	Vector3Shrt const Vector3Shrt::Zero = Vector3Shrt(0, 0, 0);

	Vector3Shrt::Vector3Shrt()
	{
		*this = Vector3Shrt::Zero;
	}

	Vector3Shrt::Vector3Shrt(short x, short y, short z)
	{
		this->x = x;
		this->y = y;
		this->z = z;
	}

	bool Vector3Shrt::operator ==(Vector3Shrt vector)
	{
		return (x == vector.x && y == vector.y && z == vector.z);
	}

	bool Vector3Shrt::operator !=(Vector3Shrt vector)
	{
		return (x != vector.x || y != vector.y || z != vector.z);
	}

	Vector3Shrt Vector3Shrt::operator +(Vector3Shrt vector)
	{
		return Vector3Shrt(x + vector.x, y + vector.y, z + vector.z);
	}

	Vector3Shrt Vector3Shrt::operator -(Vector3Shrt vector)
	{
		return Vector3Shrt(x - vector.x, y - vector.y, z - vector.z);
	}

	Vector3Shrt Vector3Shrt::operator *(Vector3Shrt vector)
	{
		return Vector3Shrt(x * vector.x, y * vector.y, z * vector.z);
	}

	Vector3Shrt Vector3Shrt::operator *(float value)
	{
		return Vector3Shrt((short)round(x * value), (short)round(y * value), (short)round(z * value));
	}

	Vector3Shrt Vector3Shrt::operator /(float value)
	{
		return Vector3Shrt((short)round(x / value), (short)round(y / value), (short)round(z / value));
	}

	Vector3Shrt& Vector3Shrt::operator =(Vector3Shrt vector)
	{
		this->x = vector.x;
		this->y = vector.y;
		this->z = vector.z;
		return *this;
	}

	Vector3Shrt& Vector3Shrt::operator +=(Vector3Shrt vector)
	{
		this->x += vector.x;
		this->y += vector.y;
		this->z += vector.z;
		return *this;
	}

	Vector3Shrt& Vector3Shrt::operator -=(Vector3Shrt vector)
	{
		this->x -= vector.x;
		this->y -= vector.y;
		this->z -= vector.z;
		return *this;
	}

	Vector3Shrt& Vector3Shrt::operator *=(Vector3Shrt vector)
	{
		this->x *= vector.x;
		this->y *= vector.y;
		this->z *= vector.z;
		return *this;
	}

	Vector3Shrt& Vector3Shrt::operator *=(float value)
	{
		this->x *= value;
		this->y *= value;
		this->z *= value;
		return *this;
	}

	Vector3Shrt& Vector3Shrt::operator /=(float value)
	{
		this->x /= value;
		this->y /= value;
		this->z /= value;
		return *this;
	}
//}
