#pragma once

namespace TEN::Entities::Player::Context
{
	struct SetupGroundMovement
	{
		float Angle			  = 0.0f;
		float LowerFloorBound = 0.0f;
		float UpperFloorBound = 0.0f;
		bool  CheckSlopeDown  = true;
		bool  CheckSlopeUp	  = true;
		bool  CheckDeathFloor = true;
	};

	struct SetupMonkeyMovement
	{
		float Angle				= 0.0f;
		float LowerCeilingBound = 0.0f;
		float UpperCeilingBound = 0.0f;
	};

	struct SetupJump
	{
		float Angle			  = 0.0f;
		float Distance		  = 0.0f;
		bool  CheckWadeStatus = true;
	};

}
