#pragma once

namespace TEN::Entities::Player
{
	struct ContextSetupGroundMovement
	{
		short Angle			  = 0;
		int	  LowerFloorBound = 0;
		int   UpperFloorBound = 0;
		bool  CheckSlopeDown  = true;
		bool  CheckSlopeUp	  = true;
		bool  CheckDeathFloor = true;
	};

	struct ContextSetupMonkeyMovement
	{
		short Angle				= 0;
		int	  LowerCeilingBound = 0;
		int   UpperCeilingBound = 0;
	};

}
