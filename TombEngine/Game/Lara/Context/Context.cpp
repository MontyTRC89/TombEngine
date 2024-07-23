#include "framework.h"
#include "Game/Lara/Context/Context.h"

#include "Game/animation.h"
#include "Game/collision/Attractor.h"
#include "Game/collision/collide_item.h"
#include "Game/collision/collide_room.h"
#include "Game/collision/Point.h"
#include "Game/collision/floordata.h"
#include "Game/control/los.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_collide.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/Lara/lara_struct.h"
#include "Game/Lara/lara_tests.h"
#include "Scripting/Include/Flow/ScriptInterfaceFlowHandler.h"
#include "Specific/configuration.h"
#include "Specific/Input/Input.h"

using namespace TEN::Collision::Attractor;
using namespace TEN::Collision::Floordata;
using namespace TEN::Collision::Point;
using namespace TEN::Input;

namespace TEN::Player
{
	PlayerAttractorData::~PlayerAttractorData()
	{
		// TODO: Polymorphism to avoid global.
		Detach(*LaraItem);
	}

	void PlayerAttractorData::Attach(ItemInfo& playerItem, AttractorObject& attrac, float chainDist,
									 const Vector3& relPosOffset, const EulerAngles& relOrientOffset,
									 const Vector3& relDeltaPos, const EulerAngles& relDeltaOrient)
	{
		Detach(playerItem);

		Ptr = &attrac;
		ChainDistance = chainDist;
		RelPosOffset = relPosOffset;
		RelOrientOffset = relOrientOffset;
		RelDeltaPos = relDeltaPos;
		RelDeltaOrient = relDeltaOrient;

		Ptr->AttachPlayer(playerItem);
	}

	void PlayerAttractorData::Detach(ItemInfo& playerItem)
	{
		if (Ptr == nullptr)
			return;

		Ptr->DetachPlayer(playerItem);
		*this = {};
	};

	void PlayerAttractorData::Update(ItemInfo& playerItem, AttractorObject& attrac, float chainDist,
									 const Vector3& relPosOffset, const EulerAngles& relOrientOffset)
	{
		Attach(
			playerItem, attrac, chainDist,
			relPosOffset, relOrientOffset,
			RelDeltaPos, RelDeltaOrient);
	}

	PlayerContext::PlayerContext(const ItemInfo& item, const CollisionInfo& coll)
	{
		const auto& player = GetLaraInfo(item);

		/*ItemPtr = &item;
		PlayerPtr = &player;
		CollPtr = &coll;*/
	}

	bool CanVaultFromSprint(const ItemInfo& item, const CollisionInfo& coll)
	{
		return !TestLaraWall(&item, OFFSET_RADIUS(coll.Setup.Radius), -BLOCK(5 / 8.0f));
	}

	bool CanSlide(const ItemInfo& item, const CollisionInfo& coll)
	{
		constexpr auto ABS_FLOOR_BOUND = STEPUP_HEIGHT;

		const auto& player = GetLaraInfo(item);

		// 1) Test player water status.
		if (player.Control.WaterStatus == WaterStatus::Wade)
			return false;

		// Get point collision.
		auto pointColl = GetPointCollision(item, 0, 0, -coll.Setup.Height / 2); // NOTE: Offset required for correct bridge collision.
		int relFloorHeight = pointColl.GetFloorHeight() - item.Pose.Position.y;

		// 2) Assess point collision.
		if (abs(relFloorHeight) <= ABS_FLOOR_BOUND && // Floor height is within upper/lower floor bounds.
			pointColl.IsSteepFloor())			  // Floor is a slippery slope.
		{
			return true;
		}

		return false;
	}

	bool CanSteerOnSlide(const ItemInfo& item, const CollisionInfo& coll)
	{
		return g_GameFlow->HasSlideExtended();
	}

	bool CanPerformLedgeJump(const ItemInfo& item, const CollisionInfo& coll)
	{
		constexpr auto WALL_HEIGHT_MIN = CLICK(2);

		// 1) Check if ledge jumps are enabled.
		if (!g_GameFlow->HasLedgeJumps())
			return false;

		// Ray collision setup at minimum ledge height.
		auto origin = GameVector(
			item.Pose.Position.x,
			(item.Pose.Position.y - LARA_HEIGHT_STRETCH) + WALL_HEIGHT_MIN,
			item.Pose.Position.z,
			item.RoomNumber);
		auto target = GameVector(
			Geometry::TranslatePoint(origin.ToVector3i(), item.Pose.Orientation.y, OFFSET_RADIUS(coll.Setup.Radius)),
			item.RoomNumber);

		// 2) Assess level geometry ray collision.
		if (LOS(&origin, &target))
			return false;

		// TODO: Assess static object geometry ray collision.

		// Get point collision.
		auto pointColl = GetPointCollision(item);
		int relCeilHeight = pointColl.GetCeilingHeight() - (item.Pose.Position.y - LARA_HEIGHT_STRETCH);

		// 3) Assess point collision.
		if (relCeilHeight >= -coll.Setup.Height) // Ceiling height is below upper ceiling bound.
			return false;

		return true;
	}

	static bool TestLedgeClimbSetup(const ItemInfo& item, CollisionInfo& coll, const LedgeClimbSetupData& setup)
	{
		constexpr auto ABS_FLOOR_BOUND = CLICK(0.5f);

		const auto& player = GetLaraInfo(item);

		// 1) Check for attractor parent.
		if (player.Context.Attractor.Ptr == nullptr)
			return false;

		// Get attractor collision.
		auto attracColl = GetAttractorCollision(*player.Context.Attractor.Ptr, player.Context.Attractor.ChainDistance, item.Pose.Orientation.y);

		// TODO: Probe from player.
		// Get point collision in front of edge. NOTE: Height offset required for correct bridge collision.
		auto pointCollFront = GetPointCollision(
			attracColl.Intersection, attracColl.Attractor->GetRoomNumber(),
			attracColl.HeadingAngle, coll.Setup.Radius, -CLICK(1));

		// TODO: This check fails for no reason.
		// 2) Test for steep floor (if applicable).
		if (setup.TestSteepFloor)
		{
			if (pointCollFront.IsSteepFloor())
				return false;
		}

		// 3) Test for object obstruction.
		if (TestForObjectOnLedge(attracColl, coll.Setup.Radius, -(setup.DestFloorToCeilHeightMin - CLICK(1)), true))
			return false;

		// 4) Test ledge floor-to-edge height.
		int ledgeFloorToEdgeHeight = abs(attracColl.Intersection.y - pointCollFront.GetFloorHeight());
		if (ledgeFloorToEdgeHeight > ABS_FLOOR_BOUND)
			return false;
		
		// 5) Test ledge floor-to-ceiling height.
		int ledgeFloorToCeilHeight = abs(pointCollFront.GetCeilingHeight() - pointCollFront.GetFloorHeight());
		if (ledgeFloorToCeilHeight <= setup.DestFloorToCeilHeightMin ||
			ledgeFloorToCeilHeight > setup.DestFloorToCeilHeightMax)
		{
			return false;
		}

		// Get point collision behind edge.
		auto pointCollBack = GetPointCollision(
			attracColl.Intersection, attracColl.Attractor->GetRoomNumber(),
			attracColl.HeadingAngle, -coll.Setup.Radius);

		// 6) Test if ceiling behind is adequately higher than edge.
		int edgeToCeilHeight = pointCollBack.GetCeilingHeight() - pointCollFront.GetFloorHeight();
		if (edgeToCeilHeight > setup.LowerEdgeToCeilBound)
			return false;

		return true;
	}	

	bool CanPerformLedgeHandstand(const ItemInfo& item, CollisionInfo& coll)
	{
		constexpr auto SETUP = LedgeClimbSetupData
		{
			CLICK(3),				  // Edge-to-ceil height lower bound.
			LARA_HEIGHT, -MAX_HEIGHT, // Destination floor-to-ceil range.
			false					  // Test steep floor.
		};

		return TestLedgeClimbSetup(item, coll, SETUP);
	}

	bool CanClimbLedgeToCrouch(const ItemInfo& item, CollisionInfo& coll)
	{
		constexpr auto SETUP = LedgeClimbSetupData
		{
			(int)CLICK(0.6f),				// Edge-to-ceil height lower bound.
			LARA_HEIGHT_CRAWL, LARA_HEIGHT, // Destination floor-to-ceil range.
			true							// Test steep floor.
		};

		return TestLedgeClimbSetup(item, coll, SETUP);
	}

	bool CanClimbLedgeToStand(const ItemInfo& item, CollisionInfo& coll)
	{
		constexpr auto SETUP = LedgeClimbSetupData
		{
			CLICK(1),				  // Edge-to-ceil height lower bound.
			LARA_HEIGHT, -MAX_HEIGHT, // Destination floor-to-ceil range.
			false					  // Test steep floor.
		};

		return TestLedgeClimbSetup(item, coll, SETUP);
	}

	bool CanDismountTightrope(const ItemInfo& item, const CollisionInfo& coll)
	{
		const auto& player = GetLaraInfo(item);

		auto pointColl = GetPointCollision(item);

		if (player.Control.Tightrope.CanDismount &&			  // Dismount is allowed.
			pointColl.GetFloorHeight() == item.Pose.Position.y) // Floor is level with player.
		{
			return true;
		}

		return false;
	}

	static std::optional<AttractorCollisionData> GetEdgeHangDescecntClimbAttractorCollision(const ItemInfo& item, const CollisionInfo& coll,
																							const EdgeHangDescentClimbSetupData& setup,
																							const std::vector<AttractorCollisionData>& attracColls)
	{
		constexpr auto ABS_EDGE_BOUND = CLICK(0.5f);

		float range2D = std::max<float>(OFFSET_RADIUS(coll.Setup.Radius), Vector2(item.Animation.Velocity.x, item.Animation.Velocity.z).Length());

		// Assess attractor collision.
		for (const auto& attracColl : attracColls)
		{
			// 1) Check attractor type.
			if (attracColl.Attractor->GetType() != AttractorType::Edge &&
				attracColl.Attractor->GetType() != AttractorType::WallEdge)
			{
				continue;
			}

			// 2) Test if edge is within 2D range.
			if (attracColl.Distance2D > range2D)
				continue;

			// 3) Test if edge slope is steep.
			if (abs(attracColl.SlopeAngle) >= STEEP_FLOOR_SLOPE_ANGLE)
				continue;

			// 4) Test edge angle relation.
			if (!attracColl.IsInFront ||
				!TestPlayerInteractAngle(item.Pose.Orientation.y + setup.RelHeadingAngle, attracColl.HeadingAngle + ANGLE(180.0f)))
			{
				continue;
			}

			// Get point collision behind edge.
			auto pointCollBack = GetPointCollision(
				attracColl.Intersection, attracColl.Attractor->GetRoomNumber(),
				attracColl.HeadingAngle, -coll.Setup.Radius);

			// TODO: Add to other functions.
			// 5) Test if player vertical position is adequately close to edge.
			if (abs(attracColl.Intersection.y - item.Pose.Position.y) > ABS_EDGE_BOUND)
				continue;

			// 6) Test if relative edge height is within edge intersection bounds.
			int relEdgeHeight = pointCollBack.GetFloorHeight() - item.Pose.Position.y;
			if (relEdgeHeight >= setup.LowerEdgeBound || // Floor-to-edge height is within lower edge bound.
				relEdgeHeight < setup.UpperEdgeBound)	 // Floor-to-edge height is within upper edge bound.
			{
				continue;
			}

			// 7) Test if ceiling behind is adequately higher than edge.
			int edgeToCeilHeight = pointCollBack.GetCeilingHeight() - attracColl.Intersection.y;
			if (edgeToCeilHeight > setup.LowerEdgeToCeilBound)
				continue;

			return attracColl;
		}

		// No valid edge attractor collision; return nullopt.
		return std::nullopt;
	}

	std::optional<ClimbContextData> GetStandHangDescentFrontClimbContext(const ItemInfo& item, const CollisionInfo& coll)
	{
		constexpr auto ATTRAC_DETECT_RADIUS = BLOCK(0.5f);
		constexpr auto SETUP = EdgeHangDescentClimbSetupData
		{
			ANGLE(0.0f),					  // Relative heading angle.
			-MAX_HEIGHT, LARA_HEIGHT_STRETCH, // Edge height bounds.
			-CLICK(1)						  // Edge-to-ceil height lower bound.
		};

		// Get attractor collisions.
		auto attracColls = GetAttractorCollisions(
			item.Pose.Position.ToVector3(), item.RoomNumber, item.Pose.Orientation.y + SETUP.RelHeadingAngle,
			ATTRAC_DETECT_RADIUS);

		// Get front stand edge hang descent climb context.
		auto attracColl = GetEdgeHangDescecntClimbAttractorCollision(item, coll, SETUP, attracColls);
		if (attracColl.has_value())
		{
			auto context = ClimbContextData{};
			context.Attractor = attracColl->Attractor;
			context.ChainDistance = attracColl->ChainDistance;
			context.RelPosOffset = Vector3(0.0f, 0.0f, -coll.Setup.Radius);
			context.RelOrientOffset = EulerAngles(0, ANGLE(180.0f), 0);
			context.TargetStateID = LS_STAND_EDGE_HANG_DESCENT_FRONT;
			context.AlignType = ClimbContextAlignType::AttractorParent;
			context.IsJump = false;

			return context;
		}

		return std::nullopt;
	}

	std::optional<ClimbContextData> GetStandHangDescentBackClimbContext(const ItemInfo& item, const CollisionInfo& coll)
	{
		constexpr auto ATTRAC_DETECT_RADIUS = BLOCK(0.5f);
		constexpr auto SETUP = EdgeHangDescentClimbSetupData
		{
			ANGLE(180.0f),					  // Relative heading angle.
			-MAX_HEIGHT, LARA_HEIGHT_STRETCH, // Edge height bounds.
			-CLICK(1)						  // Edge-to-ceil height lower bound.
		};

		// Get attractor collisions.
		auto attracColls = GetAttractorCollisions(
			item.Pose.Position.ToVector3(), item.RoomNumber, item.Pose.Orientation.y + SETUP.RelHeadingAngle,
			ATTRAC_DETECT_RADIUS);

		// Get back stand edge hang descent climb context.
		auto attracColl = GetEdgeHangDescecntClimbAttractorCollision(item, coll, SETUP, attracColls);
		if (attracColl.has_value())
		{
			// TODO: Update CanSwingOnLedge().
			int targetStateID = (IsHeld(In::Sprint)/* && CanSwingOnLedge(item, coll, *attracColl)*/) ?
				LS_STAND_EDGE_HANG_DESCENT_BACK_FLIP : LS_STAND_EDGE_HANG_DESCENT_BACK;

			auto context = ClimbContextData{};
			context.Attractor = attracColl->Attractor;
			context.ChainDistance = attracColl->ChainDistance;
			context.RelPosOffset = Vector3(0.0f, 0.0f, coll.Setup.Radius);
			context.RelOrientOffset = EulerAngles::Identity;
			context.TargetStateID = targetStateID;
			context.AlignType = ClimbContextAlignType::AttractorParent;
			context.IsJump = false;

			return context;
		}

		return std::nullopt;
	}

	std::optional<ClimbContextData> GetCrawlHangDescentFrontClimbContext(const ItemInfo& item, const CollisionInfo& coll)
	{
		constexpr auto ATTRAC_DETECT_RADIUS = BLOCK(0.5f);
		constexpr auto SETUP = EdgeHangDescentClimbSetupData
		{
			ANGLE(0.0f),					  // Relative heading angle.
			-MAX_HEIGHT, LARA_HEIGHT_STRETCH, // Edge height bounds.
			-(int)CLICK(0.6f)				  // Edge-to-ceil height lower bound.
		};

		// Get attractor collisions.
		auto attracColls = GetAttractorCollisions(
			item.Pose.Position.ToVector3(), item.RoomNumber, item.Pose.Orientation.y + SETUP.RelHeadingAngle,
			ATTRAC_DETECT_RADIUS);

		// Get front crawl edge hang descent climb context.
		auto attracColl = GetEdgeHangDescecntClimbAttractorCollision(item, coll, SETUP, attracColls);
		if (attracColl.has_value())
		{
			auto context = ClimbContextData{};
			context.Attractor = attracColl->Attractor;
			context.ChainDistance = attracColl->ChainDistance;
			context.RelPosOffset = Vector3(0.0f, 0.0f, -coll.Setup.Radius);
			context.RelOrientOffset = EulerAngles(0, ANGLE(180.0f), 0);
			context.TargetStateID = LS_CRAWL_EDGE_HANG_DESCENT_FRONT;
			context.AlignType = ClimbContextAlignType::AttractorParent;
			context.IsJump = false;

			return context;
		}

		return std::nullopt;
	}

	std::optional<ClimbContextData> GetCrawlHangDescentBackClimbContext(const ItemInfo& item, const CollisionInfo& coll)
	{
		constexpr auto ATTRAC_DETECT_RADIUS = BLOCK(0.5f);
		constexpr auto SETUP = EdgeHangDescentClimbSetupData
		{
			ANGLE(180.0f),					  // Relative heading angle.
			-MAX_HEIGHT, LARA_HEIGHT_STRETCH, // Edge height bounds.
			-(int)CLICK(0.6f)				  // Edge-to-ceil height lower bound.
		};

		// Get attractor collisions.
		auto attracColls = GetAttractorCollisions(
			item.Pose.Position.ToVector3(), item.RoomNumber, item.Pose.Orientation.y + SETUP.RelHeadingAngle,
			ATTRAC_DETECT_RADIUS);

		// Get back crawl edge hang descent climb context.
		auto attracColl = GetEdgeHangDescecntClimbAttractorCollision(item, coll, SETUP, attracColls);
		if (attracColl.has_value())
		{
			auto context = ClimbContextData{};
			context.Attractor = attracColl->Attractor;
			context.ChainDistance = attracColl->ChainDistance;
			context.RelPosOffset = Vector3(0.0f, 0.0f, coll.Setup.Radius);
			context.RelOrientOffset = EulerAngles::Identity;
			context.TargetStateID = LS_CRAWL_EDGE_HANG_DESCENT_BACK;
			context.AlignType = ClimbContextAlignType::AttractorParent;
			context.IsJump = false;

			return context;
		}

		return std::nullopt;
	}

	static std::optional<AttractorCollisionData> GetEdgeCatchAttractorCollision(const ItemInfo& item, const CollisionInfo& coll)
	{
		constexpr auto ATTRAC_DETECT_RADIUS		 = BLOCK(0.5f);
		constexpr auto FLOOR_TO_EDGE_HEIGHT_MIN	 = LARA_HEIGHT_STRETCH;
		constexpr auto WALL_EDGE_FLOOR_THRESHOLD = CLICK(0.25f);

		constexpr auto POINT_COLL_BACK_DOWN_OFFSET	   = -CLICK(1);
		constexpr auto POINT_COLL_FRONT_FORWARD_OFFSET = BLOCK(1 / 256.0f);

		// Get attractor collisions.
		auto attracColls = GetAttractorCollisions(
			item.Pose.Position.ToVector3(), item.RoomNumber, item.Pose.Orientation.y,
			0.0f, -coll.Setup.Height, 0.0f, ATTRAC_DETECT_RADIUS);
		
		float range2D = std::max<float>(OFFSET_RADIUS(coll.Setup.Radius), Vector2(item.Animation.Velocity.x, item.Animation.Velocity.z).Length());

		// Assess attractor collision.
		for (const auto& attracColl : attracColls)
		{
			// 1) Check attractor type.
			if (attracColl.Attractor->GetType() != AttractorType::Edge &&
				attracColl.Attractor->GetType() != AttractorType::WallEdge)
			{
				continue;
			}

			// 2) Test if edge is within 2D range.
			if (attracColl.Distance2D > range2D)
				continue;

			// 3) Test if edge slope is steep.
			if (abs(attracColl.SlopeAngle) >= STEEP_FLOOR_SLOPE_ANGLE)
				continue;

			// 4) Test edge angle relation.
			if (!attracColl.IsInFront || !TestPlayerInteractAngle(item.Pose.Orientation.y, attracColl.HeadingAngle))
				continue;

			// Get point collision behind edge.
			auto pointCollBack = GetPointCollision(
				attracColl.Intersection, attracColl.Attractor->GetRoomNumber(),
				attracColl.HeadingAngle, -coll.Setup.Radius, 0.0f, POINT_COLL_BACK_DOWN_OFFSET);

			// 5) Test if edge is high enough from floor.
			int floorToEdgeHeight = pointCollBack.GetFloorHeight() - attracColl.Intersection.y;
			if (floorToEdgeHeight <= FLOOR_TO_EDGE_HEIGHT_MIN)
				continue;

			// 6) Test if edge is high enough from water surface (if applicable).
			if (pointCollBack.GetWaterTopHeight() != NO_HEIGHT && pointCollBack.GetWaterBottomHeight() != NO_HEIGHT)
			{
				int waterSurfaceToEdgeHeight = pointCollBack.GetWaterTopHeight() - attracColl.Intersection.y;
				int waterBottomToEdgeHeight = pointCollBack.GetWaterBottomHeight() - attracColl.Intersection.y;

				if (waterSurfaceToEdgeHeight <= FLOOR_TO_EDGE_HEIGHT_MIN &&
					waterBottomToEdgeHeight >= 0)
				{
					continue;
				}
			}

			// 7) Test if ceiling behind is adequately higher than edge.
			int edgeToCeilHeight = pointCollBack.GetCeilingHeight() - attracColl.Intersection.y;
			if (edgeToCeilHeight >= 0)
				continue;

			int vPos = item.Pose.Position.y - coll.Setup.Height;
			int relEdgeHeight = attracColl.Intersection.y - vPos;

			float projVerticalVel = item.Animation.Velocity.y + GetEffectiveGravity(item.Animation.Velocity.y);
			bool isFalling = (projVerticalVel >= 0.0f);
			
			// SPECIAL CASE: Wall edge.
			if (attracColl.Attractor->GetType() == AttractorType::WallEdge)
			{
				// 8) Test if player is falling.
				if (!isFalling)
					return std::nullopt;

				// Get point collision in front of edge.
				auto pointCollFront = GetPointCollision(
					attracColl.Intersection, attracColl.Attractor->GetRoomNumber(),
					attracColl.HeadingAngle, POINT_COLL_FRONT_FORWARD_OFFSET, 0.0f, POINT_COLL_BACK_DOWN_OFFSET);

				// TODO: Can do it another way. Parent stacked WallEdge attractors to pushables and gates?
				// 9) Test if wall edge is near wall.
				if (pointCollFront.GetFloorHeight() > (attracColl.Intersection.y + WALL_EDGE_FLOOR_THRESHOLD) &&
					pointCollFront.GetCeilingHeight() < (attracColl.Intersection.y - WALL_EDGE_FLOOR_THRESHOLD))
				{
					return std::nullopt;
				}
			}

			int lowerBound = isFalling ? (int)ceil(projVerticalVel) : 0;
			int upperBound = isFalling ? 0 : (int)floor(projVerticalVel);

			// 10) Test catch trajectory.
			if (relEdgeHeight > lowerBound ||
				relEdgeHeight < upperBound)
			{
				return std::nullopt;
			}

			return attracColl;
		}

		// No valid edge attractor collision; return nullopt.
		return std::nullopt;
	}

	const bool CanSwingOnLedge(const ItemInfo& item, const CollisionInfo& coll, const AttractorCollisionData& attracColl)
	{
		constexpr auto UPPER_FLOOR_BOUND = 0;
		constexpr auto LOWER_CEIL_BOUND	 = CLICK(1.5f);

		auto& player = GetLaraInfo(item);

		// Get point collision.
		auto pointColl = GetPointCollision(
			attracColl.Intersection, item.RoomNumber,
			attracColl.HeadingAngle, coll.Setup.Radius / 2, coll.Setup.Height);

		int relFloorHeight = pointColl.GetFloorHeight() - (attracColl.Intersection.y + coll.Setup.Height);
		int relCeilHeight = pointColl.GetCeilingHeight() - attracColl.Intersection.y;

		// Assess point collision.
		if (relFloorHeight >= UPPER_FLOOR_BOUND && // Floor height is below upper floor bound.
			relCeilHeight <= LOWER_CEIL_BOUND)	   // Ceiling height is above lower ceiling bound.
		{
			return true;
		}

		return false;
	}

	static std::optional<ClimbContextData> GetEdgeJumpCatchClimbContext(const ItemInfo& item, const CollisionInfo& coll)
	{
		constexpr auto VERTICAL_OFFSET = LARA_HEIGHT_STRETCH;

		// Get edge catch climb context.
		auto attracColl = GetEdgeCatchAttractorCollision(item, coll);
		if (attracColl.has_value())
		{
			int targetStateID = ((item.Animation.ActiveState == LS_REACH) && CanSwingOnLedge(item, coll, *attracColl)) ?
				LS_EDGE_HANG_SWING_CATCH : LS_EDGE_HANG_IDLE;

			auto context = ClimbContextData{};
			context.Attractor = attracColl->Attractor;
			context.ChainDistance = attracColl->ChainDistance;
			context.RelPosOffset = Vector3(0.0f, VERTICAL_OFFSET, -coll.Setup.Radius);
			context.RelOrientOffset = EulerAngles::Identity;
			context.TargetStateID = targetStateID;
			context.AlignType = ClimbContextAlignType::AttractorParent;
			context.IsJump = false;

			return context;
		}

		return std::nullopt;
	}

	static std::optional<ClimbContextData> GetMonkeySwingJumpCatchClimbContext(const ItemInfo& item, const CollisionInfo& coll)
	{
		constexpr auto ABS_CEIL_BOUND			= CLICK(0.5f);
		constexpr auto FLOOR_TO_CEIL_HEIGHT_MAX = LARA_HEIGHT_MONKEY;

		const auto& player = GetLaraInfo(item);

		// 1) Check for monkey swing flag.
		if (!player.Control.CanMonkeySwing)
			return std::nullopt;

		// 2) Check collision type.
		if (coll.CollisionType != CollisionType::Top &&
			coll.CollisionType != CollisionType::TopFront)
		{
			return std::nullopt;
		}

		// Get point collision.
		auto pointColl = GetPointCollision(item);

		int vPos = item.Pose.Position.y - coll.Setup.Height;
		int relCeilHeight = abs(pointColl.GetCeilingHeight() - vPos);
		int floorToCeilHeight = abs(pointColl.GetCeilingHeight() - pointColl.GetFloorHeight());

		// 3) Assess point collision.
		if (relCeilHeight <= ABS_CEIL_BOUND &&			  // Ceiling height is within lower/upper ceiling bounds.
			floorToCeilHeight > FLOOR_TO_CEIL_HEIGHT_MAX) // Floor-to-ceiling height is wide enough.
		{
			// HACK: Set catch animation.
			int animNumber = (item.Animation.ActiveState == LS_JUMP_UP) ? LA_JUMP_UP_TO_MONKEY : LA_REACH_TO_MONKEY;
			SetAnimation(*LaraItem, animNumber);

			// Get monkey swing catch climb context.
			auto context = ClimbContextData{};
			context.Attractor = nullptr;
			context.ChainDistance = 0.0f;
			context.RelPosOffset = Vector3(0.0f, item.Pose.Position.y - (pointColl.GetCeilingHeight() + LARA_HEIGHT_MONKEY), 0.0f);
			context.RelOrientOffset = EulerAngles::Identity;
			context.TargetStateID = LS_MONKEY_IDLE;
			context.AlignType = ClimbContextAlignType::Snap;
			context.IsJump = false;

			return context;
		}

		return std::nullopt;
	}

	// TODO: Dispatch checks. Should be added to several animations for most responsive catch actions.
	std::optional<ClimbContextData> GetJumpCatchClimbContext(const ItemInfo& item, const CollisionInfo& coll)
	{
		auto context = std::optional<ClimbContextData>();

		context = GetEdgeJumpCatchClimbContext(item, coll);
		if (context.has_value())
		{
			if (HasStateDispatch(&item, context->TargetStateID))
				return context;
		}

		context = GetMonkeySwingJumpCatchClimbContext(item, coll);
		if (context.has_value())
		{
			if (HasStateDispatch(&item, context->TargetStateID))
				return context;
		}

		return std::nullopt;
	}

	// Steps:
	// 1) Prioritise finding valid position on current attractor.
	// 2) If valid position on CURRENT attractor is UNAVAILABLE/BLOCKED, find nearest connecting attractor.
	//		Whether centre, left, or right, same thing: probe at next theoretical position (3 total: 1 flat and 2 for corners).
	// 3) If valid position on CONNECTING attractor (can probe multiple) is UNAVAILABLE/BLOCKED, FAIL the probe.
	//
	// Blocks include:
	// 1) Corner.
	// 2) End of attractor.
	// 3) Wall or object.
	static std::optional<AttractorCollisionData> GetEdgeHangFlatShimmyClimbAttractorCollision(const ItemInfo& item, const CollisionInfo& coll,
																							  bool isGoingRight)
	{
		constexpr auto ATTRAC_DETECT_RADIUS = BLOCK(1 / 32.0f);

		const auto& player = GetLaraInfo(item);
		const auto& handAttrac = player.Context.Attractor;

		int sign = isGoingRight ? 1 : -1;

		// TODO: Not working.
		// Get velocity from animation.
		const auto& anim = GetAnimData(item);
		float dist = 8.0f;// GetAnimVelocity(anim, item.Animation.FrameNumber).z;

		// Calculate projected chain distances along attractor.
		float chainDistCenter = handAttrac.ChainDistance + (dist * sign);
		float chainDistLeft = chainDistCenter - coll.Setup.Radius;
		float chainDistRight = chainDistCenter + coll.Setup.Radius;

		// Get attractor collisions.
		auto attracCollCenter = GetAttractorCollision(*handAttrac.Ptr, chainDistCenter, item.Pose.Orientation.y);
		return attracCollCenter;
		auto attracCollSide = std::optional<AttractorCollisionData>{};

		auto attracCollsSide = std::vector<AttractorCollisionData>{};

		// TODO: Check "current" side dist and "next" side dist. Otherwise all segments will be required to be at least 50 units long.
		// Get current side attractor collision.
		if (handAttrac.Ptr->IsLooped() ||
			((!isGoingRight && chainDistLeft > 0.0f && chainDistRight) || (isGoingRight && chainDistRight < handAttrac.Ptr->GetLength())))
		{
			float chainDist = isGoingRight ? chainDistRight : chainDistLeft;
			attracCollsSide.push_back(GetAttractorCollision(*handAttrac.Ptr, chainDist, attracCollCenter.HeadingAngle));

			// ???
			// Test for corner.
			short deltaHeadingAngle = abs(Geometry::GetShortestAngle(attracCollCenter.HeadingAngle, attracCollsSide.back().HeadingAngle));
			if (deltaHeadingAngle >= ANGLE(35.0f))
			{
				attracCollsSide.clear();
			}

		}

		// Check for corner.

		// TODO: Assess current side attractor for valid position.
		// If valid current, use.
		if (false)
		{
			attracCollSide = attracCollsSide.back();
		}

		// Get connecting side attractor collisions.
		// TODO: Use current attractor's start/end point OR corner point.
		if (!attracCollSide.has_value())
		{
			int sign = isGoingRight ? 1 : -1;
			attracCollsSide = GetAttractorCollisions(
				attracCollCenter.Intersection, attracCollCenter.Attractor->GetRoomNumber(), attracCollCenter.HeadingAngle,
				0.0f, 0.0f, coll.Setup.Radius * sign, ATTRAC_DETECT_RADIUS);
		}

		// TODO: Assess connecting side attractors for valid position.
		// If valid, return the center collision. Ensure it hugs a corner or start/end point if necessary.
		for (const auto& attracColl : attracCollsSide)
		{
			continue;
		}

		// If valid connecting, use.
		if (attracCollSide.has_value())
		{

		}

		// No valid edge attractor collision; return nullopt.
		return std::nullopt;
	}

	// NOTE: Assumes flat shimmy assessment already failed.
	static std::optional<AttractorCollisionData> GetEdgeHangCornerShimmyClimbAttractorCollision(const ItemInfo& item, const CollisionInfo& coll,
																								bool isGoingRight)
	{
		constexpr auto ATTRAC_DETECT_RADIUS = BLOCK(1 / 32.0f);

		const auto& player = GetLaraInfo(item);
		const auto& handAttrac = player.Context.Attractor;

		// Calculate projected chain distances along attractor.
		float chainDistLeft = handAttrac.ChainDistance - (coll.Setup.Radius * 2);
		float chainDistRight = handAttrac.ChainDistance + (coll.Setup.Radius * 2);

		// Get center attractor collision.
		auto attracCollCenter = GetAttractorCollision(*handAttrac.Ptr, handAttrac.ChainDistance, item.Pose.Orientation.y);

		// Get connecting attractor collisions.
		int sign = isGoingRight ? 1 : -1;
		auto connectingAttracColls = GetAttractorCollisions(
			attracCollCenter.Intersection, attracCollCenter.Attractor->GetRoomNumber(), attracCollCenter.HeadingAngle,
			0.0f, 0.0f, coll.Setup.Radius * sign, ATTRAC_DETECT_RADIUS);

		auto cornerAttracColls = std::vector<AttractorCollisionData>{};

		// 1) Collect corner attractor collision for hands attractor.
		if (handAttrac.Ptr->IsLooped() ||
			((!isGoingRight && chainDistLeft > 0.0f && chainDistRight) || (isGoingRight && chainDistRight < handAttrac.Ptr->GetLength())))
		{
			float chainDist = isGoingRight ? chainDistRight : chainDistLeft;
			cornerAttracColls.push_back(GetAttractorCollision(*handAttrac.Ptr, chainDist, attracCollCenter.HeadingAngle));
		}

		// 2) Collect corner attractor collisions for connecting attractors.
		for (const auto& attracColl : connectingAttracColls)
		{
			auto cornerAttracColl = GetAttractorCollision(*attracColl.Attractor, attracColl.ChainDistance + (coll.Setup.Radius * sign), attracColl.HeadingAngle);
			cornerAttracColls.push_back(cornerAttracColl);
		}

		// 3) Assess attractor collision.
		for (const auto& attracColl : cornerAttracColls)
		{
			// 2.1) Check attractor type.
			if (attracColl.Attractor->GetType() != AttractorType::Edge &&
				attracColl.Attractor->GetType() != AttractorType::WallEdge)
			{
				continue;
			}

			// 3.2) Test if edge slope is steep.
			if (abs(attracColl.SlopeAngle) >= STEEP_FLOOR_SLOPE_ANGLE)
				continue;

			// 3.3) Test if connecting edge heading angle is within cornering threshold.
			short deltaHeadingAngle = abs(Geometry::GetShortestAngle(attracCollCenter.HeadingAngle, attracColl.HeadingAngle));
			if (deltaHeadingAngle < ANGLE(35.0f))
				continue;

			// TODO: Test destination space.
			// TODO: Test for obstructions.

			return attracColl;
		}

		// No valid edge attractor collision; return nullopt.
		return std::nullopt;
	}

	static std::optional<AttractorCollisionData> GetEdgeVerticalMovementClimbAttractorCollision(const ItemInfo& item, const CollisionInfo& coll,
																								const EdgeVerticalMovementClimbSetupData& setup)
	{
		constexpr auto ATTRAC_DETECT_RADIUS		  = BLOCK(0.5f);
		constexpr auto WALL_CLIMB_VERTICAL_OFFSET = CLICK(2);

		const auto& player = GetLaraInfo(item);

		// Get attractor collisions.
		auto currentAttracColl = GetAttractorCollision(*player.Context.Attractor.Ptr, player.Context.Attractor.ChainDistance, item.Pose.Orientation.y);
		auto attracColls = GetAttractorCollisions(
			currentAttracColl.Intersection, currentAttracColl.Attractor->GetRoomNumber(), currentAttracColl.HeadingAngle,
			ATTRAC_DETECT_RADIUS);

		// Calculate 2D intersection on current attractor.
		auto intersect2D0 = Vector2(currentAttracColl.Intersection.x, currentAttracColl.Intersection.z);

		// Assess attractor collision.
		for (const auto& attracColl : attracColls)
		{
			// TODO: Check for wall climb.
			// 1) Check attractor type.
			if (attracColl.Attractor->GetType() != AttractorType::Edge &&
				attracColl.Attractor->GetType() != AttractorType::WallEdge)
			{
				continue;
			}

			// 2) Test if edge slope is steep.
			if (abs(attracColl.SlopeAngle) >= STEEP_FLOOR_SLOPE_ANGLE)
				continue;

			// 3) Test edge angle relation.
			if (!TestPlayerInteractAngle(item.Pose.Orientation.y, attracColl.HeadingAngle))
				continue;

			// 4) Test if relative edge height is within edge intersection bounds.
			int relEdgeHeight = attracColl.Intersection.y - currentAttracColl.Intersection.y;
			if (relEdgeHeight > setup.LowerEdgeBound ||
				relEdgeHeight < setup.UpperEdgeBound)
			{
				continue;
			}

			// 5) Test if attractors are stacked exactly.
			/*auto intersect2D1 = Vector2(attracColl.Intersection.x, attracColl.Intersection.z);
			if (Vector2::DistanceSquared(intersect2D0, intersect2D1) > EPSILON)
				continue;*/

			// Get point collision behind edge.
			auto pointCollBack = GetPointCollision(
				attracColl.Intersection, attracColl.Attractor->GetRoomNumber(),
				attracColl.HeadingAngle, -coll.Setup.Radius);

			// 6) Test if edge is blocked by floor.
			if (pointCollBack.GetFloorHeight() <= currentAttracColl.Intersection.y)
				continue;

			// 7) Test if edge is high enough from floor.
			int floorToEdgeHeight = pointCollBack.GetFloorHeight() - attracColl.Intersection.y;
			if (floorToEdgeHeight <= setup.UpperFloorToEdgeBound)
				continue;

			// 8) Test if ceiling behind is adequately higher than edge.
			int edgeToCeilHeight = pointCollBack.GetCeilingHeight() - attracColl.Intersection.y;
			if (edgeToCeilHeight >= 0)
				continue;

			// 9) Find wall edge for feet (if applicable).
			if (setup.TestClimbableWall)
			{
				bool hasWall = false;
				for (const auto& attracColl2 : attracColls)
				{
					if (&attracColl == &attracColl2)
						continue;

					// 1) Check attractor type.
					if (attracColl2.Attractor->GetType() != AttractorType::WallEdge)
						continue;

					// 2) Test if edge slope is steep.
					if (abs(attracColl2.SlopeAngle) >= STEEP_FLOOR_SLOPE_ANGLE)
						continue;

					// 3) Test edge angle relation.
					if (!TestPlayerInteractAngle(item.Pose.Orientation.y, attracColl2.HeadingAngle))
						continue;

					// TODO: Not working.
					// 4) Test if relative edge height is within edge intersection bounds.
					/*int relEdgeHeight = attracColl2.Intersection.y - currentAttracColl.Intersection.y;
					if (relEdgeHeight > (setup.LowerEdgeBound + WALL_CLIMB_VERTICAL_OFFSET) ||
						relEdgeHeight < (setup.UpperEdgeBound + WALL_CLIMB_VERTICAL_OFFSET))
					{
						continue;
					}*/

					hasWall = true;
					break;
				}

				if (!hasWall)
					continue;
			}

			return attracColl;
		}

		// No valid edge attractor collision; return nullopt.
		return std::nullopt;
	}

	std::optional<ClimbContextData> GetEdgeHangShimmyUpContext(const ItemInfo& item, const CollisionInfo& coll)
	{
		constexpr auto VERTICAL_OFFSET = CLICK(1);

		constexpr auto SETUP = EdgeVerticalMovementClimbSetupData
		{
			(int)-CLICK(0.5f), (int)-CLICK(1.5f),
			LARA_HEIGHT_STRETCH,
			false
		};

		const auto& player = GetLaraInfo(item);

		auto attracColl = GetEdgeVerticalMovementClimbAttractorCollision(item, coll, SETUP);
		if (attracColl.has_value())
		{
			auto context = ClimbContextData{};
			context.Attractor = attracColl->Attractor;
			context.ChainDistance = attracColl->ChainDistance;
			context.RelPosOffset = Vector3(0.0f, SETUP.UpperFloorToEdgeBound + VERTICAL_OFFSET, -coll.Setup.Radius);
			context.RelOrientOffset = EulerAngles::Identity;
			context.TargetStateID = LS_EDGE_HANG_SHIMMY_UP;
			context.AlignType = ClimbContextAlignType::AttractorParent;
			context.IsJump = false;

			return context;
		}

		return std::nullopt;
	}

	std::optional<ClimbContextData> GetEdgeHangShimmyDownContext(const ItemInfo& item, const CollisionInfo& coll)
	{
		constexpr auto VERTICAL_OFFSET = -CLICK(1);

		constexpr auto SETUP = EdgeVerticalMovementClimbSetupData
		{
			(int)CLICK(1.5f), (int)CLICK(0.5f),
			LARA_HEIGHT_STRETCH,
			false
		};

		const auto& player = GetLaraInfo(item);

		auto attracColl = GetEdgeVerticalMovementClimbAttractorCollision(item, coll, SETUP);
		if (attracColl.has_value())
		{
			auto context = ClimbContextData{};
			context.Attractor = attracColl->Attractor;
			context.ChainDistance = attracColl->ChainDistance;
			context.RelPosOffset = Vector3(0.0f, SETUP.UpperFloorToEdgeBound + VERTICAL_OFFSET, -coll.Setup.Radius);
			context.RelOrientOffset = EulerAngles::Identity;
			context.TargetStateID = LS_EDGE_HANG_SHIMMY_DOWN;
			context.AlignType = ClimbContextAlignType::AttractorParent;
			context.IsJump = false;

			return context;
		}

		return std::nullopt;
	}

	static std::optional<ClimbContextData> GetEdgeHangFlatShimmyLeftClimbContext(const ItemInfo& item, const CollisionInfo& coll)
	{
		constexpr auto VERTICAL_OFFSET = LARA_HEIGHT_STRETCH;

		// Get flat shimmy left context.
		auto attracColl = GetEdgeHangFlatShimmyClimbAttractorCollision(item, coll, false);
		if (attracColl.has_value())
		{
			auto context = ClimbContextData{};
			context.Attractor = attracColl->Attractor;
			context.ChainDistance = attracColl->ChainDistance;
			context.RelPosOffset = Vector3(0.0f, VERTICAL_OFFSET, -coll.Setup.Radius);
			context.RelOrientOffset = EulerAngles::Identity;
			context.TargetStateID = LS_EDGE_HANG_SHIMMY_LEFT;

			return context;
		}

		return std::nullopt;
	}

	static std::optional<ClimbContextData> GetEdgeHangCornerShimmyLeftClimbContext(const ItemInfo& item, const CollisionInfo& coll)
	{
		constexpr auto VERTICAL_OFFSET = LARA_HEIGHT_STRETCH;

		// Get corner shimmy left context.
		auto attracColl = GetEdgeHangCornerShimmyClimbAttractorCollision(item, coll, false);
		if (attracColl.has_value())
		{
			short deltaHeadingAngle = Geometry::GetShortestAngle(item.Pose.Orientation.y, attracColl->HeadingAngle);
			auto relPosOffset = (deltaHeadingAngle >= ANGLE(0.0f)) ?
				Vector3(coll.Setup.Radius, VERTICAL_OFFSET, 0.0f) :
				Vector3(-coll.Setup.Radius, VERTICAL_OFFSET, -coll.Setup.Radius * 2); // TODO

			auto context = ClimbContextData{};
			context.Attractor = attracColl->Attractor;
			context.ChainDistance = attracColl->ChainDistance;
			context.RelPosOffset = relPosOffset;
			context.RelOrientOffset = EulerAngles::Identity;
			context.TargetStateID = (deltaHeadingAngle >= ANGLE(0.0f)) ? LS_EDGE_HANG_SHIMMY_90_OUTER_LEFT : LS_EDGE_HANG_SHIMMY_90_INNER_LEFT;

			return context;
		}

		return std::nullopt;
	}
	
	std::optional<ClimbContextData> GetEdgeHangShimmyLeftContext(const ItemInfo& item, const CollisionInfo& coll)
	{
		auto context = std::optional<ClimbContextData>();

		// 1) Flat shimmy left.
		context = GetEdgeHangFlatShimmyLeftClimbContext(item, coll);
		if (context.has_value())
		{
			//if (HasStateDispatch(&item, context->TargetStateID))
				return context;
		}

		// 2) Corner shimmy left.
		context = GetEdgeHangCornerShimmyLeftClimbContext(item, coll);
		if (context.has_value())
		{
			if (HasStateDispatch(&item, context->TargetStateID))
				return context;
		}

		return std::nullopt;
	}

	static std::optional<ClimbContextData> GetEdgeHangFlatShimmyRightClimbContext(const ItemInfo& item, const CollisionInfo& coll)
	{
		constexpr auto VERTICAL_OFFSET = LARA_HEIGHT_STRETCH;

		// Get flat shimmy right context.
		auto attracColl = GetEdgeHangFlatShimmyClimbAttractorCollision(item, coll, true);
		if (attracColl.has_value())
		{
			auto context = ClimbContextData{};
			context.Attractor = attracColl->Attractor;
			context.ChainDistance = attracColl->ChainDistance;
			context.RelPosOffset = Vector3(0.0f, VERTICAL_OFFSET, -coll.Setup.Radius);
			context.RelOrientOffset = EulerAngles::Identity;
			context.TargetStateID = LS_EDGE_HANG_SHIMMY_RIGHT;

			return context;
		}

		return std::nullopt;
	}

	static std::optional<ClimbContextData> GetEdgeHangCornerShimmyRightClimbContext(const ItemInfo& item, const CollisionInfo& coll)
	{
		// Get corner shimmy right context.
		auto attracColl = GetEdgeHangCornerShimmyClimbAttractorCollision(item, coll, true);
		if (attracColl.has_value())
		{
			short deltaHeadingAngle = Geometry::GetShortestAngle(item.Pose.Orientation.y, attracColl->HeadingAngle);

			auto context = ClimbContextData{};
			context.Attractor = attracColl->Attractor;
			context.ChainDistance = attracColl->ChainDistance;
			context.TargetStateID = (deltaHeadingAngle >= ANGLE(0.0f)) ? LS_EDGE_HANG_SHIMMY_90_OUTER_RIGHT : LS_EDGE_HANG_SHIMMY_90_INNER_RIGHT;

			return context;
		}

		return std::nullopt;
	}

	std::optional<ClimbContextData> GetEdgeHangShimmyRightContext(const ItemInfo& item, const CollisionInfo& coll)
	{
		auto context = std::optional<ClimbContextData>();

		// 1) Flat shimmy right.
		context = GetEdgeHangFlatShimmyRightClimbContext(item, coll);
		if (context.has_value())
		{
			//if (HasStateDispatch(&item, context->TargetStateID))
				return context;
		}

		// 2) Corner shimmy right.
		context = GetEdgeHangCornerShimmyRightClimbContext(item, coll);
		if (context.has_value())
		{
			if (HasStateDispatch(&item, context->TargetStateID))
				return context;
		}

		return std::nullopt;
	}

	bool CanEdgeHangToWallClimb(const ItemInfo& item, const CollisionInfo& coll)
	{
		const auto& player = GetLaraInfo(item);

		if (player.Context.Attractor.Ptr == nullptr)
			return false;

		if (player.Context.Attractor.Ptr->GetType() != AttractorType::WallEdge)
			return false;

		return true;
	}

	// TODO: Climb up, to edge hang, or climb ledge.
	std::optional<ClimbContextData> GetWallClimbUpContext(const ItemInfo& item, const CollisionInfo& coll)
	{
		constexpr auto VERTICAL_OFFSET = CLICK(1);

		constexpr auto SETUP = EdgeVerticalMovementClimbSetupData
		{
			(int)-CLICK(0.5f), (int)-CLICK(1.5f),
			PLAYER_HEIGHT_WALL_CLIMB,
			true
		};

		const auto& player = GetLaraInfo(item);

		if (!HasStateDispatch(&item, LS_WALL_CLIMB_UP))
			return std::nullopt;

		auto attracColl = GetEdgeVerticalMovementClimbAttractorCollision(item, coll, SETUP);
		if (attracColl.has_value())
		{
			auto context = ClimbContextData{};
			context.Attractor = attracColl->Attractor;
			context.ChainDistance = attracColl->ChainDistance;
			context.RelPosOffset = Vector3(0.0f, coll.Setup.Height + VERTICAL_OFFSET, -coll.Setup.Radius);
			context.RelOrientOffset = EulerAngles::Identity;
			context.TargetStateID = LS_WALL_CLIMB_UP;
			context.AlignType = ClimbContextAlignType::AttractorParent;
			context.IsJump = false;

			return context;
		}

		return std::nullopt;
	}

	// TODO: Climb down or to edge hang.
	std::optional<ClimbContextData> GetWallClimbDownContext(const ItemInfo& item, const CollisionInfo& coll)
	{
		constexpr auto VERTICAL_OFFSET = -CLICK(1);

		constexpr auto SETUP = EdgeVerticalMovementClimbSetupData
		{
			(int)CLICK(1.5f), (int)CLICK(0.5f),
			PLAYER_HEIGHT_WALL_CLIMB,
			true
		};

		const auto& player = GetLaraInfo(item);

		if (!HasStateDispatch(&item, LS_WALL_CLIMB_DOWN))
			return std::nullopt;

		auto attracColl = GetEdgeVerticalMovementClimbAttractorCollision(item, coll, SETUP);
		if (attracColl.has_value())
		{
			auto context = ClimbContextData{};
			context.Attractor = attracColl->Attractor;
			context.ChainDistance = attracColl->ChainDistance;
			context.RelPosOffset = Vector3(0.0f, coll.Setup.Height + VERTICAL_OFFSET, -coll.Setup.Radius);
			context.RelOrientOffset = EulerAngles::Identity;
			context.TargetStateID = LS_WALL_CLIMB_DOWN;
			context.AlignType = ClimbContextAlignType::AttractorParent;
			context.IsJump = false;

			return context;
		}

		return std::nullopt;
	}

	static std::optional<ClimbContextData> GetWallClimbSideDismountContext(const ItemInfo& item, const CollisionInfo& coll, bool isGoingRight)
	{
		constexpr auto ABS_FLOOR_BOUND			= CLICK(0.5f);
		constexpr auto FLOOR_TO_CEIL_HEIGHT_MAX = LARA_HEIGHT;

		const auto& player = GetLaraInfo(item);

		// Get attractor collision.
		auto attracColl = GetAttractorCollision(*player.Context.Attractor.Ptr, player.Context.Attractor.ChainDistance, item.Pose.Orientation.y);

		// TODO: Use player room number?
		// Get point collision.
		auto pointColl = GetPointCollision(
			attracColl.Intersection, attracColl.Attractor->GetRoomNumber(), attracColl.HeadingAngle,
			-coll.Setup.Radius, 0.0f, (coll.Setup.Radius * 2) * (isGoingRight ? 1 : -1));
		int vPos = attracColl.Intersection.y + coll.Setup.Height;

		// 1) Test relative edge-to-floor height.
		int relFloorHeight = abs(pointColl.GetFloorHeight() - vPos);
		if (relFloorHeight > ABS_FLOOR_BOUND)
			return std::nullopt;

		// 2) Test floor-to-ceiling height.
		int floorToCeilHeight = abs(pointColl.GetFloorHeight() - pointColl.GetCeilingHeight());
		if (floorToCeilHeight >= FLOOR_TO_CEIL_HEIGHT_MAX)
			return std::nullopt;

		// 3) Create and return climb context.
		auto context = ClimbContextData{};
		context.Attractor = player.Context.Attractor.Ptr;
		context.ChainDistance = player.Context.Attractor.ChainDistance;
		context.RelPosOffset = Vector3(0.0f, coll.Setup.Height, -coll.Setup.Radius);
		context.RelOrientOffset = EulerAngles::Identity;
		context.AlignType = ClimbContextAlignType::AttractorParent;
		context.TargetStateID = isGoingRight ? LS_WALL_CLIMB_DISMOUNT_RIGHT : LS_WALL_CLIMB_DISMOUNT_LEFT;
		context.IsJump = false;

		return context;
	}

	static std::optional<ClimbContextData> GetWallClimbMoveLeftContext(const ItemInfo& item, const CollisionInfo& coll)
	{
		return std::nullopt;
	}

	static std::optional<ClimbContextData> GetWallClimbCornerLeftContext(const ItemInfo& item, const CollisionInfo& coll)
	{
		return std::nullopt;

		auto attracColl = std::optional<AttractorCollisionData>();

		// 1) Get left inner corner context.
		attracColl = std::optional<AttractorCollisionData>();// GetWallClimbInnerCornerLeftAttractorCollision(item, coll);
		if (attracColl.has_value())
		{
			// Create and return climb context.
			auto context = ClimbContextData{};
			context.Attractor = attracColl->Attractor;
			context.ChainDistance = attracColl->ChainDistance;
			context.RelPosOffset = Vector3(0.0f, coll.Setup.Height, -coll.Setup.Radius);
			context.RelOrientOffset = EulerAngles(0, ANGLE(90.0f), 0);
			context.AlignType = ClimbContextAlignType::AttractorParent;
			context.TargetStateID = LS_EDGE_HANG_SHIMMY_90_INNER_LEFT;
			context.IsJump = false;

			return context;
		}
		
		// 2) Get left outer corner context.
		attracColl = std::optional<AttractorCollisionData>();// GetWallClimbOuterCornerLeftAttractorCollision(item, coll);
		if (attracColl.has_value())
		{
			// Create and return climb context.
			auto context = ClimbContextData{};
			context.Attractor = attracColl->Attractor;
			context.ChainDistance = attracColl->ChainDistance;
			context.RelPosOffset = Vector3(coll.Setup.Radius, coll.Setup.Height, coll.Setup.Radius);
			context.RelOrientOffset = EulerAngles(0, ANGLE(-90.0f), 0);
			context.AlignType = ClimbContextAlignType::AttractorParent;
			context.TargetStateID = LS_EDGE_HANG_SHIMMY_90_OUTER_LEFT;
			context.IsJump = false;

			return context;
		}

		return std::nullopt;
	}

	static std::optional<ClimbContextData> GetWallClimbDismountLeftContext(const ItemInfo& item, const CollisionInfo& coll)
	{
		return GetWallClimbSideDismountContext(item, coll, false);
	}
	
	std::optional<ClimbContextData> GetWallClimbLeftContext(const ItemInfo& item, const CollisionInfo& coll)
	{
		auto context = std::optional<ClimbContextData>();

		// 1) Move left on wall.
		context = GetWallClimbMoveLeftContext(item, coll);
		if (context.has_value())
			return context;

		// 2) Corner left on wall.
		context = GetWallClimbCornerLeftContext(item, coll);
		if (context.has_value())
			return context;

		// 3) Dismount wall left.
		context = GetWallClimbDismountLeftContext(item, coll);
		if (context.has_value())
			return context;

		return std::nullopt;
	}

	static std::optional<ClimbContextData> GetWallClimbMoveRightContext(const ItemInfo& item, const CollisionInfo& coll)
	{
		return std::nullopt;
	}

	static std::optional<ClimbContextData> GetWallClimbCornerRightContext(const ItemInfo& item, const CollisionInfo& coll)
	{
		return std::nullopt;
	}

	static std::optional<ClimbContextData> GetWallClimbDismountRightContext(const ItemInfo& item, const CollisionInfo& coll)
	{
		return GetWallClimbSideDismountContext(item, coll, true);
	}

	std::optional<ClimbContextData> GetWallClimbRightContext(const ItemInfo& item, const CollisionInfo& coll)
	{
		auto context = std::optional<ClimbContextData>();

		// 1) Move right on wall.
		context = GetWallClimbMoveRightContext(item, coll);
		if (context.has_value())
			return context;

		// 2) Corner move right on wall.
		context = GetWallClimbCornerRightContext(item, coll);
		if (context.has_value())
			return context;

		// 3) Dismount wall right.
		context = GetWallClimbDismountRightContext(item, coll);
		if (context.has_value())
			return context;

		return std::nullopt;
	}

	bool CanWallClimbToEdgeHang(const ItemInfo& item, const CollisionInfo& coll)
	{
		return true;
	}
}
