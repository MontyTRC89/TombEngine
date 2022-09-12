#pragma once

namespace TEN::Math
{
	struct Vector2i
	{
		// Components
		int x = 0;
		int y = 0;

		// Constants
		static const Vector2i Zero;

		// Constructors
		Vector2i();
		Vector2i(int x, int y);
		Vector2i(const Vector2& vector);

		// Utilities
		static float Distance(const Vector2i& origin, const Vector2i& target);
		static float DistanceSquared(const Vector2i& origin, const Vector2i& target);

		// Converters
		Vector2 ToVector2() const;

		// Operators
		bool	  operator ==(const Vector2i& vector);
		bool	  operator !=(const Vector2i& vector);
		Vector2i  operator +(const Vector2i& vector);
		Vector2i  operator -(const Vector2i& vector);
		Vector2i  operator *(const Vector2i& vector);
		Vector2i  operator *(float value);
		Vector2i  operator /(float value);
		Vector2i& operator =(const Vector2i& vector);
		Vector2i& operator +=(const Vector2i& vector);
		Vector2i& operator -=(const Vector2i& vector);
		Vector2i& operator *=(const Vector2i& vector);
		Vector2i& operator *=(float value);
		Vector2i& operator /=(float value);
	};
}
