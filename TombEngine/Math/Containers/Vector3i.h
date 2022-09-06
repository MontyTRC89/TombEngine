#pragma once

//namespace TEN::Math
//{
	struct Vector3i
	{
		int x = 0;
		int y = 0;
		int z = 0;

		static const Vector3i Zero;

		Vector3i();
		Vector3i(int x, int y, int z);
		Vector3i(const Vector3& vector);

		static float Distance(const Vector3i& origin, const Vector3i& target);

		Vector3 ToVector3() const;

		bool		operator ==(const Vector3i& vector);
		bool		operator !=(const Vector3i& vector);
		Vector3i	operator +(const Vector3i& vector);
		Vector3i	operator -(const Vector3i& vector);
		Vector3i	operator *(const Vector3i& vector);
		Vector3i	operator *(float value);
		Vector3i	operator /(float value);
		Vector3i& operator =(const Vector3i& vector);
		Vector3i& operator +=(const Vector3i& vector);
		Vector3i& operator -=(const Vector3i& vector);
		Vector3i& operator *=(const Vector3i& vector);
		Vector3i& operator *=(float value);
		Vector3i& operator /=(float value);
	};
//}
