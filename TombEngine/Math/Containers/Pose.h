#pragma once
#include "Math/Containers/EulerAngles.h"
#include "Math/Containers/Vector3i.h"

//namespace TEN::Math
//{
	class Pose
	{
	public:
		// Components
		Vector3i	Position	= Vector3i::Zero;
		EulerAngles Orientation = EulerAngles::Zero;

		// Constants
		static const Pose Zero;

		// Constructors
		Pose();
		Pose(const Vector3i& pos);
		Pose(int xPos, int yPos, int zPos);
		Pose(const EulerAngles& orient);
		Pose(short xOrient, short yOrient, short zOrient);
		Pose(const Vector3i& pos, const EulerAngles& orient);
		Pose(const Vector3i& pos, short xOrient, short yOrient, short zOrient);
		Pose(int xPos, int yPos, int zPos, const EulerAngles& orient);
		Pose(int xPos, int yPos, int zPos, short xOrient, short yOrient, short zOrient);

		// Utilities
		void Translate(short headingAngle, float forward, float down = 0.0f, float right = 0.0f);
		void Translate(const EulerAngles& orient, float distance);
		void Translate(const Vector3& direction, float distance);
	};
//}
