#include "framework.h"
#include "Math/Containers/GameVector.h"

#include "Math/Containers/Vector3i.h"

//namespace TEN::Math
//{
	const GameVector GameVector::Zero = GameVector(0, 0, 0, 0);

	GameVector::GameVector()
	{
	}

	GameVector::GameVector(const Vector3i& pos)
	{
		this->x = pos.x;
		this->y = pos.y;
		this->z = pos.z;
	}
	
	GameVector::GameVector(const Vector3i& pos, short roomNumber)
	{
		this->x = pos.x;
		this->y = pos.y;
		this->z = pos.z;
		this->RoomNumber = roomNumber;
	}
	
	GameVector::GameVector(int xPos, int yPos, int zPos)
	{
		this->x = xPos;
		this->y = yPos;
		this->z = zPos;
	}

	GameVector::GameVector(int xPos, int yPos, int zPos, short roomNumber)
	{
		this->x = xPos;
		this->y = yPos;
		this->z = zPos;
		this->RoomNumber = roomNumber;
	}

	Vector3 GameVector::ToVector3() const
	{
		return Vector3(x, y, z);
	}

	Vector3i GameVector::ToVector3i() const
	{
		return Vector3i(x, y, z);
	}

	bool GameVector::operator ==(const GameVector& vector) const
	{
		return ((x == vector.x) && (y == vector.y) && (z == vector.z));
	}

	bool GameVector::operator !=(const GameVector& vector) const
	{
		return ((x != vector.x) || (y != vector.y) || (z != vector.z));
	}

	GameVector& GameVector::operator =(const GameVector& vector)
	{
		this->x = vector.x;
		this->y = vector.y;
		this->z = vector.z;
		return *this;
	}

	GameVector& GameVector::operator +=(const GameVector& vector)
	{
		this->x += vector.x;
		this->y += vector.y;
		this->z += vector.z;
		return *this;
	}

	GameVector& GameVector::operator -=(const GameVector& vector)
	{
		this->x -= vector.x;
		this->y -= vector.y;
		this->z -= vector.z;
		return *this;
	}

	GameVector& GameVector::operator *=(const GameVector& vector)
	{
		this->x *= vector.x;
		this->y *= vector.y;
		this->z *= vector.z;
		return *this;
	}

	GameVector& GameVector::operator *=(float scale)
	{
		this->x *= scale;
		this->y *= scale;
		this->z *= scale;
		return *this;
	}

	GameVector& GameVector::operator /=(float scale)
	{
		this->x /= scale;
		this->y /= scale;
		this->z /= scale;
		return *this;
	}

	GameVector GameVector::operator +(const GameVector& vector) const
	{
		return GameVector(x + vector.x, y + vector.y, z + vector.z);
	}

	GameVector GameVector::operator -(const GameVector& vector) const
	{
		return GameVector(x - vector.x, y - vector.y, z - vector.z);
	}

	GameVector GameVector::operator *(const GameVector& vector) const
	{
		return GameVector(x * vector.x, y * vector.y, z * vector.z);
	}

	GameVector GameVector::operator *(float scale) const
	{
		return GameVector((int)round(x * scale), (int)round(y * scale), (int)round(z * scale));
	}

	GameVector GameVector::operator /(float scale) const
	{
		return GameVector((int)round(x / scale), (int)round(y / scale), (int)round(z / scale));
	}
//}
