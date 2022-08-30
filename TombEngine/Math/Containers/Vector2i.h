#pragma once

//namespace TEN::Math
//{
	// TODO: Possibly rename to the more common standard: Vector2i
	struct Vector2Int
	{
		int x = 0;
		int y = 0;

		static const Vector2Int Zero;

		Vector2Int();
		Vector2Int(int x, int y);
		Vector2Int(Vector2 vector);

		static float Distance(Vector2Int origin, Vector2Int target);

		Vector2 ToVector2() const;

		bool		operator ==(Vector2Int vector);
		bool		operator !=(Vector2Int vector);
		Vector2Int	operator +(Vector2Int vector);
		Vector2Int	operator -(Vector2Int vector);
		Vector2Int	operator *(Vector2Int vector);
		Vector2Int	operator *(float value);
		Vector2Int	operator /(float value);
		Vector2Int& operator =(Vector2Int vector);
		Vector2Int& operator +=(Vector2Int vector);
		Vector2Int& operator -=(Vector2Int vector);
		Vector2Int& operator *=(Vector2Int vector);
		Vector2Int& operator *=(float value);
		Vector2Int& operator /=(float value);
	};
//}
