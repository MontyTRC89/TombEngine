#pragma once

//namespace TEN::Math
//{
	class Vector3i
	{
	public:
		// Members
		int x = 0;
		int y = 0;
		int z = 0;

		// Constants
		static const Vector3i Zero;

		// Constructors
		constexpr Vector3i() {};
		constexpr Vector3i(int x, int y, int z) { this->x = x; this->y = y; this->z = z; };
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
		Vector3i& operator *=(float scalar);
		Vector3i& operator /=(float scalar);
		Vector3i  operator +(const Vector3i& vector) const;
		Vector3i  operator -(const Vector3i& vector) const;
		Vector3i  operator *(const Vector3i& vector) const;
		Vector3i  operator *(float scalar) const;
		Vector3i  operator /(float scalar) const;
	};
//}
