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
	Vector3i TranslatePoint(const Vector3i& point, short headingAngle, float forward, float down, float right, const Vector3& axis)
	{
		return Vector3i(TranslatePoint(point.ToVector3(), headingAngle, forward, down, right, axis));
	}

	Vector3i TranslatePoint(const Vector3i& point, short headingAngle, const Vector3i& relOffset, const Vector3& axis)
	{
		return Vector3i(TranslatePoint(point.ToVector3(), headingAngle, relOffset.ToVector3(), axis));
	}

	Vector3i TranslatePoint(const Vector3i& point, const EulerAngles& orient, const Vector3i& relOffset)
	{
		return Vector3i(TranslatePoint(point.ToVector3(), orient, relOffset.ToVector3()));
	}

	Vector3i TranslatePoint(const Vector3i& point, const EulerAngles& orient, float dist)
	{
		return Vector3i(TranslatePoint(point.ToVector3(), orient, dist));
	}

	Vector3i TranslatePoint(const Vector3i& point, const AxisAngle& orient, float dist)
	{
		return Vector3i(TranslatePoint(point.ToVector3(), orient, dist));
	}

	Vector3i TranslatePoint(const Vector3i& point, const Vector3& dir, float dist)
	{
		return Vector3i(TranslatePoint(point.ToVector3(), dir, dist));
	}

	Vector3 TranslatePoint(const Vector3& point, short headingAngle, float forward, float down, float right, const Vector3& axis)
	{
		if (forward == 0.0f && down == 0.0f && right == 0.0f)
			return point;

		auto relOffset = Vector3(right, down, forward);
		return TranslatePoint(point, headingAngle, relOffset, axis);
	}

	Vector3 TranslatePoint(const Vector3& point, short headingAngle, const Vector3& relOffset, const Vector3& axis)
	{
		auto orient = AxisAngle(axis, headingAngle);
		auto rotMatrix = orient.ToRotationMatrix();
		return (point + Vector3::Transform(relOffset, rotMatrix));
	}

	Vector3 TranslatePoint(const Vector3& point, const EulerAngles& orient, const Vector3& relOffset)
	{
		auto rotMatrix = orient.ToRotationMatrix();
		return (point + Vector3::Transform(relOffset, rotMatrix));
	}

	// NOTE: Roll (Z axis) of EulerAngles orientation is disregarded.
	Vector3 TranslatePoint(const Vector3& point, const EulerAngles& orient, float dist)
	{
		if (dist == 0.0f)
			return point;

		auto dir = orient.ToDirection();
		return TranslatePoint(point, dir, dist);
	}

	Vector3 TranslatePoint(const Vector3& point, const AxisAngle& orient, float dist)
	{
		auto dir = orient.ToDirection();
		return TranslatePoint(point, dir, dist);
	}

	Vector3 TranslatePoint(const Vector3& point, const Vector3& dir, float dist)
	{
		if (dist == 0.0f)
			return point;

		auto normalizedDir = dir;
		normalizedDir.Normalize();
		return (point + (normalizedDir * dist));
	}

	Vector3 RotatePoint(const Vector3& point, const EulerAngles& rot)
	{
		auto rotMatrix = rot.ToRotationMatrix();
		return Vector3::Transform(point, rotMatrix);
	}

	Vector3 RotatePoint(const Vector3& point, const AxisAngle& rot)
	{
		auto rotMatrix = rot.ToRotationMatrix();
		return Vector3::Transform(point, rotMatrix);
	}

	short GetShortestAngle(short fromAngle, short toAngle)
	{
		if (fromAngle == toAngle)
			return 0;

		return short(toAngle - fromAngle);
	}

	short GetSurfaceSlopeAngle(const Vector3& normal, const Vector3& axis)
	{
		if (normal == -axis)
			return 0;

		return FROM_RAD(acos(normal.Dot(-axis)));
	}

	short GetSurfaceAspectAngle(const Vector3& normal, const Vector3& axis)
	{
		if (normal == -axis)
			return 0;

		// TODO: Consider axis.
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

		auto dir = linePoint1 - linePoint0;
		float distAlpha = dir.Dot(origin - linePoint0) / dir.Dot(dir);

		if (distAlpha < 0.0f)
		{
			return linePoint0;
		}
		else if (distAlpha > 1.0f)
		{
			return linePoint1;
		}

		return (linePoint0 + (dir * distAlpha));
	}

	EulerAngles GetOrientToPoint(const Vector3& origin, const Vector3& target)
	{
		if (origin == target)
			return EulerAngles::Zero;

		return EulerAngles(target - origin);
	}

	EulerAngles GetRelOrientToNormal(short orient, const Vector3& normal, const Vector3& gravity)
	{
		// TODO: Consider axis.

		// Determine relative angle properties of normal.
		short aspectAngle = Geometry::GetSurfaceAspectAngle(normal);
		short slopeAngle = Geometry::GetSurfaceSlopeAngle(normal);

		short deltaAngle = Geometry::GetShortestAngle(orient, aspectAngle);
		float sinDeltaAngle = phd_sin(deltaAngle);
		float cosDeltaAngle = phd_cos(deltaAngle);

		// Calculate relative orientation adhering to normal.
		return EulerAngles(
			-slopeAngle * cosDeltaAngle,
			orient,
			slopeAngle * sinDeltaAngle);
	}

	Quaternion ConvertDirectionToQuat(const Vector3& dir)
	{
		constexpr auto SINGULARITY_THRESHOLD = 1.0f - EPSILON;

		static const auto REF_DIR = Vector3::UnitZ;

		// Vectors are nearly opposite; return orientation 180 degrees around arbitrary axis.
		float dot = REF_DIR.Dot(dir);
		if (dot < -SINGULARITY_THRESHOLD)
		{
			auto axis = Vector3::UnitX.Cross(REF_DIR);
			if (axis.LengthSquared() < EPSILON)
				axis = Vector3::UnitY.Cross(REF_DIR);
			axis.Normalize();

			auto axisAngle = AxisAngle(axis, FROM_RAD(PI));
			return axisAngle.ToQuaternion();
		}

		// Vectors are nearly identical; return identity quaternion.
		if (dot > SINGULARITY_THRESHOLD)
			return Quaternion::Identity;

		// Calculate axis-angle and return converted quaternion.
		auto axisAngle = AxisAngle(REF_DIR.Cross(dir), FROM_RAD(acos(dot)));
		return axisAngle.ToQuaternion();
	}

	Vector3 ConvertQuatToDirection(const Quaternion& quat)
	{
		static const auto REF_DIR = Vector3::UnitZ;

		return Vector3::Transform(REF_DIR, quat);
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

		// 2D heading direction vector: X = +sinY, Y = 0, Z = +cosY
		auto headingDir = Vector3(sinY, 0.0f, cosY);
		auto targetDir = target - origin;

		float dot = headingDir.Dot(targetDir);
		if (dot > 0.0f)
			return true;

		return false;
	}

	bool IsPointInFront(const Vector3& origin, const Vector3& target, const Vector3& refPoint)
	{
		if (origin == target)
			return false;

		auto refDir = refPoint - origin;

		// 2D heading direction vector to 3D reference direction vector: X = +refDirection.x, Y = 0, Z = +refDirection.z
		auto headingDir = Vector3(refDir.x, 0.0f, refDir.z);
		auto targetDir = target - origin;

		float dot = headingDir.Dot(targetDir);
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

		// 2D normal vector to 2D heading direction vector: X = +cosY, Y = 0, Z = -sinY
		auto headingNormal = Vector3(cosY, 0.0f, -sinY);
		auto targetDir = target - origin;

		float dot = headingNormal.Dot(targetDir);
		if (dot > 0.0f)
			return true;

		return false;
	}

	bool IsPointOnLeft(const Vector3& origin, const Vector3& target, const Vector3& refPoint)
	{
		if (origin == target)
			return false;

		auto refDir = refPoint - origin;

		// 2D normal vector to 3D reference direction vector: X = +refDirection.z, Y = 0, Z = -refDirection.x
		auto headingNormal = Vector3(refDir.z, 0.0f, -refDir.x);
		auto targetDir = target - origin;

		float dot = headingNormal.Dot(targetDir);
		if (dot > 0.0f)
			return true;

		return false;
	}

	bool IsPointInBox(const Vector3& point, const BoundingBox& box)
	{
		// Calculate box-relative point.
		auto relPoint = box.Center - point;

		// Test if point intersects box.
		if (relPoint.x >= -box.Extents.x && relPoint.x <= box.Extents.x &&
			relPoint.y >= -box.Extents.y && relPoint.y <= box.Extents.y &&
			relPoint.z >= -box.Extents.z && relPoint.z <= box.Extents.z)
		{
			return true;
		}

		return false;
	}

	bool IsPointInBox(const Vector3& point, const BoundingOrientedBox& box)
	{
		// Calculate box-relative point.
		auto invRotMatrix = Matrix::CreateFromQuaternion(box.Orientation).Invert();
		auto relPoint = Vector3::Transform(box.Center - point, invRotMatrix);

		// Test if point intersects box.
		if (relPoint.x >= -box.Extents.x && relPoint.x <= box.Extents.x &&
			relPoint.y >= -box.Extents.y && relPoint.y <= box.Extents.y &&
			relPoint.z >= -box.Extents.z && relPoint.z <= box.Extents.z)
		{
			return true;
		}

		return false;
	}

	bool IsPointInSphere(const Vector3& point, const BoundingSphere& sphere)
	{
		float distSqr = Vector3::DistanceSquared(point, sphere.Center);
		float radiusSqr = SQUARE(sphere.Radius);

		return (distSqr <= radiusSqr);
	}
}
