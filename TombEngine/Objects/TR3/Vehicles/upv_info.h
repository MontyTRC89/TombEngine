#pragma once
#include "Specific/phd_global.h"

namespace TEN::Entities::Vehicles
{
	struct UpvInfo
	{
		float Velocity = 0.0f;

		Vector3Shrt TurnRate;

		unsigned int HarpoonTimer = 0;
		bool HarpoonLeft = false;
		short TurbineRotation = 0;
		short LeftRudderRotation = 0;
		short RightRudderRotation = 0;

		char Flags = NULL;
	};
}
