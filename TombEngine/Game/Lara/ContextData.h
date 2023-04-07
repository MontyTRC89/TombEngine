#pragma once

class Pose;

namespace TEN::Entities::Player::Context
{
	enum class EdgeType
	{
		Ledge,
		ClimbableWall
	};

	enum class CornerType
	{
		None,
		Inner,
		Outer
	};

	struct EdgeCatchData
	{
		EdgeType Type	= EdgeType::Ledge;
		int		 Height = 0;
	};

	struct MonkeySwingCatchData
	{
		int AnimNumber = 0; // TODO: State dispatch.
		int Height	   = 0;
	};

	struct CornerShimmyData
	{
		bool Success;
		Pose ProbeResult;
		Pose RealPositionResult;
	};

	struct LedgeClimbSetupData
	{
		short HeadingAngle		   = 0;
		int	  FloorToCeilHeightMin = 0;
		int	  FloorToCeilHeightMax = 0;
		int	  GapHeightMin		   = 0;

		bool TestSlipperySlope = false;
	};
}
