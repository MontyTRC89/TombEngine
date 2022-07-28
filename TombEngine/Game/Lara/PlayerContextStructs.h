#pragma once

namespace TEN::Entities::Player
{
	struct ContextGroundMovementSetup
	{
		short Angle			  = 0;
		int	  LowerFloorBound = 0;
		int   UpperFloorBound = 0;
		bool  CheckSlopeDown  = true;
		bool  CheckSlopeUp	  = true;
		bool  CheckDeathFloor = true;
	};
}
