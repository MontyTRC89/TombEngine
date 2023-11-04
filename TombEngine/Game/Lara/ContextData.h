#pragma once
#include "Math/Math.h"

namespace TEN::Collision::Attractor { class Attractor; }

using namespace TEN::Collision::Attractor;
using namespace TEN::Math;

namespace TEN::Player::Context
{
	enum class EdgeType
	{
		Ledge,
		ClimbableWall
	};

	enum ShimmyType
	{
		Up,
		Down,
		Left,
		LeftInnerCorner,
		LeftOuterCorner,
		Right,
		RightInnerCorner,
		RightOuterCorner
	};

	struct EdgeCatchData
	{
		Attractor* AttracPtr = nullptr;
		EdgeType   Type		 = EdgeType::Ledge; // TODO: Won't be needed later.

		Vector3 IntersectPoint = Vector3::Zero;
		float	ChainDistance  = 0.0f;
		short	HeadingAngle   = 0;
	};

	struct MonkeySwingCatchData
	{
		int Height = 0;
	};

	struct ShimmyData
	{
		ShimmyType Type			= ShimmyType::Up;
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
