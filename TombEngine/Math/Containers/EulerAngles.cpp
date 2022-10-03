#include "framework.h"
#include "Math/Containers/EulerAngles.h"

#include "Math/Constants.h"
#include "Math/Legacy.h"

//namespace TEN::Math
//{
	const EulerAngles EulerAngles::Zero = EulerAngles(0, 0, 0);

	EulerAngles::EulerAngles()
	{
	}

	EulerAngles::EulerAngles(short x, short y, short z)
	{
		this->x = x;
		this->y = y;
		this->z = z;
	}

	EulerAngles::EulerAngles(const Vector3& direction)
	{
		auto directionNorm = direction;
		directionNorm.Normalize();

		this->x = FROM_RAD(-asin(directionNorm.y));
		this->y = FROM_RAD(atan2(directionNorm.x, directionNorm.z));
		this->z = 0;
	}

	// TODO: Check.
	EulerAngles::EulerAngles(const Quaternion& quat)
	{
		// X axis
		float sinX = ((quat.w * quat.y) - (quat.z * quat.x)) * 2;
		// Use 90 degrees if out of range.
		if (std::abs(sinX) >= 1)
			this->x = FROM_RAD(std::copysign(PI_DIV_2, sinX));
		else
			this->x = FROM_RAD(asin(sinX));

		// Y axis
		float sinYCosX = ((quat.w * quat.z) + (quat.x * quat.y)) * 2;
		float cosYCosX = 1 - (((quat.y * quat.y) * 2) + (quat.z * quat.z));
		this->y = FROM_RAD(atan2(sinYCosX, cosYCosX));

		// Z axis
		float sinZCosX = ((quat.w * quat.x) + (quat.y * quat.z)) * 2;
		float cosZCosX = 1 - (((quat.x * quat.x) * 2) + (quat.y * quat.y));
		this->z = FROM_RAD(atan2(sinZCosX, cosZCosX));
	}

	// TODO: Check.
	EulerAngles::EulerAngles(const Matrix& rotMatrix)
	{
		this->x = FROM_RAD(asin(rotMatrix._31));
		this->y = FROM_RAD(-atan2(rotMatrix._21, rotMatrix._11));
		this->z = FROM_RAD(atan2(rotMatrix._32, rotMatrix._33));
	}

	Vector3 EulerAngles::ToDirection() const
	{
		float sinX = phd_sin(x);
		float cosX = phd_cos(x);
		float sinY = phd_sin(y);
		float cosY = phd_cos(y);

		// Return normalized direction vector.
		return Vector3(
			sinY * cosX,
			-sinX,
			cosY * cosX
		);
	}

	Quaternion EulerAngles::ToQuaternion() const
	{
		return Quaternion::CreateFromYawPitchRoll(TO_RAD(y), TO_RAD(x), TO_RAD(z));
	}

	Matrix EulerAngles::ToRotationMatrix() const
	{
		return Matrix::CreateFromYawPitchRoll(TO_RAD(y), TO_RAD(x), TO_RAD(z));
	}

	bool EulerAngles::operator ==(const EulerAngles& eulers) const
	{
		return ((x == eulers.x) && (y == eulers.y) && (z == eulers.z));
	}

	bool EulerAngles::operator !=(const EulerAngles& eulers) const
	{
		return ((x != eulers.x) || (y != eulers.y) || (z != eulers.z));
	}

	EulerAngles EulerAngles::operator +(const EulerAngles& eulers) const
	{
		return EulerAngles(x + eulers.x, y + eulers.y, z + eulers.z);
	}

	EulerAngles EulerAngles::operator -(const EulerAngles& eulers) const
	{
		return EulerAngles(x - eulers.x, y - eulers.y, z - eulers.z);
	}

	EulerAngles EulerAngles::operator *(const EulerAngles& eulers) const
	{
		return EulerAngles(x * eulers.x, y * eulers.y, z * eulers.z);
	}

	EulerAngles EulerAngles::operator *(float value) const
	{
		return EulerAngles((short)round(x * value), (short)round(y * value), (short)round(z * value));
	}

	EulerAngles EulerAngles::operator /(float value) const
	{
		return EulerAngles((short)round(x / value), (short)round(y / value), (short)round(z / value));
	}

	EulerAngles& EulerAngles::operator =(const EulerAngles& eulers)
	{
		this->x = eulers.x;
		this->y = eulers.y;
		this->z = eulers.z;
		return *this;
	}

	EulerAngles& EulerAngles::operator +=(const EulerAngles& eulers)
	{
		this->x += eulers.x;
		this->y += eulers.y;
		this->z += eulers.z;
		return *this;
	}

	EulerAngles& EulerAngles::operator -=(const EulerAngles& eulers)
	{
		this->x -= eulers.x;
		this->y -= eulers.y;
		this->z -= eulers.z;
		return *this;
	}

	EulerAngles& EulerAngles::operator *=(const EulerAngles& eulers)
	{
		this->x *= eulers.x;
		this->y *= eulers.y;
		this->z *= eulers.z;
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
