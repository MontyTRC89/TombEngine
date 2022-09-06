#pragma once
#include "Math/Angles/EulerAngles.h"
#include "Math/Containers/Vector3i.h"

//namespace TEN::Math
//{
	struct PoseData
	{
		Vector3i	Position	= Vector3i::Zero;
		EulerAngles Orientation = EulerAngles::Zero;

		static const PoseData Empty;

		PoseData();
		PoseData(const Vector3i& pos);
		PoseData(int xPos, int yPos, int zPos);
		PoseData(const EulerAngles& orient);
		PoseData(short xOrient, short yOrient, short zOrient);
		PoseData(const Vector3i& pos, const EulerAngles& orient);
		PoseData(const Vector3i& pos, short xOrient, short yOrient, short zOrient);
		PoseData(int xPos, int yPos, int zPos, const EulerAngles& orient);
		PoseData(int xPos, int yPos, int zPos, short xOrient, short yOrient, short zOrient);
	};
//}
