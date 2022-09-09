#pragma once

//namespace TEN::Math
//{
	class EulerAngles
	{
	public:
		// CONVENTION: X = pitch, Y = yaw, Z = roll
		short x = 0;
		short y = 0;
		short z = 0;

		static const EulerAngles Zero;

		EulerAngles();
		EulerAngles(short x, short y, short z);

		bool		 operator ==(const EulerAngles& eulers);
		bool		 operator !=(const EulerAngles& eulers);
		EulerAngles	 operator +(const EulerAngles& eulers);
		EulerAngles	 operator -(const EulerAngles& eulers);
		EulerAngles	 operator *(const EulerAngles& eulers);
		EulerAngles	 operator *(float value);
		EulerAngles	 operator /(float value);
		EulerAngles& operator =(const EulerAngles& eulers);
		EulerAngles& operator +=(const EulerAngles& eulers);
		EulerAngles& operator -=(const EulerAngles& eulers);
		EulerAngles& operator *=(const EulerAngles& eulers);
		EulerAngles& operator *=(float value);
		EulerAngles& operator /=(float value);
	};
//}
