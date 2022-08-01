#pragma once
#include "Math/Angles/Angle.h"

//namespace TEN::Math::Angles
//{
	class EulerAngles
	{
	public:
		// TODO: Remove.
		void SetY(float angle = 0.0f) {};

		// Normalized angle components (stored as radians)
		Angle x = 0.0f;
		Angle y = 0.0f;
		Angle z = 0.0f;

		static const EulerAngles Zero;

		// Constructors
		EulerAngles();
		EulerAngles(float xAngle, float yAngle, float zAngle);
		EulerAngles(Vector3 directionVector);

		// Utilities
		static bool Compare(EulerAngles euler0, EulerAngles euler1, float epsilon = 0.0f);
		static EulerAngles GetShortestAngularDistance(EulerAngles eulerFrom, EulerAngles eulerTo);
		Vector3 GetDirectionVector();

		void InterpolateLinear(EulerAngles eulerTo, float alpha = 1.0f, float epsilon = 0.0f);
		static EulerAngles InterpolateLinear(EulerAngles eulerFrom, EulerAngles eulerTo, float alpha = 1.0f, float epsilon = 0.0f);

		void InterpolateConstant(EulerAngles eulerTo, float rate);
		static EulerAngles InterpolateConstant(EulerAngles eulerFrom, EulerAngles eulerTo, float rate);

		void InterpolateConstantEaseOut(EulerAngles eulerTo, float rate, float alpha = 1.0f, float epsilon = 0.0f);
		static EulerAngles InterpolateConstantEaseOut(EulerAngles eulerFrom, EulerAngles eulerTo, float rate, float alpha = 1.0f, float epsilon = 0.0f);

		// Operators
		bool		 operator ==(EulerAngles euler);
		bool		 operator !=(EulerAngles euler);
		EulerAngles  operator +(EulerAngles euler);
		EulerAngles  operator -(EulerAngles euler);
		EulerAngles  operator *(EulerAngles euler);
		EulerAngles  operator *(float value);
		EulerAngles	 operator /(float value);
		EulerAngles& operator =(EulerAngles euler);
		EulerAngles& operator +=(EulerAngles euler);
		EulerAngles& operator -=(EulerAngles euler);
		EulerAngles& operator *=(EulerAngles euler);
		EulerAngles& operator *=(float value);
		EulerAngles& operator /=(float value);
	};
//}
