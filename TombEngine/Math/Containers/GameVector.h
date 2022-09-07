#pragma once

struct Vector3i;

//namespace TEN::Math
//{
	struct GameVector
	{
		int x = 0;
		int y = 0;
		int z = 0;
		short roomNumber = 0;
		int boxNumber = 0;

		static const GameVector Zero;

		GameVector();
		GameVector(const Vector3i& pos);
		GameVector(const Vector3i& pos, short roomNumber);
		GameVector(const Vector3i& pos, short roomNumber, short boxNumber);
		GameVector(int xPos, int yPos, int zPos);
		GameVector(int xPos, int yPos, int zPos, short roomNumber);
		GameVector(int xPos, int yPos, int zPos, short roomNumber, short boxNumber);

		Vector3i ToVector3i();
	};
//}
