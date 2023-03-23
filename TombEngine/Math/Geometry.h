#pragma once

class AxisAngle;
class EulerAngles;
class Pose;
class Vector3i;

namespace TEN::Math::Geometry
{
	// Since Y is assumed as the vertical axis, 2D operations are simply done in the XZ plane.
	// Revise geometry functions to each take a "force" direction argument someday. -- Sezz 2023.01.26

	// Integer-based point translation
	Vector3i TranslatePoint(const Vector3i& point, short headingAngle, float forward, float down = 0.0f, float right = 0.0f);
	Vector3i TranslatePoint(const Vector3i& point, short headingAngle, const Vector3i& relOffset);
	Vector3i TranslatePoint(const Vector3i& point, const EulerAngles& orient, const Vector3i& relOffset);
	Vector3i TranslatePoint(const Vector3i& point, const EulerAngles& orient, float distance);
	Vector3i TranslatePoint(const Vector3i& point, const AxisAngle& orient, float distance);
	Vector3i TranslatePoint(const Vector3i& point, const Vector3& direction, float distance);

	// Float-based point translation
	Vector3 TranslatePoint(const Vector3& point, short headingAngle, float forward, float down = 0.0f, float right = 0.0f);
	Vector3 TranslatePoint(const Vector3& point, short headingAngle, const Vector3& relOffset);
	Vector3 TranslatePoint(const Vector3& point, const EulerAngles& orient, const Vector3& relOffset);
	Vector3 TranslatePoint(const Vector3& point, const EulerAngles& orient, float distance);
	Vector3 TranslatePoint(const Vector3& point, const AxisAngle& orient, float distance);
	Vector3 TranslatePoint(const Vector3& point, const Vector3& direction, float distance);

	// Rotation
	Vector3 RotatePoint(const Vector3& point, const EulerAngles& rotation);
	Vector3 RotatePoint(const Vector3& point, const AxisAngle& rotation);

	// Surface normal getters
	Vector3 GetFloorNormal(const Vector2& tilt);
	Vector3 GetCeilingNormal(const Vector2& tilt);

	// Angle getters
	short GetShortestAngle(short fromAngle, short toAngle);
	short GetSurfaceSlopeAngle(const Vector3& normal, const Vector3& force = Vector3::Up);	// Up = Down.
	short GetSurfaceAspectAngle(const Vector3& normal, const Vector3& force = Vector3::Up); // Up = Down.

	// Misc. getters
	float		GetDistanceToLine(const Vector3& origin, const Vector3& linePoint0, const Vector3& linePoint1);
	Vector3		GetClosestPointOnLine(const Vector3& origin, const Vector3& linePoint0, const Vector3& linePoint1);
	Quaternion	GetQuaternionFromDirection(const Vector3& direction, const Vector3& refDirection);
	EulerAngles GetOrientToPoint(const Vector3& origin, const Vector3& target);
	EulerAngles GetRelOrientToNormal(short orient2D, const Vector3& normal, const Vector3& force = Vector3::Up); // Up = Down.

	// Point relation inquirers
	bool IsPointInFront(const Pose& pose, const Vector3& target);
	bool IsPointInFront(const Vector3& origin, const Vector3& target, const EulerAngles& orient);
	bool IsPointInFront(const Vector3& origin, const Vector3& target, const Vector3& refPoint);
	bool IsPointOnLeft(const Pose& pose, const Vector3& target);
	bool IsPointOnLeft(const Vector3& origin, const Vector3& target, const EulerAngles& orient);
	bool IsPointOnLeft(const Vector3& origin, const Vector3& target, const Vector3& refPoint);
}
