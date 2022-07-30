#include "framework.h"
#include "Math/Angles/EulerAngles.h"

#include "Math/Angles/Angle.h"

EulerAngles const EulerAngles::Zero = EulerAngles(0.0f, 0.0f, 0.0f);

EulerAngles::EulerAngles()
{
	*this = EulerAngles::Zero;
}

EulerAngles::EulerAngles(float xAngle, float yAngle, float zAngle)
{
	this->Set(xAngle, yAngle, zAngle);
}

EulerAngles::EulerAngles(Vector3 directionVector)
{
	directionVector.Normalize();
	this->Set(0.0f, 0.0f, 0.0f);
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

float EulerAngles::GetX()
{
	return x;
}

float EulerAngles::GetY()
{
	return y;
}

float EulerAngles::GetZ()
{
	return z;
}

void EulerAngles::Set(EulerAngles orient)
{
	*this = orient;
}

void EulerAngles::Set(float xAngle, float yAngle, float zAngle)
{
	this->SetX(xAngle);
	this->SetY(yAngle);
	this->SetZ(zAngle);
}

void EulerAngles::SetX(float angle)
{
	this->x = Angle::Normalize(angle);
}

void EulerAngles::SetY(float angle)
{
	this->y = Angle::Normalize(angle);
}

void EulerAngles::SetZ(float angle)
{
	this->z = Angle::Normalize(angle);
}

void EulerAngles::Normalize()
{
	this->Set(x, y, z);
}

EulerAngles EulerAngles::Normalize(EulerAngles orient)
{
	orient.Normalize();
	return orient;
}

bool EulerAngles::Compare(EulerAngles orient, float epsilon)
{
	return Compare(*this, orient, epsilon);
}

bool EulerAngles::Compare(EulerAngles orient0, EulerAngles orient1, float epsilon)
{
	if (Angle::Compare(orient0.x, orient1.x, epsilon) &&
		Angle::Compare(orient0.y, orient1.y, epsilon) &&
		Angle::Compare(orient0.z, orient1.z, epsilon))
	{
		return true;
	}

	return false;
}

EulerAngles EulerAngles::ShortestAngularDistance(EulerAngles orientTo)
{
	return (orientTo - *this);
}

EulerAngles EulerAngles::ShortestAngularDistance(EulerAngles orientFrom, EulerAngles orientTo)
{
	return (orientTo - orientFrom);
}

void EulerAngles::InterpolateLinear(EulerAngles orientTo, float alpha, float epsilon)
{
	this->Set(
		Angle::InterpolateLinear(x, orientTo.x, alpha, epsilon),
		Angle::InterpolateLinear(y, orientTo.y, alpha, epsilon),
		Angle::InterpolateLinear(z, orientTo.z, alpha, epsilon));
}

EulerAngles EulerAngles::InterpolateLinear(EulerAngles orientFrom, EulerAngles orientTo, float alpha, float epsilon)
{
	return EulerAngles(
		Angle::InterpolateLinear(orientFrom.x, orientTo.x, alpha, epsilon),
		Angle::InterpolateLinear(orientFrom.y, orientTo.y, alpha, epsilon),
		Angle::InterpolateLinear(orientFrom.z, orientTo.z, alpha, epsilon)
	);
}

void EulerAngles::InterpolateConstant(EulerAngles orientTo, float rate)
{
	this->Set(
		Angle::InterpolateConstant(x, orientTo.x, rate),
		Angle::InterpolateConstant(y, orientTo.y, rate),
		Angle::InterpolateConstant(z, orientTo.z, rate));
}

EulerAngles EulerAngles::InterpolateConstant(EulerAngles orientFrom, EulerAngles orientTo, float rate)
{
	return EulerAngles(
		Angle::InterpolateConstant(orientFrom.x, orientTo.x, rate),
		Angle::InterpolateConstant(orientFrom.y, orientTo.y, rate),
		Angle::InterpolateConstant(orientFrom.z, orientTo.z, rate)
	);
}

void EulerAngles::InterpolateConstantEaseOut(EulerAngles orientTo, float rate, float alpha, float epsilon)
{
	this->Set(
		Angle::InterpolateConstantEaseOut(x, orientTo.x, rate, alpha, epsilon),
		Angle::InterpolateConstantEaseOut(y, orientTo.y, rate, alpha, epsilon),
		Angle::InterpolateConstantEaseOut(z, orientTo.z, rate, alpha, epsilon));
}

EulerAngles EulerAngles::InterpolateConstantEaseOut(EulerAngles orientFrom, EulerAngles orientTo, float rate, float alpha, float epsilon)
{
	return EulerAngles(
		Angle::InterpolateConstantEaseOut(orientFrom.x, orientTo.x, rate, alpha, epsilon),
		Angle::InterpolateConstantEaseOut(orientFrom.y, orientTo.y, rate, alpha, epsilon),
		Angle::InterpolateConstantEaseOut(orientFrom.z, orientTo.z, rate, alpha, epsilon)
	);
}

// TODO: This is a geometry function and it doesn't belong in this class. Also name should be GetOrientTowardPoint().
EulerAngles EulerAngles::OrientBetweenPoints(Vector3 origin, Vector3 target)
{
	auto direction = target - origin;
	float yOrient = atan2(direction.x, direction.z);

	auto vector = direction;
	auto matrix = Matrix::CreateRotationY(-yOrient);
	Vector3::Transform(vector, matrix, vector);

	float xOrient = -atan2(direction.y, vector.z);
	return EulerAngles(xOrient, yOrient, 0.0f);
}

bool EulerAngles::operator ==(EulerAngles orient)
{
	return (x == orient.x && y == orient.y && z == orient.z);
}

bool EulerAngles::operator !=(EulerAngles orient)
{
	return (x != orient.x || y != orient.y || z != orient.z);
}

EulerAngles EulerAngles::operator +(EulerAngles orient)
{
	return EulerAngles(x + orient.x, y + orient.y, z + orient.z);
}

EulerAngles EulerAngles::operator -(EulerAngles orient)
{
	return EulerAngles(x - orient.x, y - orient.y, z - orient.z);
}

EulerAngles EulerAngles::operator *(EulerAngles orient)
{
	return EulerAngles(x * orient.x, y * orient.y, z * orient.z);
}

EulerAngles EulerAngles::operator *(float value)
{
	return EulerAngles(x * value, y * value, z * value);
}

EulerAngles EulerAngles::operator /(float value)
{
	return EulerAngles(x / value, y / value, z / value);
}

EulerAngles& EulerAngles::operator +=(EulerAngles orient)
{
	this->Set(x + orient.x, y + orient.y, z + orient.z);
	return *this;
}

EulerAngles& EulerAngles::operator -=(EulerAngles orient)
{
	this->Set(x - orient.x, y - orient.y, z - orient.z);
	return *this;
}

EulerAngles& EulerAngles::operator *=(EulerAngles orient)
{
	this->Set(x * orient.x, y * orient.y, z * orient.z);
	return *this;
}

EulerAngles& EulerAngles::operator *=(float value)
{
	this->Set(x * value, y * value, z * value);
	return *this;
}

EulerAngles& EulerAngles::operator /=(float value)
{
	this->Set(x / value, y / value, z / value);
	return *this;
}
