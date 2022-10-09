#include "framework.h"
#include "Math/Containers/Pose.h"

#include "Math/Containers/EulerAngles.h"
#include "Math/Containers/Vector3i.h"
#include "Math/Geometry.h"

using namespace TEN::Math;

//namespace TEN::Math
//{
	const Pose Pose::Zero = Pose(Vector3i::Zero, EulerAngles::Zero);

	Pose::Pose()
	{
	}

	Pose::Pose(const Vector3i& pos)
	{
		this->Position = pos;
	}

	Pose::Pose(int xPos, int yPos, int zPos)
	{
		this->Position = Vector3i(xPos, yPos, zPos);
	}

	Pose::Pose(const EulerAngles& orient)
	{
		this->Orientation = orient;
	}

	Pose::Pose(short xOrient, short yOrient, short zOrient)
	{
		this->Orientation = EulerAngles(xOrient, yOrient, zOrient);
	}

	Pose::Pose(const Vector3i& pos, const EulerAngles& orient)
	{
		this->Position = pos;
		this->Orientation = orient;
	}

	Pose::Pose(const Vector3i& pos, short xOrient, short yOrient, short zOrient)
	{
		this->Position = pos;
		this->Orientation = EulerAngles(xOrient, yOrient, zOrient);
	}

	Pose::Pose(int xPos, int yPos, int zPos, const EulerAngles& orient)
	{
		this->Position = Vector3i(xPos, yPos, zPos);
		this->Orientation = orient;
	}

	Pose::Pose(int xPos, int yPos, int zPos, short xOrient, short yOrient, short zOrient)
	{
		this->Position = Vector3i(xPos, yPos, zPos);
		this->Orientation = EulerAngles(xOrient, yOrient, zOrient);
	}

	void Pose::Translate(short headingAngle, float forward, float down, float right)
	{
		this->Position = Geometry::TranslatePoint(this->Position, headingAngle, forward, down, right);
	}

	void Pose::Translate(const EulerAngles& orient, float distance)
	{
		this->Position = Geometry::TranslatePoint(this->Position, orient, distance);
	}

	void Pose::Translate(const Vector3& direction, float distance)
	{
		this->Position = Geometry::TranslatePoint(this->Position, direction, distance);
	}
//}
