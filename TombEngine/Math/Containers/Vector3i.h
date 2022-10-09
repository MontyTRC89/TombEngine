#pragma once

//namespace TEN::Math
//{
	struct Vector3i
	{
		// Components
		int x = 0;
		int y = 0;
		int z = 0;

		// Constants
		static const Vector3i Zero;

		// Constructors
		Vector3i();
		Vector3i(int x, int y, int z);
		Vector3i(const Vector3& vector);

		// Utilities
		static float Distance(const Vector3i& origin, const Vector3i& target);
		static float DistanceSquared(const Vector3i& origin, const Vector3i& target);

		// Converters
		Vector3 ToVector3() const;

		// Operators
		bool	  operator ==(const Vector3i& vector) const;
		bool	  operator !=(const Vector3i& vector) const;
		Vector3i& operator =(const Vector3i& vector);
		Vector3i& operator +=(const Vector3i& vector);
		Vector3i& operator -=(const Vector3i& vector);
		Vector3i& operator *=(const Vector3i& vector);
		Vector3i& operator *=(float scale);
		Vector3i& operator /=(float scale);
		Vector3i  operator +(const Vector3i& vector) const;
		Vector3i  operator -(const Vector3i& vector) const;
		Vector3i  operator *(const Vector3i& vector) const;
		Vector3i  operator *(float scale) const;
		Vector3i  operator /(float scale) const;
	};
//}
