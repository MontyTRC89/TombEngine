#include "framework.h"
#include "Math/Containers/PoseData.h"

#include "Math/Containers/EulerAngles.h"
#include "Math/Containers/Vector3i.h"

//using namespace TEN::Math::Angles;

//namespace TEN::Math
//{
	const PoseData PoseData::Empty = PoseData(Vector3i::Zero, EulerAngles::Zero);

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
//}
