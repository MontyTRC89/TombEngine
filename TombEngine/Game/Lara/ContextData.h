#pragma once

namespace TEN::Player
{
	struct GroundMovementSetupData
	{
		short HeadingAngle	  = 0;
		int	  LowerFloorBound = 0;
		int	  UpperFloorBound = 0;

		bool TestSlipperySlopeBelow = true;
		bool TestSlipperySlopeAbove = true;
		bool TestDeathFloor			= true;
	};

	struct MonkeySwingSetupData
	{
		short HeadingAngle		= 0;
		float LowerCeilingBound = 0.0f;
		float UpperCeilingBound = 0.0f;
	};

	struct JumpSetupData
	{
		short HeadingAngle = 0;
		float Distance	   = 0.0f;

		bool TestWadeStatus = true;
	};
}
