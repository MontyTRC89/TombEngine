#include "framework.h"
#include "Math/Angles/EulerAngles.h"

#include "Math/Angles/Angle.h"

//namespace TEN::Math::Angles
//{
	EulerAngles const EulerAngles::Zero = EulerAngles(0.0f, 0.0f, 0.0f);

	EulerAngles::EulerAngles()
	{
		*this = EulerAngles::Zero;
	}

	EulerAngles::EulerAngles(float xAngle, float yAngle, float zAngle)
	{
		this->x = xAngle;
		this->y = yAngle;
		this->z = zAngle;
	}

	// TODO
	EulerAngles::EulerAngles(Vector3 directionVector)
	{
		directionVector.Normalize();
		*this = EulerAngles::Zero;
	}

	bool EulerAngles::Compare(EulerAngles euler0, EulerAngles euler1, float epsilon)
	{
		if (Angle::Compare(euler0.x, euler1.x, epsilon) &&
			Angle::Compare(euler0.y, euler1.y, epsilon) &&
			Angle::Compare(euler0.z, euler1.z, epsilon))
		{
			return true;
		}

		return false;
	}

	EulerAngles EulerAngles::GetShortestAngularDistance(EulerAngles eulerFrom, EulerAngles eulerTo)
	{
		return (eulerTo - eulerFrom);
	}

	void EulerAngles::InterpolateLinear(EulerAngles eulerTo, float alpha, float epsilon)
	{
		this->x.InterpolateLinear(eulerTo.x, alpha, epsilon);
		this->y.InterpolateLinear(eulerTo.y, alpha, epsilon);
		this->z.InterpolateLinear(eulerTo.z, alpha, epsilon);
	}

	EulerAngles EulerAngles::InterpolateLinear(EulerAngles eulerFrom, EulerAngles eulerTo, float alpha, float epsilon)
	{
		return EulerAngles(
			Angle::InterpolateLinear(eulerFrom.x, eulerTo.x, alpha, epsilon),
			Angle::InterpolateLinear(eulerFrom.y, eulerTo.y, alpha, epsilon),
			Angle::InterpolateLinear(eulerFrom.z, eulerTo.z, alpha, epsilon)
		);
	}

	void EulerAngles::InterpolateConstant(EulerAngles eulerTo, float rate)
	{
		this->x.InterpolateConstant(eulerTo.x, rate);
		this->y.InterpolateConstant(eulerTo.y, rate);
		this->z.InterpolateConstant(eulerTo.z, rate);
	}

	EulerAngles EulerAngles::InterpolateConstant(EulerAngles eulerFrom, EulerAngles eulerTo, float rate)
	{
		return EulerAngles(
			Angle::InterpolateConstant(eulerFrom.x, eulerTo.x, rate),
			Angle::InterpolateConstant(eulerFrom.y, eulerTo.y, rate),
			Angle::InterpolateConstant(eulerFrom.z, eulerTo.z, rate)
		);
	}

	void EulerAngles::InterpolateConstantEaseOut(EulerAngles eulerTo, float rate, float alpha, float epsilon)
	{
		this->x.InterpolateConstantEaseOut(eulerTo.x, alpha, epsilon);
		this->y.InterpolateConstantEaseOut(eulerTo.y, alpha, epsilon);
		this->z.InterpolateConstantEaseOut(eulerTo.z, alpha, epsilon);
	}

	EulerAngles EulerAngles::InterpolateConstantEaseOut(EulerAngles eulerFrom, EulerAngles eulerTo, float rate, float alpha, float epsilon)
	{
		return EulerAngles(
			Angle::InterpolateConstantEaseOut(eulerFrom.x, eulerTo.x, rate, alpha, epsilon),
			Angle::InterpolateConstantEaseOut(eulerFrom.y, eulerTo.y, rate, alpha, epsilon),
			Angle::InterpolateConstantEaseOut(eulerFrom.z, eulerTo.z, rate, alpha, epsilon)
		);
	}

	Vector3 EulerAngles::GetDirectionVector()
	{
		float sinX = sin(x);
		float cosX = cos(x);
		float sinY = sin(y);
		float cosY = cos(y);

		// TODO: Since -y is up, need to check if this is correct.
		return Vector3(
			sinY * cosX,
			-sinX,
			cosY * cosX
		);
	}

	bool EulerAngles::operator ==(EulerAngles euler)
	{
		return (x == euler.x && y == euler.y && z == euler.z);
	}

	bool EulerAngles::operator !=(EulerAngles euler)
	{
		return (x != euler.x || y != euler.y || z != euler.z);
	}

	EulerAngles EulerAngles::operator +(EulerAngles euler)
	{
		return EulerAngles(x + euler.x, y + euler.y, z + euler.z);
	}

	EulerAngles EulerAngles::operator -(EulerAngles euler)
	{
		return EulerAngles(x - euler.x, y - euler.y, z - euler.z);
	}

	EulerAngles EulerAngles::operator *(EulerAngles euler)
	{
		return EulerAngles(x * euler.x, y * euler.y, z * euler.z);
	}

	EulerAngles EulerAngles::operator *(float value)
	{
		return EulerAngles(x * value, y * value, z * value);
	}

	EulerAngles EulerAngles::operator /(float value)
	{
		return EulerAngles(x / value, y / value, z / value);
	}

	EulerAngles& EulerAngles::operator =(EulerAngles euler)
	{
		this->x = euler.x;
		this->y = euler.y;
		this->z = euler.z;
		return *this;
	}
	
	EulerAngles& EulerAngles::operator +=(EulerAngles euler)
	{
		this->x += euler.x;
		this->y += euler.y;
		this->z += euler.z;
		return *this;
	}

	EulerAngles& EulerAngles::operator -=(EulerAngles euler)
	{
		this->x -= euler.x;
		this->y -= euler.y;
		this->z -= euler.z;
		return *this;
	}

	EulerAngles& EulerAngles::operator *=(EulerAngles euler)
	{
		this->x *= euler.x;
		this->y *= euler.y;
		this->z *= euler.z;
		return *this;
	}

	EulerAngles& EulerAngles::operator *=(float value)
	{
		this->x *= value;
		this->y *= value;
		this->z *= value;
		return *this;
	}

	EulerAngles& EulerAngles::operator /=(float value)
	{
		this->x /= value;
		this->y /= value;
		this->z /= value;
		return *this;
	}
//}
