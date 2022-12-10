#pragma once
#include "Math/Math.h"

using namespace TEN::Math;

namespace TEN::Entities::Vehicles
{
	struct BigGunInfo
	{
		EulerAngles BaseOrientation = EulerAngles::Zero;
		EulerAngles TurnRate		= EulerAngles::Zero;
		EulerAngles Rotation		= EulerAngles::Zero;
		short		BarrelRotation	= 0;
		int			XOrientFrame	= 0;

		unsigned int FireCount		  = 0;
		bool		 IsBarrelRotating = false;

		int Flags = 0;
	};
}
