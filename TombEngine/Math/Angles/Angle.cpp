#include "framework.h"
#include "Math/Angles/Angle.h"

#include "Specific/trmath.h"

//namespace TEN::Math::Angles
//{
	Angle::Angle(float radians)
	{
		this->Component = Normalize(radians);
	}

	void Angle::Normalize()
	{
		this->Component = Normalize(Component);
	}

	float Angle::Normalize(float angle)
	{
		if (angle < -PI || angle > PI)
			return atan2(sin(angle), cos(angle));

		return angle;

		// Alternative (faster?) method:
		/*return (angle > 0) ?
			fmod(angle + PI, PI * 2) - PI :
			fmod(angle - PI, PI * 2) + PI;*/
	}

	bool Angle::Compare(float angle0, float angle1, float epsilon)
	{
		epsilon = ClampEpsilon(epsilon);

		float difference = ShortestAngularDistance(angle0, angle1);
		if (abs(difference) <= epsilon)
			return true;

		return false;
	}

	float Angle::ShortestAngularDistance(float angleFrom, float angleTo)
	{
		return Normalize(angleTo - angleFrom);
	}

	float Angle::InterpolateLinear(float angleFrom, float angleTo, float alpha, float epsilon)
	{
		alpha = ClampAlpha(alpha);
		epsilon = ClampEpsilon(epsilon);

		if (!Compare(angleFrom, angleTo, epsilon))
		{
			float difference = ShortestAngularDistance(angleFrom, angleTo);
			return (angleFrom + (difference * alpha));
		}

		return angleTo;
	}

	float Angle::InterpolateConstant(float angleFrom, float angleTo, float rate)
	{
		rate = ClampEpsilon(rate);

		if (!Compare(angleFrom, angleTo, rate))
		{
			int sign = copysign(1, ShortestAngularDistance(angleFrom, angleTo));
			return (angleFrom + (rate * sign));
		}

		return angleTo;
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
				int sign = copysign(1, ShortestAngularDistance(angleFrom, angleTo));
				return (angleFrom + (rate * sign));
			}
			else
			{
				float difference = ShortestAngularDistance(angleFrom, angleTo);
				return (angleFrom + (difference * alpha));
			}
		}

		return angleTo;
	}

	float Angle::OrientBetweenPoints(Vector3 point0, Vector3 point1)
	{
		auto direction = point1 - point0;
		return atan2(direction.x, direction.z);
	}

	float Angle::DeltaHeading(Vector3 origin, Vector3 target, float heading)
	{
		float difference = OrientBetweenPoints(origin, target);
		return ShortestAngularDistance(heading, difference + DegToRad(90.0f));
	}

	float Angle::DegToRad(float degrees)
	{
		return Normalize(degrees * (PI / 180.0f)); // dont normalise
	}

	float Angle::RadToDeg(float radians)
	{
		return fmod((radians * (180.0f / PI)) + 360.0f, 360.0f);
	}

	float Angle::ShrtToRad(short shortForm)
	{
		return Normalize(shortForm * ((360.0f / (USHRT_MAX + 1)) * (PI / 180.0f))); // dont normalise
	}

	short Angle::DegToShrt(float degrees)
	{
		return (short)round(degrees * ((USHRT_MAX + 1) / 360.0f));
	}

	short Angle::RadToShrt(float radians)
	{
		return (short)round((radians / (PI / 180.0f)) * ((USHRT_MAX + 1) / 360.0f));
	}

	Angle::operator float() const
	{
		return Component;
	}

	Angle Angle::operator ==(float value)
	{
		return (Component == value);
	}
	
	Angle Angle::operator !=(float value)
	{
		return (Component != value);
	}
	
	Angle Angle::operator +(float value)
	{
		return Angle(Component + value);
	}
	
	Angle Angle::operator -(float value)
	{
		return Angle(Component - value);
	}
	
	Angle Angle::operator *(float value)
	{
		return Angle(Component * value);
	}
	
	Angle Angle::operator *(int value)
	{
		return Angle(Component * value);
	}
	
	Angle Angle::operator /(float value)
	{
		return Angle(Component / value);
	}
	
	Angle Angle::operator /(int value)
	{
		return Angle(Component / value);
	}

	Angle& Angle::operator =(float value)
	{
		this->Component = value;
		this->Normalize();
		return *this;
	}
	
	Angle& Angle::operator +=(float value)
	{
		this->Component += value;
		this->Normalize();
		return *this;
	}
	
	Angle& Angle::operator -=(float value)
	{
		this->Component -= value;
		this->Normalize();
		return *this;
	}
	
	Angle& Angle::operator *=(float value)
	{
		this->Component *= value;
		this->Normalize();
		return *this;
	}
	
	Angle& Angle::operator *=(int value)
	{
		this->Component *= value;
		this->Normalize();
		return *this;
	}
	
	Angle& Angle::operator /=(float value)
	{
		this->Component /= value;
		this->Normalize();
		return *this;
	}
	
	Angle& Angle::operator /=(int value)
	{
		this->Component *= value;
		this->Normalize();
		return *this;
	}

	float Angle::ClampAlpha(float value)
	{
		return ((abs(value) > 1.0f) ? 1.0f : abs(value));
	}

	float Angle::ClampEpsilon(float value)
	{
		return abs(Normalize(value));
	}
//}
