#pragma once

namespace TEN::Entities::Player::Context
{
	struct LedgeClimbSetupData
	{
		short HeadingAngle = 0;
		int	  SpaceMin	   = 0;
		int	  SpaceMax	   = 0;
		int	  GapMin	   = 0;

		bool TestSlipperySlope = false;
	};
}
