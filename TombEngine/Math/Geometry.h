#pragma once

struct Vector3Int;
struct Vector3Shrt;

//namespace TEN::Math::Geometry
//{
	Vector3	   TranslatePoint(Vector3& point, short angle, float forward, float down = 0.0f, float right = 0.0f);
	Vector3Int TranslatePoint(Vector3Int& point, short angle, float forward, float down = 0.0f, float right = 0.0f);
	Vector3	   TranslatePoint(Vector3& point, Vector3Shrt& orient, float distance);
	Vector3Int TranslatePoint(Vector3Int& point, Vector3Shrt& orient, float distance);
	Vector3	   TranslatePoint(Vector3& point, Vector3& direction, float distance);
	Vector3Int TranslatePoint(Vector3Int& point, Vector3& direction, float distance);

	short		GetSurfaceSteepnessAngle(Vector2 tilt);
	short		GetSurfaceAspectAngle(Vector2 tilt);
	Vector3Shrt GetOrientTowardPoint(Vector3 origin, Vector3 target);
	
	bool IsPointOnLeft(Vector3 origin, Vector3 refPoint, Vector3 target);
	bool IsPointOnLeft(Vector3 origin, Vector3Shrt orient, Vector3 target);
//}
