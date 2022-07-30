#pragma once

// TODOs:
// - Strong typedef for Angle class component (use this library? https://www.foonathan.net/2016/10/strong-typedefs/).
// - Make non-static method variations to easily manipulate class instances.
// - Make TEN::Math::Angles namespace for all angle-related classes and potential utility functions.

class Angle
{
private:
	float Component;

public:
	// Utilities
	static float Normalize(float angle);
	static bool  Compare(float angle0, float angle1, float epsilon = 0.0f);
	static float ShortestAngularDistance(float angleFrom, float angleTo);
	static float InterpolateLinear(float angleFrom, float angleTo, float alpha = 1.0f, float epsilon = 0.0f);
	static float InterpolateConstant(float angleFrom, float angleTo, float rate);
	static float InterpolateConstantEaseOut(float angleFrom, float angleTo, float rate, float alpha = 1.0f, float epsilon = 0.0f);
	static float OrientBetweenPoints(Vector3 point0, Vector3 point1);

	static float DeltaHeading(Vector3 origin, Vector3 target, float heading); // TODO: I don't even know what this does.

	// Converters
	static float DegToRad(float degrees);
	static float RadToDeg(float radians);

	// Temporary legacy short angle form support for particularly cryptic code
	static float ShrtToRad(short shortForm);
	static short DegToShrt(float degrees);
	static short RadToShrt(float radians);

private:
	// Utilities
	static float ClampAlpha(float value);
	static float ClampEpsilon(float value);
};
