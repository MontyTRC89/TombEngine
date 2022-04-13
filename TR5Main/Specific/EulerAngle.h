#pragma once
#include "framework.h"
#include "Specific/trmath.h"

class EulerAngle : public Vector3
{
private:
	// TODO: Due to clamping requirements, the components must be made private?
	//using Vector3::x;
	//using Vector3::y;
	//using Vector3::z;

	EulerAngle(Vector3 orient);

public:
	EulerAngle();
	EulerAngle(float xRadians, float yRadians, float zRadians);

	EulerAngle operator ==(const EulerAngle orient);
	EulerAngle operator !=(const EulerAngle orient);
	EulerAngle operator +(const EulerAngle orient);
	EulerAngle operator -(const EulerAngle orient);
	EulerAngle operator *(const EulerAngle orient);
	EulerAngle operator *(const float value);
	EulerAngle operator /(const float value);
	EulerAngle& operator +=(const EulerAngle orient);
	EulerAngle& operator -=(const EulerAngle orient);
	EulerAngle& operator *=(const EulerAngle orient);
	EulerAngle& operator *=(const float value);
	EulerAngle& operator /=(const float value);

	EulerAngle Get();
	float GetX();
	float GetY();
	float GetZ();

	void Set(EulerAngle orient);
	void Set(float xRadians, float yRadians, float zRadians);
	void SetX(float radians);
	void SetY(float radians);
	void SetZ(float radians);

	void Clamp();
	EulerAngle Interpolate(EulerAngle targetOrient, float rate);
	Vector3 ToVector3();

	static float Clamp(float radians);
	static EulerAngle Interpolate(EulerAngle startOrient, EulerAngle targetOrient, float rate);
	static float ShortestAngle(float radiansFrom, float radiansTo);
	static float DegToRad(float degrees);
	static float RadToDeg(float radians);
	static float ShrtToRad(short shortForm);
};

inline EulerAngle::EulerAngle(Vector3 orient) :
	Vector3(orient)
{
}

inline EulerAngle::EulerAngle()
{
}

inline EulerAngle::EulerAngle(float xRadians, float yRadians, float zRadians) :
	Vector3(xRadians, yRadians, zRadians)
{
}

inline EulerAngle EulerAngle::operator ==(const EulerAngle orient)
{
	auto copy = orient;
	copy.Clamp();
	return *this == copy;
}

inline EulerAngle EulerAngle::operator !=(const EulerAngle orient)
{
	auto copy = orient;
	copy.Clamp();
	return *this != copy;
}

inline EulerAngle& EulerAngle::operator +=(const EulerAngle orient)
{
	*this += orient;
	this->Clamp();
	return *this;
}

inline EulerAngle EulerAngle::operator +(const EulerAngle orient)
{
	auto newOrient = *this + orient;
	newOrient.Clamp();
	return newOrient;
}

inline EulerAngle EulerAngle::operator -(const EulerAngle orient)
{
	auto newOrient = *this + orient;
	newOrient.Clamp();
	return newOrient;
}

inline EulerAngle EulerAngle::operator *(const EulerAngle orient)
{
	auto newOrient = *this * orient;
	newOrient.Clamp();
	return newOrient;
}

inline EulerAngle EulerAngle::operator *(const float value)
{
	auto newOrient = *this * value;
	newOrient.Clamp();
	return newOrient;
}

inline EulerAngle EulerAngle::operator /(const float value)
{
	auto newOrient = *this / value;
	newOrient.Clamp();
	return newOrient;
}

inline EulerAngle& EulerAngle::operator -=(const EulerAngle orient)
{
	*this -= orient;
	this->Clamp();
	return *this;
}

inline EulerAngle& EulerAngle::operator *=(const EulerAngle orient)
{
	*this *= orient;
	this->Clamp();
	return *this;
}

inline EulerAngle& EulerAngle::operator *=(const float value)
{
	*this *= value;
	this->Clamp();
	return *this;
}

inline EulerAngle& EulerAngle::operator /=(const float value)
{
	*this /= value;
	this->Clamp();
	return *this;
}

inline EulerAngle EulerAngle::Get()
{
	return EulerAngle(x, y, z);
}

inline float EulerAngle::GetX()
{
	return x;
}

inline float EulerAngle::GetY()
{
	return y;
}

inline float EulerAngle::GetZ()
{
	return z;
}

inline void EulerAngle::Set(EulerAngle orient = Vector3::Zero)
{
	*this = orient;
	this->Clamp();
}

inline void EulerAngle::Set(float xRadians, float yRadians, float zRadians)
{
	this->x = xRadians;
	this->y = yRadians;
	this->z = zRadians;
	this->Clamp();
}

inline void EulerAngle::SetX(float radians = 0)
{
	this->x = Clamp(radians);
}

inline void EulerAngle::SetY(float radians = 0)
{
	this->y = Clamp(radians);
}

inline void EulerAngle::SetZ(float radians = 0)
{
	this->z = Clamp(radians);
}

inline void EulerAngle::Clamp()
{
	this->x = Clamp(x);
	this->y = Clamp(y);
	this->z = Clamp(z);
}

inline EulerAngle EulerAngle::Interpolate(EulerAngle targetOrient, float rate = 1.0f)
{
	auto startOrient = *this;
	startOrient.Clamp();
	targetOrient.Clamp();

	auto difference = EulerAngle(targetOrient - startOrient);
	difference.Clamp();
	difference *= rate;
	return (startOrient + difference);
}

inline Vector3 EulerAngle::ToVector3()
{
	return Vector3(x, y, z);
}

inline float EulerAngle::Clamp(float radians)
{
	return atan2(sin(radians), cos(radians));

	// Alternative method:
	/*if (radians > 0)
		radians = fmod(radians + M_PI, M_PI * 2) - M_PI;
	else
		radians = fmod(radians - M_PI, M_PI * 2) + M_PI;

	return radians;*/
}

inline EulerAngle EulerAngle::Interpolate(EulerAngle startOrient, EulerAngle targetOrient, float rate = 1.0f)
{
	startOrient.Clamp();
	targetOrient.Clamp();

	auto difference = EulerAngle(targetOrient - startOrient);
	difference.Clamp();
	difference *= rate;
	return (startOrient + difference);
}

inline float EulerAngle::ShortestAngle(float radiansFrom, float radiansTo)
{
	//float from = Clamp(radiansFrom);
	//float to = Clamp(radiansTo);
	return Clamp(radiansTo - radiansFrom);
}

inline float EulerAngle::DegToRad(float degrees)
{
	return Clamp(degrees * (M_PI / 180.0f));
}

inline float EulerAngle::RadToDeg(float radians)
{
	float degrees = Clamp(radians) * (180.0f / M_PI);
	degrees += (degrees < 0) ? 360.0f : 0;
	return degrees;
}

inline float EulerAngle::ShrtToRad(short shortForm)
{
	return Clamp(shortForm * (360.0f / (USHRT_MAX + 1)) * (180.0f / M_PI));
}
