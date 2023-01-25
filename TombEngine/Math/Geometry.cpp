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

	short GetShortestAngle(short fromAngle, short toAngle)
	{
		if (fromAngle == toAngle)
			return 0;

		return short(toAngle - fromAngle);
	}

	short GetSurfaceSteepnessAngle(Vector2 tilt)
	{
		static const short qtrBlockAngleIncrement = ANGLE(45.0f) / 4;

		return (short)sqrt(SQUARE(tilt.x * qtrBlockAngleIncrement) + SQUARE(tilt.y * qtrBlockAngleIncrement));
	}

	short GetSurfaceAspectAngle(Vector2 tilt)
	{
		if (tilt == Vector2::Zero)
			return 0;

		return FROM_RAD(atan2(-tilt.x, -tilt.y));
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
