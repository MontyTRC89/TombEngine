#include "framework.h"
#include "Math/Geometry.h"

#include "Math/Containers/PoseData.h"
#include "Math/Containers/Vector3i.h"
#include "Math/Containers/Vector3s.h"
#include "Specific/trmath.h"

//namespace TEN::Math::Geometry
//{
	Vector3Int TranslatePoint(const Vector3Int& point, short angle, float forward, float down, float right)
	{
		return Vector3Int(TranslatePoint(point.ToVector3(), angle, forward, down, right));
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

	Vector3Int TranslatePoint(const Vector3Int& point, const Vector3Shrt& orient, float distance)
	{
		return Vector3Int(TranslatePoint(point.ToVector3(), orient, distance));
	}

	Vector3 TranslatePoint(const Vector3& point, const Vector3Shrt& orient, float distance)
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

	Vector3Int TranslatePoint(const Vector3Int& point, const Vector3& direction, float distance)
	{
		return Vector3Int(TranslatePoint(point.ToVector3(), direction, distance));
	}

	Vector3 TranslatePoint(const Vector3& point, const Vector3& direction, float distance)
	{
		auto directionNorm = direction;
		directionNorm.Normalize();
		return (point + (directionNorm * distance));
	}

	short GetShortestAngularDistance(short angleFrom, short angleTo, bool getShortest)
	{
		if (getShortest)
			return short(angleTo - angleFrom);
		else
			return short(angleFrom - angleTo); // TODO: Check this. I have no clue. -- Sezz 2022.08.30
	}

	short GetSurfaceSteepnessAngle(Vector2 tilt)
	{
		short qtrBlockAngleIncrement = ANGLE(45.0f) / 4;
		return (short)sqrt(pow(tilt.x * qtrBlockAngleIncrement, 2) + pow(tilt.y * qtrBlockAngleIncrement, 2));
	}

	short GetSurfaceAspectAngle(Vector2 tilt)
	{
		return (short)phd_atan(-tilt.y, -tilt.x);
	}

	Vector3Shrt GetOrientTowardPoint(const Vector3& origin, const Vector3& target)
	{
		auto direction = target - origin;
		float yOrient = phd_atan(direction.x, direction.z);

		auto vector = direction;
		auto matrix = Matrix::CreateRotationY(-yOrient);
		Vector3::Transform(vector, matrix, vector);

		float xOrient = -phd_atan(direction.y, vector.z);
		return Vector3Shrt(xOrient, yOrient, 0.0f);
	}

	bool IsPointInFront(const PHD_3DPOS& pose, const Vector3& target)
	{
		return IsPointInFront(pose.Position.ToVector3(), target, pose.Orientation);
	}
	
	bool IsPointInFront(const Vector3& origin, const Vector3& target, const Vector3Shrt& orient)
	{
		float sinY = phd_sin(orient.y);
		float cosY = phd_cos(orient.y);

		auto normal = Vector3(sinY, 0.0f, cosY);
		auto difference = origin - target;

		float dot = normal.Dot(difference);
		if (dot >= 0.0f)
			return false;

		return true;
	}

	bool IsPointOnLeft(const Vector3& origin, const Vector3& target, const Vector3Shrt& orient)
	{
		float sinY = phd_sin(orient.y);
		float cosY = phd_cos(orient.y);

		auto normal = Vector3(cosY, 0.0f, -sinY);
		auto difference = origin - target;

		float dot = normal.Dot(difference);
		if (dot >= 0.0f)
			return false;

		return true;
	}

	bool IsPointOnLeft(const Vector3& origin, const Vector3& target, const Vector3& refPoint)
	{
		auto refDirection = refPoint - origin;

		auto normal = Vector3(refDirection.z, 0.0f, -refDirection.x);
		auto direction = target - origin;

		float dot = normal.Dot(direction);
		if (dot > 0.0f)
			return true;

		return false;
	}
//}
