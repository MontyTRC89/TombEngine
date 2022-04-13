#pragma once
#include "framework.h"
#include "Specific/trmath.h"

class EulerAngle : public Vector3
{
public:
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
	void Interpolate(EulerAngle targetOrient, float rate);
	Vector3 ToVector3();

	static EulerAngle Clamp(EulerAngle orient);
	static float Clamp(float radians);
	static EulerAngle Interpolate(EulerAngle startOrient, EulerAngle targetOrient, float rate);
	static EulerAngle ShortestAngle(EulerAngle orientFrom, EulerAngle orientTo);
	static float ShortestAngle(float radiansFrom, float radiansTo);

	static float DegToRad(float degrees);
	static float RadToDeg(float radians);
	static float ShrtToRad(short shortForm);
	static short RadToShrt(float radians);

	bool operator ==(const EulerAngle orient);
	bool operator !=(const EulerAngle orient);
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

	EulerAngle();
	EulerAngle(float xRadians, float yRadians, float zRadians);

private:
	EulerAngle(Vector3 orient);

	// TODO: Due to clamping requirements, the components must be made private?
	//using Vector3::x;
	//using Vector3::y;
	//using Vector3::z;
};

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

inline void EulerAngle::Set(EulerAngle orient)
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

inline void EulerAngle::Interpolate(EulerAngle targetOrient, float rate = 1.0f)
{
	rate = (abs(rate) > 1.0f) ? 1.0f : abs(rate);

	auto startOrient = *this;
	startOrient.Clamp();
	targetOrient.Clamp();

	auto difference = ShortestAngle(*this, targetOrient);
	difference.Clamp();
	difference *= rate;
	*this = (startOrient + difference);
}

inline Vector3 EulerAngle::ToVector3()
{
	return Vector3(x, y, z);
}

inline EulerAngle EulerAngle::Clamp(EulerAngle orient)
{
	return Clamp(orient);
}

inline float EulerAngle::Clamp(float radians)
{
	return atan2(sin(radians), cos(radians));

	// Alternative method:
	/*return (radians > 0) ?
		fmod(radians + M_PI, M_PI * 2) - M_PI :
		fmod(radians - M_PI, M_PI * 2) + M_PI;*/
}

inline EulerAngle EulerAngle::Interpolate(EulerAngle startOrient, EulerAngle targetOrient, float rate = 1.0f)
{
	rate = (abs(rate) > 1.0f) ? 1.0f : abs(rate);

	startOrient.Clamp();
	targetOrient.Clamp();

	auto difference = EulerAngle(targetOrient - startOrient);
	difference.Clamp();
	difference *= rate;
	return (startOrient + difference);
}

inline EulerAngle EulerAngle::ShortestAngle(EulerAngle orientFrom, EulerAngle orientTo)
{
	return Clamp(orientTo - orientFrom);
}

inline float EulerAngle::ShortestAngle(float radiansFrom, float radiansTo)
{
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

inline short EulerAngle::RadToShrt(float radians)
{
	return 0; // TODO
}

inline bool EulerAngle::operator ==(const EulerAngle orient)
{
	auto orient1 = Clamp(*this);
	auto orient2 = Clamp(orient);
	return (orient1.x == orient2.x && orient1.y == orient2.y && orient1.z == orient2.z);
}

inline bool EulerAngle::operator !=(const EulerAngle orient)
{
	auto orient1 = Clamp(*this);
	auto orient2 = Clamp(orient);
	return (orient1.x != orient2.x || orient1.y != orient2.y || orient1.z != orient2.z);
}

inline EulerAngle EulerAngle::operator +(const EulerAngle orient)
{
	return Clamp(EulerAngle(x + orient.x, y + orient.y, z + orient.z));
}

inline EulerAngle EulerAngle::operator -(const EulerAngle orient)
{
	return Clamp(EulerAngle(x - orient.x, y - orient.y, z - orient.z));
}

inline EulerAngle EulerAngle::operator *(const EulerAngle orient)
{
	return Clamp(EulerAngle(x * orient.x, y * orient.y, z * orient.z));
}

inline EulerAngle EulerAngle::operator *(const float value)
{
	return Clamp(EulerAngle(x * value, y * value, z * value));
}

inline EulerAngle EulerAngle::operator /(const float value)
{
	return Clamp(EulerAngle(x / value, y / value, z / value));
}

inline EulerAngle& EulerAngle::operator +=(const EulerAngle orient)
{
	this->x += orient.x;
	this->y += orient.y;
	this->z += orient.z;
	this->Clamp();
	return *this;
}

inline EulerAngle& EulerAngle::operator -=(const EulerAngle orient)
{
	this->x -= orient.x;
	this->y -= orient.y;
	this->z -= orient.z;
	this->Clamp();
	return *this;
}

inline EulerAngle& EulerAngle::operator *=(const EulerAngle orient)
{
	this->x *= orient.x;
	this->y *= orient.y;
	this->z *= orient.z;
	this->Clamp();
	return *this;
}

inline EulerAngle& EulerAngle::operator *=(const float value)
{
	this->x *= value;
	this->y *= value;
	this->z *= value;
	this->Clamp();
	return *this;
}

inline EulerAngle& EulerAngle::operator /=(const float value)
{
	this->x /= value;
	this->y /= value;
	this->z /= value;
	this->Clamp();
	return *this;
}

inline EulerAngle::EulerAngle()
{
}

inline EulerAngle::EulerAngle(float xRadians, float yRadians, float zRadians) :
	Vector3(xRadians, yRadians, zRadians)
{
}

inline EulerAngle::EulerAngle(Vector3 orient) :
	Vector3(orient)
{
}
