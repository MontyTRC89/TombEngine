#include "framework.h"
#include "Math/Containers/EulerAngles.h"

#include "Math/Constants.h"
#include "Math/Geometry.h"
#include "Math/Legacy.h"

using namespace TEN::Math;

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

	bool EulerAngles::Compare(const EulerAngles& eulers0, const EulerAngles& eulers1, short epsilon)
	{
		if (Compare(eulers0.x, eulers1.x, epsilon) &&
			Compare(eulers0.y, eulers1.y, epsilon) &&
			Compare(eulers0.z, eulers1.z, epsilon))
		{
			return true;
		}

		return false;
	}

	void EulerAngles::Lerp(const EulerAngles& eulersTo, float alpha, short epsilon)
	{
		*this = Lerp(*this, eulersTo, alpha, epsilon);
	}

	EulerAngles EulerAngles::Lerp(const EulerAngles& eulersFrom, const EulerAngles& eulersTo, float alpha, short epsilon) const
	{
		return EulerAngles(
			InterpolateLinear(eulersFrom.x, eulersTo.x, alpha, epsilon),
			InterpolateLinear(eulersFrom.y, eulersTo.y, alpha, epsilon),
			InterpolateLinear(eulersFrom.z, eulersTo.z, alpha, epsilon)
		);
	}

	Vector3 EulerAngles::ToDirection() const
	{
		float sinX = sin(TO_RAD(x));
		float cosX = cos(TO_RAD(x));
		float sinY = sin(TO_RAD(y));
		float cosY = cos(TO_RAD(y));

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

	EulerAngles& EulerAngles::operator *=(float scale)
	{
		this->x *= scale;
		this->y *= scale;
		this->z *= scale;
		return *this;
	}

	EulerAngles& EulerAngles::operator /=(float scale)
	{
		this->x /= scale;
		this->y /= scale;
		this->z /= scale;
		return *this;
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

	EulerAngles EulerAngles::operator *(float scale) const
	{
		return EulerAngles((short)round(x * scale), (short)round(y * scale), (short)round(z * scale));
	}

	EulerAngles EulerAngles::operator /(float scale) const
	{
		return EulerAngles((short)round(x / scale), (short)round(y / scale), (short)round(z / scale));
	}

	float EulerAngles::ClampAlpha(float alpha) const
	{
		return ((abs(alpha) > 1.0f) ? 1.0f : abs(alpha));
	}

	bool EulerAngles::Compare(short angle0, short angle1, short epsilon) const
	{
		short difference = Geometry::GetShortestAngularDistance(angle0, angle1);
		if (abs(difference) <= epsilon)
			return true;

		return false;
	}

	short EulerAngles::InterpolateLinear(short angleFrom, short angleTo, float alpha, short epsilon) const
	{
		alpha = ClampAlpha(alpha);

		if (Compare(angleFrom, angleTo, epsilon))
			return angleTo;

		short difference = Geometry::GetShortestAngularDistance(angleFrom, angleTo);
		return (short)round(angleFrom + (difference * alpha));
	}
//}
