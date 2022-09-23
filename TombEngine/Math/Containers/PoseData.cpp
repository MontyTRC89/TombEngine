#include "framework.h"
#include "Math/Containers/PoseData.h"

#include "Math/Containers/EulerAngles.h"
#include "Math/Containers/Vector3i.h"
#include "Math/Geometry.h"

using namespace TEN::Math;

//namespace TEN::Math
//{
	const PoseData PoseData::Zero = PoseData(Vector3i::Zero, EulerAngles::Zero);

	PoseData::PoseData()
	{
	}

	PoseData::PoseData(const Vector3i& pos)
	{
		this->Position = pos;
	}

	PoseData::PoseData(int xPos, int yPos, int zPos)
	{
		this->Position = Vector3i(xPos, yPos, zPos);
	}

	PoseData::PoseData(const EulerAngles& orient)
	{
		this->Orientation = orient;
	}

	PoseData::PoseData(short xOrient, short yOrient, short zOrient)
	{
		this->Orientation = EulerAngles(xOrient, yOrient, zOrient);
	}

	PoseData::PoseData(const Vector3i& pos, const EulerAngles& orient)
	{
		this->Position = pos;
		this->Orientation = orient;
	}

	PoseData::PoseData(const Vector3i& pos, short xOrient, short yOrient, short zOrient)
	{
		this->Position = pos;
		this->Orientation = EulerAngles(xOrient, yOrient, zOrient);
	}

	PoseData::PoseData(int xPos, int yPos, int zPos, const EulerAngles& orient)
	{
		this->Position = Vector3i(xPos, yPos, zPos);
		this->Orientation = orient;
	}

	PoseData::PoseData(int xPos, int yPos, int zPos, short xOrient, short yOrient, short zOrient)
	{
		this->Position = Vector3i(xPos, yPos, zPos);
		this->Orientation = EulerAngles(xOrient, yOrient, zOrient);
	}

	void PoseData::Translate(short headingAngle, float forward, float down, float right)
	{
		this->Position = Geometry::TranslatePoint(this->Position, headingAngle, forward, down, right);
	}

	void PoseData::Translate(const EulerAngles& orient, float distance)
	{
		this->Position = Geometry::TranslatePoint(this->Position, orient, distance);
	}

	void PoseData::Translate(const Vector3& direction, float distance)
	{
		this->Position = Geometry::TranslatePoint(this->Position, direction, distance);
	}
//}
