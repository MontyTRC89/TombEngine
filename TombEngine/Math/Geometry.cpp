#include "framework.h"
#include "Math/Geometry.h"

#include "Math/Angles/Angle.h"
#include "Math/Angles/EulerAngles.h"

//namespace TEN::Math::Geometry
//{
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

	float GetDeltaHeading(Vector3 origin, Vector3 target, float heading)
	{
		auto difference = GetOrientTowardPoint(origin, target).y;
		return Angle::GetShortestAngularDistance(heading, difference + Angle::DegToRad(90.0f));
	}
//}
