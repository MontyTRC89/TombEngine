#include "framework.h"
#include "Math/Objects/AxisAngle.h"

#include "Math/Legacy.h"
#include "Math/Objects/EulerAngles.h"

namespace TEN::Math
{
	const AxisAngle AxisAngle::Zero = AxisAngle(Vector3::Zero, 0);

	AxisAngle::AxisAngle()
	{
	}

	AxisAngle::AxisAngle(const Vector3& axis, short angle)
	{
		auto axisNorm = axis;
		axisNorm.Normalize();

		this->Axis = axisNorm;
		this->Angle = angle;
	}

	AxisAngle::AxisAngle(const Vector3& direction)
	{
		this->Axis = direction;
		this->Angle = 0;
	}

	AxisAngle::AxisAngle(const EulerAngles& eulers)
	{
		float sinX = sin(FROM_RAD(eulers.x));
		float cosX = cos(FROM_RAD(eulers.x));
		float sinY = sin(FROM_RAD(eulers.y));
		float cosY = cos(FROM_RAD(eulers.y));
		float sinZ = sin(FROM_RAD(eulers.z));
		float cosZ = cos(FROM_RAD(eulers.z));

		auto axis = Vector3(
			(sinY * cosZ) + (cosY * sinX * sinZ),
			cosX * sinZ,
			(-cosY * cosZ) + (sinY * sinX * sinZ));
		axis.Normalize();
		float angle = acos(std::clamp((cosX * cosY * cosZ) + (sinX * sinY * sinZ), -1.0f, 1.0f));

		this->Axis = axis;
		this->Angle = FROM_RAD(angle);
	}

	AxisAngle::AxisAngle(const Quaternion& quat)
	{
		auto axis = Quaternion::Identity;
		float angle = 0.0f;
		XMQuaternionToAxisAngle((XMVECTOR*)&axis, &angle, quat);

		this->Axis = Vector3(axis);
		this->Angle = FROM_RAD(angle);
	}

	AxisAngle::AxisAngle(const Matrix& rotMatrix)
	{
		static constexpr auto epsilon = 0.00001f;

		float trace = rotMatrix(1, 1) + rotMatrix(2, 2) + rotMatrix(3, 3);
		float cosAngle = 0.5f * (trace - 1.0f);
		this->Angle = FROM_RAD(acos(cosAngle));

		if (abs(TO_RAD(Angle)) <= epsilon)
		{
			this->Axis = Vector3::UnitX;
		}
		else
		{
			float invSinAngle = 1.0f / sin(TO_RAD(Angle));
			this->Axis = Vector3(
				(rotMatrix(3, 2) - rotMatrix(2, 3)) * invSinAngle,
				(rotMatrix(1, 3) - rotMatrix(3, 1)) * invSinAngle,
				(rotMatrix(2, 1) - rotMatrix(1, 2)) * invSinAngle);
		}
	}

	Vector3 AxisAngle::GetAxis() const
	{
		return Axis;
	}

	short AxisAngle::GetAngle() const
	{
		return Angle;
	}

	void AxisAngle::SetAxis(const Vector3& axis)
	{
		auto axisNorm = axis;
		axisNorm.Normalize();
		
		this->Axis = axisNorm;
	}

	void AxisAngle::SetAngle(short angle)
	{
		this->Angle = angle;
	}

	void AxisAngle::Slerp(const AxisAngle& axisAngleTo, float alpha)
	{
		*this = Slerp(*this, axisAngleTo, alpha);
	}

	AxisAngle AxisAngle::Slerp(const AxisAngle& axisAngleFrom, const AxisAngle& axisAngleTo, float alpha)
	{
		static constexpr auto epsilon = 0.00001f;

		auto axis = Vector3::Zero;
		float angle = 0.0f;

		// Find the angle between the two axes.
		float angleBetweenAxes = acos(axisAngleFrom.GetAxis().Dot(axisAngleTo.GetAxis()));

		// If the angle between the axes is close to 0, do simple interpolation of angle values.
		if (abs(angleBetweenAxes) <= epsilon)
		{
			axis = axisAngleFrom.GetAxis();
			angle = TO_RAD(axisAngleFrom.GetAngle()) + (TO_RAD(axisAngleTo.GetAngle() - axisAngleFrom.GetAngle()) * alpha);
			return AxisAngle(axis, FROM_RAD(angle));
		}

		float sinAngle = sin(angleBetweenAxes);
		float weight0 = sin((1.0f - alpha) * angleBetweenAxes) / sinAngle;
		float weight1 = sin(alpha * angleBetweenAxes) / sinAngle;

		// Slerp axes and angles.
		axis = (axisAngleFrom.GetAxis() * weight0) + (axisAngleTo.GetAxis() * weight1);
		angle = (TO_RAD(axisAngleFrom.GetAngle()) * weight0) + (TO_RAD(axisAngleTo.GetAngle()) * weight1);
		return AxisAngle(axis, FROM_RAD(angle));
	}

	Vector3 AxisAngle::RotatePoint(const Vector3& point)
	{
		auto quat = this->ToQuaternion();
		auto pointAsQuat = Quaternion(point, 0.0f);
		auto quatInverse = quat;
		quatInverse.Inverse(quatInverse);

		// Rotate the point.
		return Vector3(pointAsQuat * (quat * quatInverse));
	}

	Vector3 AxisAngle::ToDirection() const
	{
		auto quat = this->ToQuaternion();

		// Rotate the point (0, 0, 1) by the quaternion.
		auto direction = Vector3(0.0f, 0.0f, 1.0f);
		auto pointAsQuat = Quaternion(direction, 0.0f);
		auto newPointAsQuat = quat * pointAsQuat;

		// Extract and normalize the direction vector.
		direction = Vector3(newPointAsQuat);
		direction.Normalize();
		return direction;
	}

	EulerAngles AxisAngle::ToEulerAngles() const
	{
		return EulerAngles(*this);
	}

	Quaternion AxisAngle::ToQuaternion() const
	{
		return Quaternion::CreateFromAxisAngle(Axis, TO_RAD(Angle));
	}

	Matrix AxisAngle::ToRotationMatrix() const
	{
		return Matrix::CreateFromAxisAngle(Axis, TO_RAD(Angle));
	}

	bool AxisAngle::operator ==(const AxisAngle& axisAngle) const
	{
		return ((Axis == axisAngle.GetAxis()) && (Angle == axisAngle.GetAngle()));
	}

	bool AxisAngle::operator !=(const AxisAngle& axisAngle) const
	{
		return !(*this == axisAngle);
	}

	AxisAngle& AxisAngle::operator =(const AxisAngle& axisAngle)
	{
		this->Axis = axisAngle.GetAxis();
		this->Angle = axisAngle.GetAngle();
		return *this;
	}

	AxisAngle& AxisAngle::operator *=(const AxisAngle& axisAngle)
	{
		*this = *this * axisAngle;
		return *this;
	}

	AxisAngle AxisAngle::operator *(const AxisAngle& axisAngle) const
	{
		auto axis = Axis.Cross(axisAngle.GetAxis());
		short angle = Angle + axisAngle.GetAngle();
		return AxisAngle(axis, angle);
	}
}
