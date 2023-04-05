#pragma once

namespace TEN::Entities::Player::Context
{
	enum class EdgeType
	{
		Ledge,
		ClimbableWall
	};

	struct EdgeCatchData
	{
		EdgeType Type	= EdgeType::Ledge;
		int		 Height = 0;
	};

	struct LedgeClimbSetupData
	{
		short HeadingAngle		= 0;
		int	  FloorToCeilingMin = 0;
		int	  FloorToCeilingMax = 0;
		int	  GapHeightMin		= 0;

		bool TestSlipperySlope = false;
	};
}
