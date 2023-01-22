#pragma once

class EulerAngles;

//namespace TEN::Math
//{
	class AxisAngle
	{
	private:
		// Components
		Vector3 Axis  = Vector3::Zero;
		short	Angle = 0;

	public:
		// Constants
		static const AxisAngle Identity;

		// Constructors
		AxisAngle();
		AxisAngle(const Vector3& axis, short angle);
		AxisAngle(const Vector3& direction);
		AxisAngle(const EulerAngles& eulers);
		AxisAngle(const Quaternion& quat);
		AxisAngle(const Matrix& rotMatrix);

		// Getters
		Vector3 GetAxis() const;
		short	GetAngle() const;

		// Setters
		void SetAxis(const Vector3& axis);
		void SetAngle(short angle);

		// Utilities
		void			 Slerp(const AxisAngle& axisAngleTo, float alpha = 1.0f);
		static AxisAngle Slerp(const AxisAngle& axisAngleFrom, const AxisAngle& axisAngleTo, float alpha = 1.0f);

		// Converters
		Vector3		ToDirection() const;
		EulerAngles ToEulerAngles() const;
		Quaternion	ToQuaternion() const;
		Matrix		ToRotationMatrix() const;

		// Operators
		bool	   operator ==(const AxisAngle& axisAngle) const;
		bool	   operator !=(const AxisAngle& axisAngle) const;
		AxisAngle& operator =(const AxisAngle& axisAngle);
		AxisAngle& operator *=(const AxisAngle& axisAngle);
		AxisAngle  operator *(const AxisAngle& axisAngle) const;
	};
//}
