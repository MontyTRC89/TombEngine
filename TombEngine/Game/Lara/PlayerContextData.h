#pragma once
#include "Math/Math.h"

namespace TEN::Collision::Attractor { class Attractor; }

using namespace TEN::Collision::Attractor;
using namespace TEN::Math;

namespace TEN::Entities::Player
{
	enum class EdgeType
	{
		Attractor,
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

	struct GroundMovementSetupData
	{
		short HeadingAngle	  = 0;
		int	  LowerFloorBound = 0;
		int	  UpperFloorBound = 0;

		bool TestSlipperySlopeBelow = true;
		bool TestSlipperySlopeAbove = true;
		bool TestDeathFloor			= true;
	};

	struct MonkeySwingMovementSetupData
	{
		short HeadingAngle		= 0;
		int	  LowerCeilingBound = 0.0f;
		int	  UpperCeilingBound = 0.0f;
	};

	struct JumpSetupData
	{
		short HeadingAngle = 0;
		float Distance	   = 0.0f;

		bool TestWadeStatus = true;
	};

	struct VaultSetupData
	{
		int LowerEdgeBound			  = 0;
		int UpperEdgeBound			  = 0;
		int LedgeFloorToCeilHeightMin = 0;
		int LedgeFloorToCeilHeightMax = 0;
		int EdgeToCeilHeightMin		  = 0;

		bool TestSwampDepth = false;
		bool TestLedge		= false;
	};

	struct VaultContextData
	{
		Attractor* AttracPtr	 = nullptr;
		Vector3	   Intersection	 = Vector3::Zero;
		short	   EdgeAngle	 = 0;
		int		   TargetStateID = 0;

		bool SetBusyHands	 = false;
		bool SnapToLedge	 = false;
		bool SetJumpVelocity = false;
	};

	struct EdgeCatchData
	{
		Attractor* AttracPtr = nullptr;
		EdgeType   Type		 = EdgeType::Attractor; // TODO: Won't be needed later.

		Vector3 Intersection  = Vector3::Zero;
		float	ChainDistance = 0.0f;
		short	HeadingAngle  = 0;
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

		bool TestIllegalSlope = false;
	};
}
