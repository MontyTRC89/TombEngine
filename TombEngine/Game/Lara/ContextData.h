#pragma once
#include "Math/Math.h"

using namespace TEN::Math;

namespace TEN::Entities::Player::Context
{
	enum class EdgeType
	{
		Ledge,
		ClimbableWall,
		Attractor
	};

	enum class CornerType
	{
		None,
		Inner,
		Outer
	};

	struct EdgeCatchData
	{
		EdgeType Type		 = EdgeType::Ledge;
		Vector3	 Position	 = Vector3::Zero;
		short	 FacingAngle = 0;
	};

	struct MonkeySwingCatchData
	{
		int AnimNumber = 0; // TODO: State dispatch.
		int Height	   = 0;
	};

	struct CornerShimmyData
	{
		bool Success			= false;
		Pose RealPositionResult = Pose::Zero;
		Pose ProbeResult		= Pose::Zero;
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
