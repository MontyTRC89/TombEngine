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

	// TODO: Check.
	EulerAngles::EulerAngles(const Vector3& direction)
	{
		auto directionNorm = direction;
		directionNorm.Normalize();
		this->x = FROM_RAD(atan2(directionNorm.y, directionNorm.x));
		this->y = FROM_RAD(-asin(directionNorm.z));
		this->z = 0.0f;
	}

	// TODO: Check.
	EulerAngles::EulerAngles(const Quaternion& quat)
	{
		// X axis
		float sinP = ((quat.w * quat.y) - (quat.z * quat.x)) * 2;
		// Use 90 degrees if out of range.
		if (std::abs(sinP) >= 1)
			this->x = FROM_RAD(std::copysign(PI_DIV_2, sinP));
		else
			this->x = FROM_RAD(asin(sinP));

		// Y axis
		float sinYCosP = ((quat.w * quat.z) + (quat.x * quat.y)) * 2;
		float cosYCosP = 1 - (((quat.y * quat.y) * 2) + (quat.z * quat.z));
		this->y = phd_atan(cosYCosP, sinYCosP);

		// Z axis
		float sinRCosP = ((quat.w * quat.x) + (quat.y * quat.z)) * 2;
		float cosRCosP = 1 - (((quat.x * quat.x) * 2) + (quat.y * quat.y));
		this->z = phd_atan(cosRCosP, sinRCosP);
	}

	// TODO: Check.
	Vector3 EulerAngles::ToDirection() const
	{
		float sinX = phd_sin(x);
		float cosX = phd_cos(x);
		float sinY = phd_sin(y);
		float cosY = phd_cos(y);

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

	bool EulerAngles::operator ==(const EulerAngles& eulers)
	{
		return (x == eulers.x && y == eulers.y && z == eulers.z);
	}

	bool EulerAngles::operator !=(const EulerAngles& eulers)
	{
		return (x != eulers.x || y != eulers.y || z != eulers.z);
	}

	EulerAngles EulerAngles::operator +(const EulerAngles& eulers)
	{
		return EulerAngles(x + eulers.x, y + eulers.y, z + eulers.z);
	}

	EulerAngles EulerAngles::operator -(const EulerAngles& eulers)
	{
		return EulerAngles(x - eulers.x, y - eulers.y, z - eulers.z);
	}

	EulerAngles EulerAngles::operator *(const EulerAngles& eulers)
	{
		return EulerAngles(x * eulers.x, y * eulers.y, z * eulers.z);
	}

	EulerAngles EulerAngles::operator *(float value)
	{
		return EulerAngles((short)round(x * value), (short)round(y * value), (short)round(z * value));
	}

	EulerAngles EulerAngles::operator /(float value)
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
