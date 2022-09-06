#pragma once

//namespace TEN::Math
//{
	// TODO: Possibly rename to the more common standard: Vector3i
	struct Vector3Int
	{
		int x = 0;
		int y = 0;
		int z = 0;

		static const Vector3Int Zero;

		Vector3Int();
		Vector3Int(int x, int y, int z);
		Vector3Int(const Vector3& vector);

		static float Distance(const Vector3Int& origin, const Vector3Int& target);

		Vector3 ToVector3() const;

		bool		operator ==(const Vector3Int& vector);
		bool		operator !=(const Vector3Int& vector);
		Vector3Int	operator +(const Vector3Int& vector);
		Vector3Int	operator -(const Vector3Int& vector);
		Vector3Int	operator *(const Vector3Int& vector);
		Vector3Int	operator *(float value);
		Vector3Int	operator /(float value);
		Vector3Int& operator =(const Vector3Int& vector);
		Vector3Int& operator +=(const Vector3Int& vector);
		Vector3Int& operator -=(const Vector3Int& vector);
		Vector3Int& operator *=(const Vector3Int& vector);
		Vector3Int& operator *=(float value);
		Vector3Int& operator /=(float value);
	};
//}
