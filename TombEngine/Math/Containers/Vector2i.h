#pragma once

namespace TEN::Math
{
	struct Vector2i
	{
		int x = 0;
		int y = 0;

		static const Vector2i Zero;

		Vector2i();
		Vector2i(int x, int y);
		Vector2i(const Vector2& vector);

		static float Distance(const Vector2i& origin, const Vector2i& target);

		Vector2 ToVector2() const;

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
