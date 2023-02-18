#include "framework.h"
#include "Math/Geometry.h"

#include "Math/Constants.h"
#include "Math/Legacy.h"
#include "Math/Objects/AxisAngle.h"
#include "Math/Objects/EulerAngles.h"
#include "Math/Objects/Pose.h"
#include "Math/Objects/Vector3i.h"

namespace TEN::Math::Geometry
{
	Vector3i TranslatePoint(const Vector3i& point, short headingAngle, float forward, float down, float right)
	{
		return Vector3i(TranslatePoint(point.ToVector3(), headingAngle, forward, down, right));
	}

	Vector3i TranslatePoint(const Vector3i& point, short headingAngle, const Vector3i& relOffset)
	{
		return Vector3i(TranslatePoint(point.ToVector3(), headingAngle, relOffset.ToVector3()));
	}

	Vector3i TranslatePoint(const Vector3i& point, const EulerAngles& orient, const Vector3i& relOffset)
	{
		return Vector3i(TranslatePoint(point.ToVector3(), orient, relOffset.ToVector3()));
	}

	Vector3i TranslatePoint(const Vector3i& point, const EulerAngles& orient, float distance)
	{
		return Vector3i(TranslatePoint(point.ToVector3(), orient, distance));
	}

	Vector3i TranslatePoint(const Vector3i& point, const Vector3& direction, float distance)
	{
		return Vector3i(TranslatePoint(point.ToVector3(), direction, distance));
	}

	Vector3 TranslatePoint(const Vector3& point, short headingAngle, float forward, float down, float right)
	{
		if (forward == 0.0f && down == 0.0f && right == 0.0f)
			return point;

		auto orient = EulerAngles(0, headingAngle, 0);
		auto relOffset = Vector3(right, down, forward);
		return TranslatePoint(point, orient, relOffset);
	}

	Vector3 TranslatePoint(const Vector3& point, short headingAngle, const Vector3& relOffset)
	{
		auto orient = EulerAngles(0, headingAngle, 0);
		return TranslatePoint(point, orient, relOffset);
	}

	Vector3 TranslatePoint(const Vector3& point, const EulerAngles& orient, const Vector3& relOffset)
	{
		auto rotMatrix = orient.ToRotationMatrix();
		return (point + Vector3::Transform(relOffset, rotMatrix));
	}

	// NOTE: Roll (Z axis) of EulerAngles orientation is disregarded.
	Vector3 TranslatePoint(const Vector3& point, const EulerAngles& orient, float distance)
	{
		if (distance == 0.0f)
			return point;

		auto direction = orient.ToDirection();
		return TranslatePoint(point, direction, distance);
	}

	Vector3 TranslatePoint(const Vector3& point, const Vector3& direction, float distance)
	{
		if (distance == 0.0f)
			return point;

		auto directionNorm = direction;
		directionNorm.Normalize();
		return (point + (directionNorm * distance));
	}

	Vector3 RotatePoint(const Vector3& point, const EulerAngles& rotation)
	{
		auto rotMatrix = rotation.ToRotationMatrix();
		return Vector3::Transform(point, rotMatrix);
	}

	Vector3 RotatePoint(const Vector3& point, const AxisAngle& rotation)
	{
		auto rotMatrix = rotation.ToRotationMatrix();
		return Vector3::Transform(point, rotMatrix);
	}

	Vector3 GetFloorNormal(const Vector2& tilt)
	{
		auto normal = Vector3(-tilt.x / 4, -1.0f, -tilt.y / 4);
		normal.Normalize();
		return normal;
	}

	Vector3 GetCeilingNormal(const Vector2& tilt)
	{
		auto normal = Vector3(tilt.x / 4, 1.0f, tilt.y / 4);
		normal.Normalize();
		return normal;
	}

	short GetShortestAngle(short fromAngle, short toAngle)
	{
		if (fromAngle == toAngle)
			return 0;

		return short(toAngle - fromAngle);
	}

	short GetSurfaceSlopeAngle(const Vector3& normal, const Vector3& force)
	{
		if (normal == -force)
			return 0;

		return FROM_RAD(acos(normal.Dot(-force)));
	}

	short GetSurfaceAspectAngle(const Vector3& normal, const Vector3& force)
	{
		if (normal == -force)
			return 0;

		// TODO: Consider normal of downward force.
		return FROM_RAD(atan2(normal.x, normal.z));
	}

	float GetDistanceToLine(const Vector3& origin, const Vector3& linePoint0, const Vector3& linePoint1)
	{
		auto target = GetClosestPointOnLine(origin, linePoint0, linePoint1);
		return Vector3::Distance(origin, target);
	}

	Vector3 GetClosestPointOnLine(const Vector3& origin, const Vector3& linePoint0, const Vector3& linePoint1)
	{
		if (linePoint0 == linePoint1)
			return linePoint0;

		auto direction = linePoint1 - linePoint0;
		float distanceAlpha = direction.Dot(origin - linePoint0) / direction.Dot(direction);

		if (distanceAlpha < 0.0f)
			return linePoint0;
		else if (distanceAlpha > 1.0f)
			return linePoint1;

		return (linePoint0 + (direction * distanceAlpha));
	}

	EulerAngles GetOrientToPoint(const Vector3& origin, const Vector3& target)
	{
		if (origin == target)
			return EulerAngles::Zero;

		return EulerAngles(target - origin);
	}

	EulerAngles GetRelOrientToNormal(short orient2D, const Vector3& normal, const Vector3& force)
	{
		// TODO: Consider normal of downward force.

		// Determine relative angle properties of normal.
		short aspectAngle = Geometry::GetSurfaceAspectAngle(normal);
		short slopeAngle = Geometry::GetSurfaceSlopeAngle(normal);

		short deltaAngle = Geometry::GetShortestAngle(orient2D, aspectAngle);
		float sinDeltaAngle = phd_sin(deltaAngle);
		float cosDeltaAngle = phd_cos(deltaAngle);

		// Calculate relative orientation adhering to normal.
		return EulerAngles(
			-slopeAngle * cosDeltaAngle,
			orient2D,
			slopeAngle * sinDeltaAngle);
	}

	Quaternion DirectionToQuaternion(const Vector3& direction)
	{
		auto directionNorm = direction;
		directionNorm.Normalize();

		auto axis = directionNorm.Cross(Vector3::Up);

		// Direction vectorand up vector are parallel, use X-axis as rotation axis instead.
		if (axis.LengthSquared() == 0.0f)
			axis = Vector3::Right;

		// Normalize axis.
		axis.Normalize();

		// Calculate rotation angle as angle between direction vector and up vector.
		float angle = acos(directionNorm.y);

		// Convert the axis-angle representation to a quaternion
		return Quaternion(axis, angle);
	}

	bool IsPointInFront(const Pose& pose, const Vector3& target)
	{
		return IsPointInFront(pose.Position.ToVector3(), target, pose.Orientation);
	}

	bool IsPointInFront(const Vector3& origin, const Vector3& target, const EulerAngles& orient)
	{
		if (origin == target)
			return false;

		float sinY = phd_sin(orient.y);
		float cosY = phd_cos(orient.y);

		// The 2D heading direction vector: X = +sinY, Y = 0, Z = +cosY
		auto headingDirection = Vector3(sinY, 0.0f, cosY);
		auto targetDirection = target - origin;

		float dot = headingDirection.Dot(targetDirection);
		if (dot > 0.0f)
			return true;

		return false;
	}

	bool IsPointInFront(const Vector3& origin, const Vector3& target, const Vector3& refPoint)
	{
		if (origin == target)
			return false;

		auto refDirection = refPoint - origin;

		// The 2D heading direction vector to the 3D reference direction vector: X = +refDirection.x, Y = 0, Z = +refDirection.z
		auto headingDirection = Vector3(refDirection.x, 0.0f, refDirection.z);
		auto targetDirection = target - origin;

		float dot = headingDirection.Dot(targetDirection);
		if (dot > 0.0f)
			return true;

		return false;
	}

	bool IsPointOnLeft(const Pose& pose, const Vector3& target)
	{
		return IsPointOnLeft(pose.Position.ToVector3(), target, pose.Orientation);
	}

	bool IsPointOnLeft(const Vector3& origin, const Vector3& target, const EulerAngles& orient)
	{
		if (origin == target)
			return false;

		float sinY = phd_sin(orient.y);
		float cosY = phd_cos(orient.y);

		// The 2D normal vector to the 2D heading direction vector: X = +cosY, Y = 0, Z = -sinY
		auto headingNormal = Vector3(cosY, 0.0f, -sinY);
		auto targetDirection = target - origin;

		float dot = headingNormal.Dot(targetDirection);
		if (dot > 0.0f)
			return true;

		return false;
	}

	bool IsPointOnLeft(const Vector3& origin, const Vector3& target, const Vector3& refPoint)
	{
		if (origin == target)
			return false;

		auto refDirection = refPoint - origin;

		// The 2D normal vector to the 3D reference direction vector: X = +refDirection.z, Y = 0, Z = -refDirection.x
		auto headingNormal = Vector3(refDirection.z, 0.0f, -refDirection.x);
		auto targetDirection = target - origin;

		float dot = headingNormal.Dot(targetDirection);
		if (dot > 0.0f)
			return true;

		return false;
	}
}
