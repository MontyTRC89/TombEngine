#pragma once

struct PHD_3DPOS;
struct Vector3Int;
struct Vector3Shrt;

//namespace TEN::Math::Geometry
//{
	Vector3Int TranslatePoint(const Vector3Int& point, short angle, float forward, float down = 0.0f, float right = 0.0f);
	Vector3	   TranslatePoint(const Vector3& point, short angle, float forward, float down = 0.0f, float right = 0.0f);
	Vector3Int TranslatePoint(const Vector3Int& point, const Vector3Shrt& orient, float distance);
	Vector3	   TranslatePoint(const Vector3& point, const Vector3Shrt& orient, float distance);
	Vector3Int TranslatePoint(const Vector3Int& point, const Vector3& direction, float distance);
	Vector3	   TranslatePoint(const Vector3& point, const Vector3& direction, float distance);

	short GetShortestAngularDistance(short angleFrom, short angleTo);
	short GetSurfaceSteepnessAngle(Vector2 tilt);
	short GetSurfaceAspectAngle(Vector2 tilt);
	Vector3Shrt GetOrientTowardPoint(const Vector3& origin, const Vector3& target);

	// Since Y is assumed as the vertical axis, only the Y Euler component needs to be considered and
	// 2D vector operations can be done in the XZ plane. Maybe revise these to each take an up vector argument someday.

	bool IsPointInFront(const PHD_3DPOS& pose, const Vector3& target);
	bool IsPointInFront(const Vector3& origin, const Vector3& target, const Vector3Shrt& orient);
	bool IsPointInFront(const Vector3& origin, const Vector3& target, const Vector3& refPoint);
	bool IsPointOnLeft(const PHD_3DPOS& pose, const Vector3& target);
	bool IsPointOnLeft(const Vector3& origin, const Vector3& target, const Vector3Shrt& orient);
	bool IsPointOnLeft(const Vector3& origin, const Vector3& target, const Vector3& refPoint);
//}
