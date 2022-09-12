#pragma once
#include "Math/Containers/EulerAngles.h"
#include "Math/Containers/Vector3i.h"

//namespace TEN::Math
//{
	struct PoseData
	{
		// Components
		Vector3i	Position	= Vector3i::Zero;
		EulerAngles Orientation = EulerAngles::Zero;

		// Constants
		static const PoseData Zero;

		// Constructors
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
