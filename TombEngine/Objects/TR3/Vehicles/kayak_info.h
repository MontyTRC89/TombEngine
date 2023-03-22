#pragma once
#include "Math/Math.h"

using namespace TEN::Math;

namespace TEN::Entities::Vehicles
{
	struct KayakInfo
	{
		int Velocity			  = 0;
		int FrontVerticalVelocity = 0;
		int LeftVerticalVelocity  = 0;
		int RightVerticalVelocity = 0;

		short TurnRate = 0;

		Pose OldPose	 = Pose::Zero;
		int	 WaterHeight = 0;
		bool Turn		 = false;
		bool Forward	 = false;
		bool TrueWater	 = false;

		unsigned int LeftRightPaddleCount = 0;

		int Flags = 0;
	};
}
