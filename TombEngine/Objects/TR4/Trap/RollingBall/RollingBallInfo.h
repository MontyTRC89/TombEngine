#pragma once

#include "Math/Math.h"

//TODO: Is this namespace correct?
namespace TEN::Entities::Generic
{

	struct RollingBallInfo
	{
		int radius = BLOCK(0.5f);
		bool isOnGround = true;

		short energy_movement = 0;
		short energy_bounce = 0;
		float speed;
		
		Vector3 movementVector;
		
	};
}