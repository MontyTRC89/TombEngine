#pragma once
#include "Math/Math.h"

using namespace TEN::Math;

namespace TEN::Entities::Player::Context
{
	enum class EdgeType
	{
		Ledge,
		ClimbableWall
	};

	enum ShimmyType
	{
		Lateral,
		LeftCornerInner90,
		RightCornerInner90,
		LeftCornerOuter90,
		RightCornerOuter90
	};

	struct EdgeCatchData
	{
		EdgeType Type		  = EdgeType::Ledge;
		Vector3	 Position	  = Vector3::Zero;
		short	 HeadingAngle = 0;
	};

	struct MonkeySwingCatchData
	{
		int Height = 0;
	};

	struct ShimmyData
	{
		ShimmyType Type			= ShimmyType::Lateral;
		short	   HeadingAngle = 0;
	};

	struct LedgeClimbSetupData
	{
		short HeadingAngle		   = 0;
		int	  FloorToCeilHeightMin = 0;
		int	  FloorToCeilHeightMax = 0;
		int	  GapHeightMin		   = 0;

		bool TestSlipperySlope = false;
	};

	// Old
	enum class CornerType
	{
		None,
		Inner,
		Outer
	};
	struct CornerShimmyData
	{
		bool Success			= false;
		Pose RealPositionResult = Pose::Zero;
		Pose ProbeResult		= Pose::Zero;
	};
}
