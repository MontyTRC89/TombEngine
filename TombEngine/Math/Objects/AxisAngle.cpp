#include "framework.h"
#include "Math/Objects/AxisAngle.h"

#include "Math/Constants.h"
#include "Math/Legacy.h"
#include "Math/Objects/EulerAngles.h"

//namespace TEN::Math
//{
	const AxisAngle AxisAngle::Identity = AxisAngle(Vector3::Zero, 0);

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

	AxisAngle::AxisAngle(const EulerAngles& eulers)
	{
		*this = AxisAngle(eulers.ToQuaternion());
	}

	// NOTE: Some precision drift may occur.
	AxisAngle::AxisAngle(const Quaternion& quat)
	{
		float scale = sqrt(1.0f - quat.w * quat.w);
		auto axis = Vector3(quat) / scale;
		axis.Normalize();
		float angle = 2.0f * acos(quat.w);

		this->Axis = axis;
		this->Angle = FROM_RAD(angle);
	}

	AxisAngle::AxisAngle(const Matrix& rotMatrix)
	{
		// Decompose matrix into quaternion.
		auto scale = Vector3::Zero;
		auto quat = Quaternion::Identity;
		auto translation = Vector3::Zero;
		auto rotMatrixCopy = rotMatrix;
		rotMatrixCopy.Decompose(scale, quat, translation);

		// Convert quaternion to AxisAngle.
		*this = AxisAngle(quat);

		// Extract rotation axis from matrix.
		auto rotAxis = Vector3::TransformNormal(Vector3::Right, rotMatrix);
		
		// Check if rotation axis and unit axis are pointing in opposite directions.
		float dot = rotAxis.Dot(Axis);
		if (dot < 0.0f)
		{
			// Negate angle and unit axis to ensure the angle stays within [0, PI] range.
			this->Angle = -Angle;
			this->Axis = -Axis;
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
		auto axis = Vector3::Zero;
		float angle = 0.0f;

		// If angle between axes is close to 0, do simple interpolation of angle values.
		float angleBetweenAxes = acos(axisAngleFrom.GetAxis().Dot(axisAngleTo.GetAxis()));
		if (abs(angleBetweenAxes) <= EPSILON)
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

	// TODO: Get this working properly.
	Vector3 AxisAngle::ToDirection() const
	{
		float sinAngle = sin(TO_RAD(Angle));
		float cosAngle = cos(TO_RAD(Angle));

		auto a = Vector3(cosAngle, sinAngle, 0.0f);
		auto b = Axis;

		// Calculate the normal vector of the plane
		auto n = (b.Cross(-Vector3::UnitY).LengthSquared() > 0.0f) ?
			b.Cross(-Vector3::UnitY) :
			b.Cross(Vector3::UnitX);

		// Project the vector a onto the plane
		auto projection = a - (a.Dot(n) * n);
		projection.Normalize();
		return projection;

		//auto quat = this->ToQuaternion();
		//return Vector3::Transform(Vector3::UnitZ, quat);
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
		auto quat0 = this->ToQuaternion();
		auto quat1 = axisAngle.ToQuaternion();
		return AxisAngle(quat0 * quat1);
	}
//}
