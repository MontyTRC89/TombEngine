#pragma once

struct Vector3Int;

//namespace TEN::Math
//{
	struct GameVector
	{
		int x = 0;
		int y = 0;
		int z = 0;
		short roomNumber = 0;
		int boxNumber = 0;

		static const GameVector Empty;

		GameVector();
		GameVector(Vector3Int pos);
		GameVector(Vector3Int pos, short roomNumber);
		GameVector(Vector3Int pos, short roomNumber, short boxNumber);
		GameVector(int xPos, int yPos, int zPos);
		GameVector(int xPos, int yPos, int zPos, short roomNumber);
		GameVector(int xPos, int yPos, int zPos, short roomNumber, short boxNumber);

		Vector3Int ToVector3Int();
	};
//}
