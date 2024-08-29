#pragma once
#include "Math/Math.h"

namespace TEN::Collision::Attractor { class AttractorObject; }

using namespace TEN::Collision::Attractor;
using namespace TEN::Math;

namespace TEN::Player
{
	enum class ClimbContextAlignType
	{
		None,
		Snap,
		OffsetBlend,
		AttractorParent
	};

	struct BasicMovementSetupData
	{
		short HeadingAngle	  = 0;
		int	  LowerFloorBound = 0;
		int	  UpperFloorBound = 0;

		bool TestSteepFloorBelow = true;
		bool TestSteepFloorAbove = true;
		bool TestDeathFloor		 = true;
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
		int LowerEdgeBound			 = 0;
		int UpperEdgeBound			 = 0;
		int LowerEdgeToCeilBound	 = 0;
		int DestFloorToCeilHeightMin = 0;
		int DestFloorToCeilHeightMax = 0;

		bool FindHighest		= false;
		bool TestSwampDepth		= false;
		bool TestEdgeFront		= false;
		bool TestDestSpace		= false;
		bool TestDestSteepFloor = false;
	};

	struct WallEdgeMountClimbSetupData
	{
		int LowerEdgeBound		 = 0;
		int UpperEdgeBound		 = 0;
		int LowerEdgeToCeilBound = 0;
		int CeilHeightMax		 = 0;
	};

	struct EdgeHangDescentClimbSetupData
	{
		short RelHeadingAngle = 0;

		int LowerEdgeBound		 = 0;
		int UpperEdgeBound		 = 0;
		int LowerEdgeToCeilBound = 0;
	};

	struct EdgeVerticalMovementClimbSetupData
	{
		int LowerEdgeBound		  = 0;
		int UpperEdgeBound		  = 0;
		int UpperFloorToEdgeBound = 0;

		bool TestClimbableWall = false;
	};

	struct LedgeClimbSetupData
	{
		int LowerEdgeToCeilBound	 = 0;
		int DestFloorToCeilHeightMin = 0;
		int DestFloorToCeilHeightMax = 0;

		bool TestSteepFloor = false;
	};

	struct ClimbContextData
	{
		AttractorObject* Attractor		 = nullptr;
		float			 PathDistance	 = 0.0f;
		Vector3			 RelPosOffset	 = Vector3::Zero;
		EulerAngles		 RelOrientOffset = EulerAngles::Identity;
		int				 TargetStateID	 = 0;

		ClimbContextAlignType AlignType = ClimbContextAlignType::None;

		bool IsJump = false;
	};

	struct WaterTreadStepOutContextData
	{
		int FloorHeight = 0;
		int AnimNumber	= 0;
	};
}
