#include "framework.h"
#include "Math/Angles/Angle.h"

#include "Specific/trmath.h"

//namespace TEN::Math::Angles
//{
	Angle::Angle(float radians)
	{
		this->Value = Normalize(radians);
	}

	Angle Angle::FromDeg(float degrees)
	{
		return Angle(Normalize(DegToRad(degrees)));
	}

	void Angle::Compare(Angle angle, float epsilon)
	{
		this->Value = Compare(*this, angle, epsilon);
	}

	bool Angle::Compare(float angle0, float angle1, float epsilon)
	{
		epsilon = ClampEpsilon(epsilon);

		float difference = GetShortestAngularDistance(angle0, angle1);
		if (abs(difference) <= epsilon)
			return true;

		return false;
	}

	void Angle::GetShortestAngularDistance(Angle angleTo)
	{
		this->Value = GetShortestAngularDistance(*this, angleTo);
	}
	
	float Angle::GetShortestAngularDistance(float angleFrom, float angleTo)
	{
		return Normalize(angleTo - angleFrom);
	}

	void Angle::InterpolateLinear(Angle angleTo, float alpha, float epsilon)
	{
		this->Value = InterpolateLinear(*this, angleTo, alpha, epsilon);
	}
	
	float Angle::InterpolateLinear(float angleFrom, float angleTo, float alpha, float epsilon)
	{
		alpha = ClampAlpha(alpha);
		epsilon = ClampEpsilon(epsilon);

		if (!Compare(angleFrom, angleTo, epsilon))
		{
			float difference = GetShortestAngularDistance(angleFrom, angleTo);
			return (angleFrom + (difference * alpha));
		}

		return angleTo;
	}

	void Angle::InterpolateConstant(Angle angleTo, float rate)
	{
		this->Value = InterpolateConstant(*this, angleTo, rate);
	}
	
	float Angle::InterpolateConstant(float angleFrom, float angleTo, float rate)
	{
		rate = ClampEpsilon(rate);

		if (!Compare(angleFrom, angleTo, rate))
		{
			int sign = copysign(1, GetShortestAngularDistance(angleFrom, angleTo));
			return (angleFrom + (rate * sign));
		}

		return angleTo;
	}

	void Angle::InterpolateConstantEaseOut(Angle angleTo, float rate, float alpha, float epsilon)
	{
		this->Value = InterpolateConstantEaseOut(*this, angleTo, rate, alpha, epsilon);
	}

	float Angle::InterpolateConstantEaseOut(float angleFrom, float angleTo, float rate, float alpha, float epsilon)
	{
		rate = ClampEpsilon(rate);
		alpha = ClampAlpha(alpha);
		epsilon = ClampEpsilon(epsilon);

		if (!Compare(angleFrom, angleTo, epsilon))
		{
			if (!Compare(angleFrom, angleTo, rate))
			{
				int sign = copysign(1, GetShortestAngularDistance(angleFrom, angleTo));
				return (angleFrom + (rate * sign));
			}
			else
			{
				float difference = GetShortestAngularDistance(angleFrom, angleTo);
				return (angleFrom + (difference * alpha));
			}
		}

		return angleTo;
	}

	float Angle::DegToRad(float degrees)
	{
		return (degrees * RADIAN);
	}

	float Angle::RadToDeg(float radians)
	{
		return fmod((radians * (180.0f / PI)) + 360.0f, 360.0f);
	}

	float Angle::ShrtToRad(short shortForm)
	{
		return (shortForm * ((360.0f / (USHRT_MAX + 1)) * RADIAN));
	}

	short Angle::DegToShrt(float degrees)
	{
		return (short)round(degrees * ((USHRT_MAX + 1) / 360.0f));
	}

	short Angle::RadToShrt(float radians)
	{
		return (short)round((radians / RADIAN) * ((USHRT_MAX + 1) / 360.0f));
	}

	Angle::operator float() const
	{
		return Value;
	}

	bool Angle::operator ==(float value)
	{
		return (Value == value);
	}
	
	bool Angle::operator !=(float value)
	{
		return (Value != value);
	}

	/*bool Angle::operator >=(float value)
	{
		return (RadToDeg(Value) >= Normalize(value));
	}
	
	bool Angle::operator <=(float value)
	{
		return (RadToDeg(Value) <= Normalize(value));
	}
	
	bool Angle::operator >(float value)
	{
		return (RadToDeg(Value) > Normalize(value));
	}
	
	bool Angle::operator <(float value)
	{
		return (RadToDeg(Value) < Normalize(value));
	}*/
	
	Angle Angle::operator +(float value)
	{
		return Angle(Value + value);
	}
	
	Angle Angle::operator -(float value)
	{
		return Angle(Value - value);
	}
	
	Angle Angle::operator *(float value)
	{
		return Angle(Value * value);
	}
	
	Angle Angle::operator *(int value)
	{
		return Angle(Value * value);
	}
	
	Angle Angle::operator /(float value)
	{
		return Angle(Value / value);
	}
	
	Angle Angle::operator /(int value)
	{
		return Angle(Value / value);
	}

	Angle& Angle::operator =(float value)
	{
		this->Value = value;
		this->Normalize();
		return *this;
	}
	
	Angle& Angle::operator +=(float value)
	{
		this->Value += value;
		this->Normalize();
		return *this;
	}
	
	Angle& Angle::operator -=(float value)
	{
		this->Value -= value;
		this->Normalize();
		return *this;
	}
	
	Angle& Angle::operator *=(float value)
	{
		this->Value *= value;
		this->Normalize();
		return *this;
	}
	
	Angle& Angle::operator *=(int value)
	{
		this->Value *= value;
		this->Normalize();
		return *this;
	}
	
	Angle& Angle::operator /=(float value)
	{
		this->Value /= value;
		this->Normalize();
		return *this;
	}
	
	Angle& Angle::operator /=(int value)
	{
		this->Value *= value;
		this->Normalize();
		return *this;
	}

	void Angle::Normalize()
	{
		this->Value = Normalize(Value);
	}

	float Angle::Normalize(float radians)
	{
		if (radians < -PI || radians > PI)
			return atan2(sin(radians), cos(radians));

		return radians;

		// Alternative (faster?) method:
		/*return (radians > 0) ?
			fmod(radians + PI, PI * 2) - PI :
			fmod(radians - PI, PI * 2) + PI;*/
	}

	float Angle::ClampAlpha(float alpha)
	{
		return ((abs(alpha) > 1.0f) ? 1.0f : abs(alpha));
	}

	float Angle::ClampEpsilon(float epsilon)
	{
		return abs(Normalize(epsilon));
	}
//}
