#pragma once

namespace TEN::Entities::Player::Context
{
	struct LedgeClimbSetupData
	{
		short HeadingAngle		= 0;
		int	  FloorToCeilingMin = 0;
		int	  FloorToCeilingMax = 0;
		int	  GapHeightMin		= 0;

		bool TestSlipperySlope = false;
	};
}
