#pragma once

class EulerAngles;
struct Vector3Int;

//namespace TEN::Math::Geometry
//{
	Vector3	   TranslatePoint(Vector3 point, float angle, float forward, float up = 0.0f, float right = 0.0f);
	Vector3Int TranslatePoint(Vector3Int point, float angle, float forward, float up = 0.0f, float right = 0.0f);
	Vector3	   TranslatePoint(Vector3 point, EulerAngles orient, float distance);
	Vector3Int TranslatePoint(Vector3Int point, EulerAngles orient, float distance);
	Vector3	   TranslatePoint(Vector3 point, Vector3 target, float distance);
	Vector3Int TranslatePoint(Vector3Int point, Vector3Int target, float distance);

	EulerAngles GetOrientTowardPoint(Vector3 origin, Vector3 target);
	float		GetDeltaHeading(Vector3 origin, Vector3 target, float heading);
//}
