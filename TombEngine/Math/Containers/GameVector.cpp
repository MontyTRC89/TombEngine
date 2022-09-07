#include "framework.h"
#include "Math/Containers/GameVector.h"

#include "Math/Containers/Vector3i.h"

//namespace TEN::Math
//{
	GameVector const GameVector::Zero = GameVector(0, 0, 0, 0, 0);

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
		this->roomNumber = roomNumber;
	}
	
	GameVector::GameVector(const Vector3i& pos, short roomNumber, short boxNumber)
	{
		this->x = pos.x;
		this->y = pos.y;
		this->z = pos.z;
		this->roomNumber = roomNumber;
		this->boxNumber = boxNumber;
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
		this->roomNumber = roomNumber;
	}

	GameVector::GameVector(int xPos, int yPos, int zPos, short roomNumber, short boxNumber)
	{
		this->x = xPos;
		this->y = yPos;
		this->z = zPos;
		this->roomNumber = roomNumber;
		this->boxNumber = boxNumber;
	}

	Vector3i GameVector::ToVector3i()
	{
		return Vector3i(x, y, z);
	}
//}
