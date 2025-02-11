#include "framework.h"
#include "Game/Lara/Context/HangDescent.h"

#include "Game/animation.h"
#include "Game/collision/Attractor.h"
#include "Game/collision/collide_item.h"
#include "Game/collision/collide_room.h"
#include "Game/collision/Point.h"
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
using namespace TEN::Collision::Point;
using namespace TEN::Input;

namespace TEN::Player
{
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
			if (abs(attracColl.SlopeAngle) >= DEFAULT_STEEP_FLOOR_SLOPE_ANGLE)
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
		auto attracColls = GetAttractorCollisions(item.Pose.Position.ToVector3(), ATTRAC_DETECT_RADIUS, item.RoomNumber, item.Pose.Orientation.y + SETUP.RelHeadingAngle);

		// Get front stand edge hang descent climb context.
		auto attracColl = GetEdgeHangDescecntClimbAttractorCollision(item, coll, SETUP, attracColls);
		if (attracColl.has_value())
		{
			auto context = ClimbContextData{};
			context.Attractor = attracColl->Attractor;
			context.PathDistance = attracColl->PathDistance;
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
			ANGLE(-180.0f),					  // Relative heading angle.
			-MAX_HEIGHT, LARA_HEIGHT_STRETCH, // Edge height bounds.
			-CLICK(1)						  // Edge-to-ceil height lower bound.
		};

		// Get attractor collisions.
		auto attracColls = GetAttractorCollisions(item.Pose.Position.ToVector3(), ATTRAC_DETECT_RADIUS, item.RoomNumber, item.Pose.Orientation.y + SETUP.RelHeadingAngle);

		// Get back stand edge hang descent climb context.
		auto attracColl = GetEdgeHangDescecntClimbAttractorCollision(item, coll, SETUP, attracColls);
		if (attracColl.has_value())
		{
			// TODO: Update CanSwingOnLedge().
			int targetStateID = (IsHeld(In::Sprint)/* && CanSwingOnLedge(item, coll, *attracColl)*/) ?
				LS_STAND_EDGE_HANG_DESCENT_BACK_FLIP : LS_STAND_EDGE_HANG_DESCENT_BACK;

			auto context = ClimbContextData{};
			context.Attractor = attracColl->Attractor;
			context.PathDistance = attracColl->PathDistance;
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
		auto attracColls = GetAttractorCollisions(item.Pose.Position.ToVector3(), ATTRAC_DETECT_RADIUS, item.RoomNumber, item.Pose.Orientation.y + SETUP.RelHeadingAngle);

		// Get front crawl edge hang descent climb context.
		auto attracColl = GetEdgeHangDescecntClimbAttractorCollision(item, coll, SETUP, attracColls);
		if (attracColl.has_value())
		{
			auto context = ClimbContextData{};
			context.Attractor = attracColl->Attractor;
			context.PathDistance = attracColl->PathDistance;
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
			ANGLE(-180.0f),					  // Relative heading angle.
			-MAX_HEIGHT, LARA_HEIGHT_STRETCH, // Edge height bounds.
			-(int)CLICK(0.6f)				  // Edge-to-ceil height lower bound.
		};

		// Get attractor collisions.
		auto attracColls = GetAttractorCollisions(item.Pose.Position.ToVector3(), ATTRAC_DETECT_RADIUS, item.RoomNumber, item.Pose.Orientation.y + SETUP.RelHeadingAngle);

		// Get back crawl edge hang descent climb context.
		auto attracColl = GetEdgeHangDescecntClimbAttractorCollision(item, coll, SETUP, attracColls);
		if (attracColl.has_value())
		{
			auto context = ClimbContextData{};
			context.Attractor = attracColl->Attractor;
			context.PathDistance = attracColl->PathDistance;
			context.RelPosOffset = Vector3(0.0f, 0.0f, coll.Setup.Radius);
			context.RelOrientOffset = EulerAngles::Identity;
			context.TargetStateID = LS_CRAWL_EDGE_HANG_DESCENT_BACK;
			context.AlignType = ClimbContextAlignType::AttractorParent;
			context.IsJump = false;

			return context;
		}

		return std::nullopt;
	}
}
