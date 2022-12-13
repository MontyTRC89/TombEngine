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

EulerAngles::EulerAngles(const Quaternion& quat)
{
	static constexpr float singularityLimit = 0.9999995f;

	// Handle singularity case.
	float sinP = 2.0f * ((quat.w * quat.x) - (quat.y * quat.z));
	if (abs(sinP) > singularityLimit)
	{
		if (sinP > 0.0f)
			*this = EulerAngles(FROM_RAD(PI_DIV_2), 0, FROM_RAD(2.0f * FROM_RAD(atan2(quat.z, quat.w))));
		else
			*this = EulerAngles(FROM_RAD(-PI_DIV_2), 0, FROM_RAD(-2.0f * atan2(quat.z, quat.w)));
	}

	// Roll (X axis)
	float sinR = 2.0f * ((quat.w * quat.z) + (quat.x * quat.y));
	float cosR = 1.0f - (2.0f * (SQUARE(x) + SQUARE(quat.x)));
	float roll = atan2(sinR, cosR);

	// Pitch (Y axis)
	float pitch = 0.0f;
	if (abs(sinP) >= 1.0f)
		pitch = (sinP > 0) ? PI_DIV_2 : -PI_DIV_2;
	else
		pitch = asin(sinP);

	// Yaw (Z axis)
	float sinY = 2.0f * ((quat.w * quat.y) + (quat.z * quat.x));
	float cosY = 1.0f - (2.0f * (SQUARE(quat.x) + SQUARE(quat.y)));
	float yaw = atan2(sinY, cosY);

	*this = EulerAngles(FROM_RAD(pitch), FROM_RAD(yaw), FROM_RAD(roll));
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

void EulerAngles::InterpolateConstant(const EulerAngles& eulersTo, short angularVel)
{
	*this = InterpolateConstant(*this, eulersTo, angularVel);
}

EulerAngles EulerAngles::InterpolateConstant(const EulerAngles& eulersFrom, const EulerAngles& eulerTo, short angularVel)
{
	return EulerAngles(
		InterpolateConstant(eulersFrom.x, eulerTo.x, angularVel),
		InterpolateConstant(eulersFrom.y, eulerTo.y, angularVel),
		InterpolateConstant(eulersFrom.z, eulerTo.z, angularVel));
}

void EulerAngles::Lerp(const EulerAngles& eulersTo, float alpha, short epsilon)
{
	*this = Lerp(*this, eulersTo, alpha, epsilon);
}

EulerAngles EulerAngles::Lerp(const EulerAngles& eulersFrom, const EulerAngles& eulersTo, float alpha, short epsilon)
{
	return EulerAngles(
		Lerp(eulersFrom.x, eulersTo.x, alpha, epsilon),
		Lerp(eulersFrom.y, eulersTo.y, alpha, epsilon),
		Lerp(eulersFrom.z, eulersTo.z, alpha, epsilon));
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
		cosY * cosX);
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

float EulerAngles::ClampAlpha(float alpha)
{
	return ((abs(alpha) > 1.0f) ? 1.0f : abs(alpha));
}

bool EulerAngles::Compare(short angle0, short angle1, short epsilon)
{
	short difference = Geometry::GetShortestAngle(angle0, angle1);
	if (abs(difference) <= epsilon)
		return true;

	return false;
}

short EulerAngles::Lerp(short angleFrom, short angleTo, float alpha, short epsilon)
{
	alpha = ClampAlpha(alpha);

	if (Compare(angleFrom, angleTo, epsilon))
		return angleTo;

	short difference = Geometry::GetShortestAngle(angleFrom, angleTo);
	return (short)round(angleFrom + (difference * alpha));
}

short EulerAngles::InterpolateConstant(short angleFrom, short angleTo, short angularVel)
{
	if (Compare(angleFrom, angleTo, angularVel))
		return angleTo;

	int sign = copysign(1, Geometry::GetShortestAngle(angleFrom, angleTo));
	return (angleFrom + (angularVel * sign));
}
//}
