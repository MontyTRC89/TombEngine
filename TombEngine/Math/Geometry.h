#pragma once

class EulerAngles;
class Pose;
class Vector3i;

namespace TEN::Math::Geometry
{
	// Since Y is assumed as the vertical axis, only the Y Euler component needs to be considered and
	// 2D vector operations can be done in the XZ plane. Maybe revise geometry functions to each take an "up" vector argument someday.

	Vector3i TranslatePoint(const Vector3i& point, short headingAngle, float forward, float down = 0.0f, float right = 0.0f);
	Vector3	 TranslatePoint(const Vector3& point, short headingAngle, float forward, float down = 0.0f, float right = 0.0f);
	Vector3i TranslatePoint(const Vector3i& point, const EulerAngles& orient, float distance);
	Vector3	 TranslatePoint(const Vector3& point, const EulerAngles& orient, float distance);
	Vector3i TranslatePoint(const Vector3i& point, const Vector3& direction, float distance);
	Vector3	 TranslatePoint(const Vector3& point, const Vector3& direction, float distance);

	short GetShortestAngle(short fromAngle, short toAngle);
	short GetSurfaceSteepnessAngle(Vector2 tilt);
	short GetSurfaceAspectAngle(Vector2 tilt);

	float		GetDistanceToLine(const Vector3& origin, const Vector3& linePoint0, const Vector3& linePoint1);
	Vector3		GetClosestPointOnLine(const Vector3& origin, const Vector3& linePoint0, const Vector3& linePoint1);
	EulerAngles GetOrientToPoint(const Vector3& origin, const Vector3& target);

	bool IsPointInFront(const Pose& pose, const Vector3& target);
	bool IsPointInFront(const Vector3& origin, const Vector3& target, const EulerAngles& orient);
	bool IsPointInFront(const Vector3& origin, const Vector3& target, const Vector3& refPoint);
	bool IsPointOnLeft(const Pose& pose, const Vector3& target);
	bool IsPointOnLeft(const Vector3& origin, const Vector3& target, const EulerAngles& orient);
	bool IsPointOnLeft(const Vector3& origin, const Vector3& target, const Vector3& refPoint);
}
