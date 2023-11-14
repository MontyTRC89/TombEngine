#pragma once
#include "Math/Objects/EulerAngles.h"
#include "Math/Objects/Vector3i.h"

//namespace TEN::Math
//{
	class Pose
	{
	public:
		// Members
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
		void Translate(const EulerAngles& orient, float dist);
		void Translate(const Vector3& dir, float dist);

		// Operators
		bool operator ==(const Pose& pose) const;
		bool operator !=(const Pose& pose) const;
	};
//}
