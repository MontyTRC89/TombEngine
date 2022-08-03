#include "framework.h"
#include "Math/Geometry.h"

#include "Math/Angles/Angle.h"
#include "Math/Angles/EulerAngles.h"
#include "Math/Vector3i.h"

//namespace TEN::Math::Geometry
//{
	Vector3 TranslatePoint(Vector3 point, float angle, float forward, float up, float right)
	{
		if (forward == 0.0f && up == 0.0f && right == 0.0f)
			return point;

		float sinAngle = sin(angle);
		float cosAngle = cos(angle);

		point.x += (forward * sinAngle) + (right * cosAngle);
		point.y += up;
		point.z += (forward * cosAngle) - (right * sinAngle);
		return point;
	}

	Vector3Int TranslatePoint(Vector3Int point, float angle, float forward, float up, float right)
	{
		auto newPoint = TranslatePoint(point.ToVector3(), angle, forward, up, right);
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

	Vector3 TranslatePoint(Vector3 point, EulerAngles orient, float distance)
	{
		if (distance == 0.0f)
			return point;

		float sinX = sin(orient.x);
		float cosX = cos(orient.x);
		float sinY = sin(orient.y);
		float cosY = cos(orient.y);

		point.x += distance * (sinY * cosX);
		point.y -= distance * sinX;
		point.z += distance * (cosY * cosX);
		return point;
	}

	Vector3Int TranslatePoint(Vector3Int point, EulerAngles orient, float distance)
	{
		auto newPoint = TranslatePoint(point.ToVector3(), orient, distance);
		return Vector3Int(
			(int)round(newPoint.x),
			(int)round(newPoint.y),
			(int)round(newPoint.z)
		);
	}

	/*Vector3 TranslatePoint(Vector3 point, Vector3 target, float distance)
	{
		if (distance == 0.0f)
			return point;

		float distanceBetweenPoints = Vector3::Distance(point, target);
		if (distance > distanceBetweenPoints)
			return target;

		auto direction = target - point;
		direction.Normalize();
		return (point + (direction * distance));
	}

	Vector3Int TranslatePoint(Vector3Int point, Vector3Int target, float distance)
	{
		auto newPoint = TranslatePoint(point.ToVector3(), target.ToVector3(), distance);
		return Vector3Int(
			(int)round(newPoint.x),
			(int)round(newPoint.y),
			(int)round(newPoint.z)
		);
	}*/

	EulerAngles GetOrientTowardPoint(Vector3 origin, Vector3 target)
	{
		auto direction = target - origin;
		auto yOrient = Angle(atan2(direction.x, direction.z));

		auto vector = direction;
		auto matrix = Matrix::CreateRotationY(-yOrient);
		Vector3::Transform(vector, matrix, vector);

		auto xOrient = Angle(-atan2(direction.y, vector.z));
		return EulerAngles(xOrient, yOrient, 0.0f);
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

	bool IsPointOnLeft(Vector3 origin, EulerAngles orient, Vector3 target)
	{
		float sinY = sin(orient.y);
		float cosY = cos(orient.y);

		auto normal = Vector3(cosY, 0.0f, -sinY);
		auto difference = origin - target;

		float dot = normal.Dot(difference);
		if (dot >= 0.0f)
			return false;
		
		return true;
	}
//}
