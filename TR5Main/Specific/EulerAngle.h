#pragma once
#include "framework.h"
#include "Specific/trmath.h"

class EulerAngle : public Vector3
{
public:
	Vector3 ToVector3();
	float GetX();
	float GetY();
	float GetZ();

	void Set(EulerAngle orient);
	void Set(float xRadians, float yRadians, float zRadians);
	void SetX(float radians);
	void SetY(float radians);
	void SetZ(float radians);

	void Clamp();
	static EulerAngle Clamp(EulerAngle orient);
	static float Clamp(float radians);

	void Interpolate(EulerAngle targetOrient, float rate, float threshold);
	static EulerAngle Interpolate(EulerAngle orientFrom, EulerAngle orientTo, float rate, float threshold);
	static float Interpolate(float radiansFrom, float radiansTo, float rate, float threshold);

	static EulerAngle ShortestAngle(EulerAngle orientFrom, EulerAngle orientTo);
	static float ShortestAngle(float radiansFrom, float radiansTo);

	static float DegToRad(float degrees);
	static float RadToDeg(float radians);
	static float ShrtToRad(short shortForm); // Temporary legacy short form support for particularly cryptic code.
	static short DegToShrt(float degrees);
	static short RadToShrt(float radians);

	bool operator ==(EulerAngle orient);
	bool operator !=(EulerAngle orient);
	EulerAngle operator +(EulerAngle orient);
	EulerAngle operator -(EulerAngle orient);
	EulerAngle operator *(EulerAngle orient);
	EulerAngle operator *(float value);
	EulerAngle operator /(float value);
	EulerAngle& operator +=(EulerAngle orient);
	EulerAngle& operator -=(EulerAngle orient);
	EulerAngle& operator *=(EulerAngle orient);
	EulerAngle& operator *=(float value);
	EulerAngle& operator /=(float value);

	EulerAngle();
	EulerAngle(float xRadians, float yRadians, float zRadians);

private:
	EulerAngle(Vector3 orient);

	// TODO: Due to clamping requirements, the components must be made private?
	//using Vector3::x;
	//using Vector3::y;
	//using Vector3::z;
};

inline Vector3 EulerAngle::ToVector3()
{
	return Vector3(this->GetX(), this->GetY(), this->GetZ());
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
	*this = Clamp(orient);
}

inline void EulerAngle::Set(float xRadians, float yRadians, float zRadians)
{
	this->SetX(xRadians);
	this->SetY(yRadians);
	this->SetZ(zRadians);
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
	this->SetX(Clamp(this->GetX()));
	this->SetY(Clamp(this->GetY()));
	this->SetZ(Clamp(this->GetZ()));
}

inline EulerAngle EulerAngle::Clamp(EulerAngle orient)
{
	orient.Clamp();
	return orient;
}

inline float EulerAngle::Clamp(float radians)
{
	return atan2(sin(radians), cos(radians));

	// Alternative method:
	/*return (radians > 0) ?
		fmod(radians + M_PI, M_PI * 2) - M_PI :
		fmod(radians - M_PI, M_PI * 2) + M_PI;*/
}

inline void EulerAngle::Interpolate(EulerAngle targetOrient, float rate = 1.0f, float threshold = 0)
{
	*this = Interpolate(*this, targetOrient, rate, threshold);
}

inline EulerAngle EulerAngle::Interpolate(EulerAngle orientFrom, EulerAngle orientTo, float rate = 1.0f, float threshold = 0)
{
	return EulerAngle(
		Interpolate(orientFrom.GetX(), orientTo.GetX(), rate, threshold),
		Interpolate(orientFrom.GetY(), orientTo.GetY(), rate, threshold),
		Interpolate(orientFrom.GetZ(), orientTo.GetZ(), rate, threshold)
	);
}

inline float EulerAngle::Interpolate(float radiansFrom, float radiansTo, float rate = 1.0f, float threshold = 0)
{
	rate = (abs(rate) > 1.0f) ? 1.0f : abs(rate);
	
	float difference = ShortestAngle(radiansFrom, radiansTo);
	if (abs(difference) > threshold)
		return Clamp(radiansFrom + (difference * rate));
	else
		return Clamp(radiansTo);
}

inline EulerAngle EulerAngle::ShortestAngle(EulerAngle orientFrom, EulerAngle orientTo)
{
	return (orientTo - orientFrom);
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

inline short EulerAngle::DegToShrt(float degrees)
{
	return (degrees * ((USHRT_MAX + 1) / 360.0f));
}

inline short EulerAngle::RadToShrt(float radians)
{
	return ((radians / (180.0f / M_PI)) * ((USHRT_MAX + 1) / 360.0f));
}

inline bool EulerAngle::operator ==(EulerAngle orient)
{
	auto orient1 = Clamp(*this);
	auto orient2 = Clamp(orient);
	return (orient1.GetX() == orient2.GetX() && orient1.GetY() == orient2.GetY() && orient1.GetZ() == orient2.GetZ());
}

inline bool EulerAngle::operator !=(EulerAngle orient)
{
	auto orient1 = Clamp(*this);
	auto orient2 = Clamp(orient);
	return (orient1.GetX() != orient2.GetX() || orient1.GetY() != orient2.GetY() || orient1.GetZ() != orient2.GetZ());
}

inline EulerAngle EulerAngle::operator +(EulerAngle orient)
{
	return Clamp(EulerAngle(this->GetX() + orient.GetX(), this->GetY() + orient.GetY(), this->GetZ() + orient.GetZ()));
}

inline EulerAngle EulerAngle::operator -(EulerAngle orient)
{
	return Clamp(EulerAngle(this->GetX() - orient.GetX(), this->GetY() - orient.GetY(), this->GetZ() - orient.GetZ()));
}

inline EulerAngle EulerAngle::operator *(EulerAngle orient)
{
	return Clamp(EulerAngle(this->GetX() * orient.GetX(), this->GetY() * orient.GetY(), this->GetZ() * orient.GetZ()));
}

inline EulerAngle EulerAngle::operator *(float value)
{
	return Clamp(EulerAngle(this->GetX() * value, this->GetY() * value, this->GetZ() * value));
}

inline EulerAngle EulerAngle::operator /(float value)
{
	return Clamp(EulerAngle(this->GetX() / value, this->GetY() / value, this->GetZ() / value));
}

inline EulerAngle& EulerAngle::operator +=(EulerAngle orient)
{
	this->SetX(this->GetX() + orient.GetX());
	this->SetY(this->GetY() + orient.GetY());
	this->SetZ(this->GetZ() + orient.GetZ());
	this->Clamp();
	return *this;
}

inline EulerAngle& EulerAngle::operator -=(EulerAngle orient)
{
	this->SetX(this->GetX() - orient.GetX());
	this->SetY(this->GetY() - orient.GetY());
	this->SetZ(this->GetZ() - orient.GetZ());
	this->Clamp();
	return *this;
}

inline EulerAngle& EulerAngle::operator *=(EulerAngle orient)
{
	this->SetX(this->GetX() * orient.GetX());
	this->SetY(this->GetY() * orient.GetY());
	this->SetZ(this->GetZ() * orient.GetZ());
	this->Clamp();
	return *this;
}

inline EulerAngle& EulerAngle::operator *=(float value)
{
	this->SetX(this->GetX() * value);
	this->SetY(this->GetY() * value);
	this->SetZ(this->GetZ() * value);
	this->Clamp();
	return *this;
}

inline EulerAngle& EulerAngle::operator /=(float value)
{
	this->SetX(this->GetX() / value);
	this->SetY(this->GetY() / value);
	this->SetZ(this->GetZ() / value);
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
