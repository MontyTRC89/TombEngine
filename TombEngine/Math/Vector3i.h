#pragma once

//namespace TEN::Math
//{
	struct Vector3Int
	{
		int x = 0;
		int y = 0;
		int z = 0;

		static const Vector3Int Zero;

		Vector3Int();
		Vector3Int(int x, int y, int z);

		Vector3 ToVector3();

		bool		operator ==(Vector3Int vector);
		bool		operator !=(Vector3Int vector);
		Vector3Int	operator +(Vector3Int vector);
		Vector3Int	operator -(Vector3Int vector);
		Vector3Int	operator *(Vector3Int vector);
		Vector3Int	operator *(float value);
		Vector3Int	operator /(float value);
		Vector3Int&	operator =(Vector3Int vector);
		Vector3Int& operator +=(Vector3Int vector);
		Vector3Int& operator -=(Vector3Int vector);
		Vector3Int& operator *=(Vector3Int vector);
		Vector3Int& operator *=(float value);
		Vector3Int& operator /=(float value);
	};
//}
