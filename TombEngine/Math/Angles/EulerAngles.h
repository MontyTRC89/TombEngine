#pragma once
#include "Math/Angles/Angle.h"

//namespace TEN::Math::Angles
//{
	class EulerAngles
	{
	public:
		// Normalized radian components
		Angle x = 0.0f;
		Angle y = 0.0f;
		Angle z = 0.0f;

		static const EulerAngles Zero;

		// Constructors
		EulerAngles();
		EulerAngles(float xAngle, float yAngle, float zAngle);
		EulerAngles(Vector3 directionVector);

		// Utilities
		void Normalize();
		static EulerAngles Normalize(EulerAngles orient);

		bool Compare(EulerAngles orient, float epsilon = 0.0f);
		static bool Compare(EulerAngles orient0, EulerAngles orient1, float epsilon = 0.0f);

		EulerAngles ShortestAngularDistance(EulerAngles orientTo);
		static EulerAngles ShortestAngularDistance(EulerAngles orientFrom, EulerAngles orientTo);

		void InterpolateLinear(EulerAngles orientTo, float alpha = 1.0f, float epsilon = 0.0f);
		static EulerAngles InterpolateLinear(EulerAngles orientFrom, EulerAngles orientTo, float alpha = 1.0f, float epsilon = 0.0f);

		void InterpolateConstant(EulerAngles orientTo, float rate);
		static EulerAngles InterpolateConstant(EulerAngles orientFrom, EulerAngles orientTo, float rate);

		void InterpolateConstantEaseOut(EulerAngles orientTo, float rate, float alpha = 1.0f, float epsilon = 0.0f);
		static EulerAngles InterpolateConstantEaseOut(EulerAngles orientFrom, EulerAngles orientTo, float rate, float alpha = 1.0f, float epsilon = 0.0f);

		static EulerAngles OrientBetweenPoints(Vector3 origin, Vector3 target);

		Vector3 GetDirectionVector();

		// TODO: Remove.
		void SetY(float angle = 0.0f);

		// Operators
		bool		 operator ==(EulerAngles orient);
		bool		 operator !=(EulerAngles orient);
		EulerAngles  operator +(EulerAngles orient);
		EulerAngles  operator -(EulerAngles orient);
		EulerAngles  operator *(EulerAngles orient);
		EulerAngles  operator *(float value);
		EulerAngles	 operator /(float value);
		EulerAngles& operator =(EulerAngles orient);
		EulerAngles& operator +=(EulerAngles orient);
		EulerAngles& operator -=(EulerAngles orient);
		EulerAngles& operator *=(EulerAngles orient);
		EulerAngles& operator *=(float value);
		EulerAngles& operator /=(float value);
	};
//}
