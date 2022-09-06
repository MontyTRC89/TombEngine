#pragma once

//namespace TEN::Math
//{
	// TODO: Possibly rename to the more common standard: Vector3i
	struct Vector3Shrt
	{
		short x = 0;
		short y = 0;
		short z = 0;

		static const Vector3Shrt Zero;

		Vector3Shrt();
		Vector3Shrt(short x, short y, short z);

		bool		 operator ==(const Vector3Shrt& vector);
		bool		 operator !=(const Vector3Shrt& vector);
		Vector3Shrt	 operator +(const Vector3Shrt& vector);
		Vector3Shrt	 operator -(const Vector3Shrt& vector);
		Vector3Shrt	 operator *(const Vector3Shrt& vector);
		Vector3Shrt	 operator *(float value);
		Vector3Shrt	 operator /(float value);
		Vector3Shrt& operator =(const Vector3Shrt& vector);
		Vector3Shrt& operator +=(const Vector3Shrt& vector);
		Vector3Shrt& operator -=(const Vector3Shrt& vector);
		Vector3Shrt& operator *=(const Vector3Shrt& vector);
		Vector3Shrt& operator *=(float value);
		Vector3Shrt& operator /=(float value);
	};
//}
