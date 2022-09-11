#include "framework.h"
#include "Math/Geometry.h"

#include "Math/Containers/EulerAngles.h"
#include "Math/Containers/PoseData.h"
#include "Math/Containers/Vector3i.h"
#include "Math/Legacy.h"

namespace TEN::Math::Geometry
{
	Vector3i TranslatePoint(const Vector3i& point, short headingAngle, float forward, float down, float right)
	{
		return Vector3i(TranslatePoint(point.ToVector3(), headingAngle, forward, down, right));
	}

	Vector3 TranslatePoint(const Vector3& point, short headingAngle, float forward, float down, float right)
	{
		if (forward == 0.0f && down == 0.0f && right == 0.0f)
			return point;

		float sinHeading = phd_sin(headingAngle);
		float cosHeading = phd_cos(headingAngle);

		return Vector3(
			point.x + ((forward * sinHeading) + (right * cosHeading)),
			point.y + down,
			point.z + ((forward * cosHeading) - (right * sinHeading))
		);
	}

	Vector3i TranslatePoint(const Vector3i& point, const EulerAngles& orient, float distance)
	{
		return Vector3i(TranslatePoint(point.ToVector3(), orient, distance));
	}

	Vector3 TranslatePoint(const Vector3& point, const EulerAngles& orient, float distance)
	{
		if (distance == 0.0f)
			return point;

		float sinX = phd_sin(orient.x);
		float cosX = phd_cos(orient.x);
		float sinY = phd_sin(orient.y);
		float cosY = phd_cos(orient.y);

		return Vector3(
			point.x + (distance * (sinY * cosX)),
			point.y - (distance * sinX),
			point.z + (distance * (cosX * cosY))
		);
	}

	Vector3i TranslatePoint(const Vector3i& point, const Vector3& direction, float distance)
	{
		return Vector3i(TranslatePoint(point.ToVector3(), direction, distance));
	}

	Vector3 TranslatePoint(const Vector3& point, const Vector3& direction, float distance)
	{
		auto directionNorm = direction;
		directionNorm.Normalize();
		return (point + (directionNorm * distance));
	}

	short GetShortestAngularDistance(short angleFrom, short angleTo)
	{
		return short(angleTo - angleFrom);
	}

	short GetSurfaceSteepnessAngle(Vector2 tilt)
	{
		static const short qtrBlockAngleIncrement = ANGLE(45.0f) / 4;

		return (short)sqrt(pow(tilt.x * qtrBlockAngleIncrement, 2) + pow(tilt.y * qtrBlockAngleIncrement, 2));
	}

	short GetSurfaceAspectAngle(Vector2 tilt)
	{
		return (short)phd_atan(-tilt.y, -tilt.x);
	}

	EulerAngles GetOrientTowardPoint(const Vector3& origin, const Vector3& target)
	{
		auto direction = target - origin;
		float yRad = atan2(direction.x, direction.z);

		auto vector = direction;
		auto matrix = Matrix::CreateRotationY(-yRad);
		Vector3::Transform(vector, matrix, vector);

		return EulerAngles(
			FROM_RAD(-atan2(direction.y, vector.z)),
			FROM_RAD(yRad),
			0
		);
	}

	bool IsPointInFront(const PoseData& pose, const Vector3& target)
	{
		return IsPointInFront(pose.Position.ToVector3(), target, pose.Orientation);
	}
	
	bool IsPointInFront(const Vector3& origin, const Vector3& target, const EulerAngles& orient)
	{
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
		auto refDirection = refPoint - origin;

		// The 2D heading direction vector to the 3D reference direction vector: X = +refDirection.x, Y = 0, Z = +refDirection.z
		auto headingDirection = Vector3(refDirection.x, 0.0f, refDirection.z);
		auto targetDirection = target - origin;

		float dot = headingDirection.Dot(targetDirection);
		if (dot > 0.0f)
			return true;

		return false;
	}

	bool IsPointOnLeft(const PoseData& pose, const Vector3& target)
	{
		return IsPointOnLeft(pose.Position.ToVector3(), target, pose.Orientation);
	}

	bool IsPointOnLeft(const Vector3& origin, const Vector3& target, const EulerAngles& orient)
	{
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
