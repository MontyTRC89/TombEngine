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
		short HeadingAngle		   = 0;
		int	  FloorToCeilHeightMin = 0;
		int	  FloorToCeilHeightMax = 0;
		int	  GapHeightMin		   = 0;

		bool TestSlipperySlope = false;
	};

	struct MonkeySwingCatchData
	{
		int AnimNumber = 0; // TODO: State dispatch.
		int Height	   = 0;
	};
}
