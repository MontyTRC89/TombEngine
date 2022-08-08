#include "framework.h"
#include "Math/Geometry.h"

#include "Math/Containers/Vector3i.h"
#include "Math/Containers/Vector3s.h"
#include "Specific/trmath.h"

//namespace TEN::Math::Geometry
//{
	Vector3 TranslatePoint(Vector3 point, short angle, float forward, float up, float right)
	{
		if (forward == 0.0f && up == 0.0f && right == 0.0f)
			return point;

		float sinAngle = phd_sin(angle);
		float cosAngle = phd_cos(angle);

		point.x += (forward * sinAngle) + (right * cosAngle);
		point.y += up;
		point.z += (forward * cosAngle) - (right * sinAngle);
		return point;
	}

	Vector3Int TranslatePoint(Vector3Int point, short angle, float forward, float up, float right)
	{
		auto newPoint = TranslatePoint(point.ToVector3(), angle, forward, up, right);
		return Vector3Int(
			(int)round(newPoint.x),
			(int)round(newPoint.y),
			(int)round(newPoint.z)
		);
	}

	Vector3 TranslatePoint(Vector3 point, Vector3Shrt orient, float distance)
	{
		if (distance == 0.0f)
			return point;

		float sinX = phd_sin(orient.x);
		float cosX = phd_cos(orient.x);
		float sinY = phd_sin(orient.y);
		float cosY = phd_cos(orient.y);

		point.x += distance * (sinY * cosX);
		point.y -= distance * sinX;
		point.z += distance * (cosY * cosX);
		return point;
	}

	Vector3Int TranslatePoint(Vector3Int point, Vector3Shrt orient, float distance)
	{
		auto newPoint = TranslatePoint(point.ToVector3(), orient, distance);
		return Vector3Int(
			(int)round(newPoint.x),
			(int)round(newPoint.y),
			(int)round(newPoint.z)
		);
	}

	Vector3 TranslatePoint(Vector3 point, Vector3 direction, float distance)
	{
		direction.Normalize();
		point += direction * distance;
		return point;
	}

	Vector3Int TranslatePoint(Vector3Int point, Vector3 direction, float distance)
	{
		auto newPoint = TranslatePoint(point.ToVector3(), direction, distance);
		return Vector3Int(
			(int)round(newPoint.x),
			(int)round(newPoint.y),
			(int)round(newPoint.z)
		);
	}

	Vector3Shrt GetOrientTowardPoint(Vector3 origin, Vector3 target)
	{
		auto direction = target - origin;
		float yOrient = phd_atan(direction.x, direction.z);

		auto vector = direction;
		auto matrix = Matrix::CreateRotationY(-yOrient);
		Vector3::Transform(vector, matrix, vector);

		float xOrient = -phd_atan(direction.y, vector.z);
		return Vector3Shrt(xOrient, yOrient, 0.0f);
	}

	bool IsPointOnLeft(Vector3 origin, Vector3 refPoint, Vector3 target)
	{
		auto refDirection = refPoint - origin;

		auto normal = Vector3(refDirection.z, 0.0f, -refDirection.x);
		auto targetDirection = target - origin;

		float dot = normal.Dot(targetDirection);
		if (dot > 0.0f)
			return true;

		return false;
	}

	bool IsPointOnLeft(Vector3 origin, Vector3Shrt orient, Vector3 target)
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
//}
