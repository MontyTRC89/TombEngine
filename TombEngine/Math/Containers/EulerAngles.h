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

		// Utilities
		static bool		   Compare(const EulerAngles& eulers0, const EulerAngles& eulers1, short epsilon = 2);
		void			   Lerp(const EulerAngles& eulersTo, float alpha = 1.0f, short epsilon = 2);
		static EulerAngles Lerp(const EulerAngles& eulersFrom, const EulerAngles& eulersTo, float alpha = 1.0f, short epsilon = 2);
		void			   InterpolateConstant(const EulerAngles& eulersTo, short angularVel);
		static EulerAngles InterpolateConstant(const EulerAngles& eulersFrom, const EulerAngles& eulerTo, short angularVel);

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
	
	private:
		// Temporary. Will be integrated into eventual Angle class.
		static float ClampAlpha(float alpha);
		static bool	 Compare(short angle0, short angle1, short epsilon = 2);
		static short Lerp(short angleFrom, short angleTo, float alpha = 1.0f, short epsilon = 2);
		static short InterpolateConstant(short angleFrom, short angleTo, short angularVel);
	};
//}
