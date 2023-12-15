#pragma once
#include "Math/Math.h"

namespace TEN::Collision::Attractor { class Attractor; }
enum LaraState;

using namespace TEN::Collision::Attractor;
using namespace TEN::Math;

namespace TEN::Entities::Player
{
	enum class EdgeType
	{
		Attractor,
		ClimbableWall
	};

	enum class ClimbContextAlignType
	{
		None,
		AttractorParent,
		OffsetBlend
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

	struct EdgeVaultClimbSetupData
	{
		int LowerEdgeBound			  = 0;
		int UpperEdgeBound			  = 0;
		int LedgeFloorToCeilHeightMin = 0;
		int LedgeFloorToCeilHeightMax = 0;
		int LowerEdgeToCeilBound	  = 0;

		bool TestSwampDepth		   = false;
		bool TestEdgeFront		   = false;
		bool TestLedgeHeights	   = false;
		bool TestLedgeIllegalSlope = false;
	};

	struct EdgeDescentClimbSetupData
	{
		int LowerEdgeBound		 = 0;
		int UpperEdgeBound		 = 0;
		int LowerEdgeToCeilBound = 0;
	};

	struct ClimbContextData
	{
		Attractor*	AttractorPtr	= nullptr;
		float		ChainDistance	= 0.0f;
		Vector3		RelPosOffset	= Vector3::Zero;
		EulerAngles RelOrientOffset = EulerAngles::Zero;
		int			TargetStateID	= 0;

		ClimbContextAlignType AlignType = ClimbContextAlignType::None;

		bool SetBusyHands	 = false;
		bool SetJumpVelocity = false;
	};

	// TODO: Maybe not needed. Can reuse ClimbContextData.
	struct EdgeCatchContextData
	{
		Attractor* AttracPtr = nullptr;

		EdgeType   Type		 = EdgeType::Attractor; // TODO: Won't be needed later.

		Vector3 Intersection  = Vector3::Zero;
		float	ChainDistance = 0.0f;
		short	HeadingAngle  = 0;
	};

	struct MonkeySwingCatchContextData
	{
		int Height = 0;
	};

	struct LedgeClimbSetupData
	{
		short HeadingAngle		   = 0;
		int	  FloorToCeilHeightMin = 0;
		int	  FloorToCeilHeightMax = 0;
		int	  GapHeightMin		   = 0;

		bool TestIllegalSlope = false;
	};

	struct WaterTreadStepOutContextData
	{
		int FloorHeight	  = 0;
		int TargetStateID = 0;
	};

	// TODO: Temporary.
	struct WallClimbContextData
	{
		int Height = 0;
	};
}
