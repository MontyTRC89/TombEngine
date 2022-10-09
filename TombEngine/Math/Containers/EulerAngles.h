#pragma once

//namespace TEN::Math
//{
	class EulerAngles
	{
	public:
		// Components (CONVENTION: X = Pitch, Y = Yaw, Z = Roll)
		short x = 0;
		short y = 0;
		short z = 0;

		// Constants
		static const EulerAngles Zero;

		// Constructors
		EulerAngles();
		EulerAngles(short x, short y, short z);
		EulerAngles(const Vector3& direction);
		EulerAngles(const Quaternion& quat);
		EulerAngles(const Matrix& rotMatrix);

		// Converters
		Vector3	   ToDirection() const;
		Quaternion ToQuaternion() const;
		Matrix	   ToRotationMatrix() const;

		// Operators
		bool		 operator ==(const EulerAngles& eulers) const;
		bool		 operator !=(const EulerAngles& eulers) const;
		EulerAngles& operator =(const EulerAngles& eulers);
		EulerAngles& operator +=(const EulerAngles& eulers);
		EulerAngles& operator -=(const EulerAngles& eulers);
		EulerAngles& operator *=(const EulerAngles& eulers);
		EulerAngles& operator *=(float scale);
		EulerAngles& operator /=(float scale);
		EulerAngles	 operator +(const EulerAngles& eulers) const;
		EulerAngles	 operator -(const EulerAngles& eulers) const;
		EulerAngles	 operator *(const EulerAngles& eulers) const;
		EulerAngles	 operator *(float scale) const;
		EulerAngles	 operator /(float scale) const;
	};
//}
