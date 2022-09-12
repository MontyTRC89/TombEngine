#pragma once

struct Vector3i;

//namespace TEN::Math
//{
	struct GameVector
	{
		// Components
		int	  x			 = 0;
		int	  y			 = 0;
		int	  z			 = 0;
		short roomNumber = 0;
		int	  boxNumber	 = 0;

		// Constants
		static const GameVector Zero;

		// Constructors
		GameVector();
		GameVector(const Vector3i& pos);
		GameVector(const Vector3i& pos, short roomNumber);
		GameVector(const Vector3i& pos, short roomNumber, short boxNumber);
		GameVector(int xPos, int yPos, int zPos);
		GameVector(int xPos, int yPos, int zPos, short roomNumber);
		GameVector(int xPos, int yPos, int zPos, short roomNumber, short boxNumber);

		// Converters
		Vector3	 ToVector3() const;
		Vector3i ToVector3i() const;
	};
//}
