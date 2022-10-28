#pragma once
#include "Math/Math.h"

using namespace TEN::Math;

namespace TEN::Entities::Vehicles
{
	struct UpvInfo
	{
		float Velocity = 0.0f;

		EulerAngles TurnRate = EulerAngles::Zero;

		unsigned int HarpoonTimer = 0;
		bool HarpoonLeft = false;
		short TurbineRotation = 0;
		short LeftRudderRotation = 0;
		short RightRudderRotation = 0;

		char Flags = 0;
	};
}
