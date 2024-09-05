#include "framework.h"
#include "Game/Lara/Context/Vault.h"

#include "Game/animation.h"
#include "Game/collision/Attractor.h"
#include "Game/collision/collide_item.h"
#include "Game/collision/collide_room.h"
#include "Game/collision/Point.h"
#include "Game/control/los.h"
#include "Game/items.h"
#include "Game/Lara/Context/Structs.h"
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
	bool CanVaultFromSprint(const ItemInfo& item, const CollisionInfo& coll)
	{
		return !TestLaraWall(&item, OFFSET_RADIUS(coll.Setup.Radius), -BLOCK(5 / 8.0f));
	}

	static std::optional<AttractorCollisionData> GetEdgeVaultClimbAttractorCollision(const ItemInfo& item, const CollisionInfo& coll,
																					 const EdgeVaultClimbSetupData& setup,
																					 const std::vector<AttractorCollisionData>& attracColls)
	{
		constexpr auto SWAMP_DEPTH_MAX				= -CLICK(3);
		constexpr auto REL_SURFACE_HEIGHT_THRESHOLD = CLICK(0.5f);

		// HACK: Offset required for proper bridge surface height detection. Floordata should be revised for proper handling.
		constexpr auto PROBE_POINT_OFFSET = Vector3(0.0f, -CLICK(1), 0.0f);

		const auto& player = GetLaraInfo(item);

		// 1) Test swamp depth (if applicable).
		if (setup.TestSwampDepth)
		{
			if (TestEnvironment(ENV_FLAG_SWAMP, item.RoomNumber) && player.Context.WaterSurfaceDist < SWAMP_DEPTH_MAX)
				return std::nullopt;
		}

		int sign = setup.TestEdgeFront ? 1 : -1;
		float range2D = std::max<float>(OFFSET_RADIUS(coll.Setup.Radius), Vector2(item.Animation.Velocity.x, item.Animation.Velocity.z).Length());
		const AttractorCollisionData* highestAttracColl = nullptr;

		// 2) Assess attractor collision.
		for (const auto& attracColl : attracColls)
		{
			// 2.1) Check attractor type.
			if (attracColl.Attractor->GetType() != AttractorType::Edge &&
				attracColl.Attractor->GetType() != AttractorType::WallEdge)
			{
				continue;
			}

			// 2.2) Test if edge is within 2D range.
			if (attracColl.Distance2D > range2D)
				continue;

			// 2.3) Test if edge slope is illegal.
			if (abs(attracColl.SlopeAngle) >= STEEP_FLOOR_SLOPE_ANGLE)
				continue;

			// 2.4) Test edge angle relation.
			if (!attracColl.IsInFront ||
				!TestPlayerInteractAngle(item.Pose.Orientation.y, attracColl.HeadingAngle + (setup.TestEdgeFront ? ANGLE(0.0f) : ANGLE(180.0f))))
			{
				continue;
			}

			// TODO: Point collision probing may traverse rooms incorrectly when attractors cross rooms.
			// Solution is to probe from player's position and room. Combine player/intersect RelDeltaPos and RelPosOffset.

			// Get point collision at edge.
			auto pointCollCenter = GetPointCollision(
				Vector3i(attracColl.Intersection.x, attracColl.Intersection.y - 1, attracColl.Intersection.z),
				attracColl.Attractor->GetRoomNumber(),
				attracColl.HeadingAngle, -coll.Setup.Radius, PROBE_POINT_OFFSET.y);

			// TODO: Rotating platforms don't exist yet, so this is hypothetical.
			// 2.5) Test if intersection is blocked by ceiling.
			if (attracColl.Intersection.y <= pointCollCenter.GetCeilingHeight())
				continue;

			// Get point collision behind edge.
			auto pointCollBack = GetPointCollision(
				attracColl.Intersection, attracColl.Attractor->GetRoomNumber(),
				attracColl.HeadingAngle, -coll.Setup.Radius, PROBE_POINT_OFFSET.y);

			bool isTreadingWater = (player.Control.WaterStatus == WaterStatus::TreadWater);

			// 2.6) Test if relative edge height is within edge intersection bounds. NOTE: Special case for water tread.
			int relEdgeHeight = (attracColl.Intersection.y - (isTreadingWater ? pointCollBack.GetWaterSurfaceHeight() : pointCollBack.GetFloorHeight())) * sign;
			if (relEdgeHeight >= setup.LowerEdgeBound ||
				relEdgeHeight < setup.UpperEdgeBound)
			{
				continue;
			}

			// 2.7) Test if player vertical position is within surface threshold. NOTE: Special case for water tread.
			int surfaceHeight = isTreadingWater ? pointCollBack.GetWaterSurfaceHeight() : (setup.TestEdgeFront ? pointCollBack.GetFloorHeight() : attracColl.Intersection.y);
			int relPlayerSurfaceHeight = abs(item.Pose.Position.y - surfaceHeight);
			if (relPlayerSurfaceHeight > REL_SURFACE_HEIGHT_THRESHOLD)
				continue;

			// 2.8) Test if ceiling behind is adequately higher than edge.
			int edgeToCeilHeight = pointCollBack.GetCeilingHeight() - attracColl.Intersection.y;
			if (edgeToCeilHeight > setup.LowerEdgeToCeilBound)
				continue;

			// 2.9) Test if bridge blocks path.
			if (GetPointCollision(item).GetFloorBridgeItemNumber() != pointCollBack.GetFloorBridgeItemNumber())
				continue;

			// Get point collision in front of edge.
			auto pointCollFront = GetPointCollision(
				attracColl.Intersection, attracColl.Attractor->GetRoomNumber(),
				attracColl.HeadingAngle, coll.Setup.Radius, PROBE_POINT_OFFSET.y);

			// Test destination space (if applicable).
			if (setup.TestDestSpace)
			{
				// TODO: Doesn't detect walls properly.

				// Get point collisions at destination.
				auto& destPointCollCenter = setup.TestEdgeFront ? pointCollFront : pointCollBack;
				auto destPointCollLeft = GetPointCollision(destPointCollCenter.GetPosition(), destPointCollCenter.GetRoomNumber(), attracColl.HeadingAngle, 0.0f, 0.0f, -coll.Setup.Radius);
				auto destPointCollRight = GetPointCollision(destPointCollCenter.GetPosition(), destPointCollCenter.GetRoomNumber(), attracColl.HeadingAngle, 0.0f, 0.0f, coll.Setup.Radius);

				// Calculate destination floor-to-ceiling heights.
				int destFloorToCeilHeightCenter = abs(destPointCollCenter.GetCeilingHeight() - destPointCollCenter.GetFloorHeight());
				int destFloorToCeilHeightLeft = abs(destPointCollLeft.GetCeilingHeight() - destPointCollLeft.GetFloorHeight());
				int destFloorToCeilHeightRight = abs(destPointCollRight.GetCeilingHeight() - destPointCollRight.GetFloorHeight());

				// 2.10) Test destination floor-to-ceiling heights.
				if (destFloorToCeilHeightCenter <= setup.DestFloorToCeilHeightMin || destFloorToCeilHeightCenter > setup.DestFloorToCeilHeightMax ||
					destFloorToCeilHeightLeft <= setup.DestFloorToCeilHeightMin || destFloorToCeilHeightLeft > setup.DestFloorToCeilHeightMax ||
					destFloorToCeilHeightRight <= setup.DestFloorToCeilHeightMin || destFloorToCeilHeightRight > setup.DestFloorToCeilHeightMax)
				{
					continue;
				}

				// 2.11) Test destination floor-to-edge height if approaching from front.
				if (setup.TestEdgeFront)
				{
					int destFloorToEdgeHeight = abs(attracColl.Intersection.y - destPointCollCenter.GetFloorHeight());
					if (destFloorToEdgeHeight > REL_SURFACE_HEIGHT_THRESHOLD)
						continue;
				}

				// 2.12) Test for object obstruction.
				if (TestForObjectOnLedge(attracColl, coll.Setup.Radius, (setup.DestFloorToCeilHeightMin - CLICK(1)) * -sign, setup.TestEdgeFront))
					continue;
			}

			// 2.13) Test for steep floor at destination (if applicable).
			if (setup.TestDestSteepFloor)
			{
				if (setup.TestEdgeFront ? pointCollFront.IsSteepFloor() : pointCollBack.IsSteepFloor())
					continue;
			}

			// 2.14) Track highest or return lowest attractor collision.
			if (setup.FindHighest)
			{
				if (highestAttracColl == nullptr)
				{
					highestAttracColl = &attracColl;
					continue;
				}

				if (attracColl.Intersection.y < highestAttracColl->Intersection.y)
				{
					// Ensure attractors are stacked exactly.
					auto highest2DIntersect = Vector2(highestAttracColl->Intersection.x, highestAttracColl->Intersection.z);
					auto current2DIntersect = Vector2(attracColl.Intersection.x, attracColl.Intersection.z);
					if (Vector2::DistanceSquared(highest2DIntersect, current2DIntersect) > EPSILON)
						continue;

					highestAttracColl = &attracColl;
				}
			}
			else
			{
				return attracColl;
			}
		}

		// Return highest attractor collision (if applicable).
		if (highestAttracColl != nullptr)
			return *highestAttracColl;

		// No valid edge attractor collision; return nullopt.
		return std::nullopt;
	}

	static std::optional<ClimbContextData> GetStandVault2StepsUpClimbContext(const ItemInfo& item, const CollisionInfo& coll,
																			 const std::vector<AttractorCollisionData>& attracColls)
	{
		constexpr auto SETUP = EdgeVaultClimbSetupData
		{
			-STEPUP_HEIGHT, -(int)CLICK(2.5f), // Edge height bounds.
			-CLICK(1),						   // Edge-to-ceil height lower bound.
			LARA_HEIGHT, -MAX_HEIGHT,		   // Destination floor-to-ceil range.
			false,							   // Find highest.
			false,							   // Test swamp depth.
			true,							   // Test edge front.
			true,							   // Test ledge heights.
			true							   // Test ledge steep floor.
		};
		constexpr auto VERTICAL_OFFSET = CLICK(2);

		// Get standing vault climb context.
		auto attracColl = GetEdgeVaultClimbAttractorCollision(item, coll, SETUP, attracColls);
		if (attracColl.has_value())
		{
			auto context = ClimbContextData{};
			context.Attractor = attracColl->Attractor;
			context.PathDistance = attracColl->PathDistance;
			context.RelPosOffset = Vector3(0.0f, VERTICAL_OFFSET, -coll.Setup.Radius);
			context.RelOrientOffset = EulerAngles::Identity;
			context.TargetStateID = LS_STAND_VAULT_2_STEPS_UP;
			context.AlignType = ClimbContextAlignType::AttractorParent;
			context.IsJump = false;

			return context;
		}

		return std::nullopt;
	}

	static std::optional<ClimbContextData> GetStandVault3StepsUpClimbContext(const ItemInfo& item, const CollisionInfo& coll,
																			 const std::vector<AttractorCollisionData>& attracColls)
	{
		constexpr auto SETUP = EdgeVaultClimbSetupData
		{
			-(int)CLICK(2.5f), -(int)CLICK(3.5f), // Edge height bounds.
			-CLICK(1),							  // Edge-to-ceil height lower bound.
			LARA_HEIGHT, -MAX_HEIGHT,			  // Destination floor-to-ceil range.
			false,								  // Find highest.
			false,								  // Test swamp depth.
			true,								  // Test edge front.
			true,								  // Test ledge heights.
			true								  // Test ledge steep floor.
		};
		constexpr auto VERTICAL_OFFSET = CLICK(3);

		// Get standing vault climb context.
		auto attracColl = GetEdgeVaultClimbAttractorCollision(item, coll, SETUP, attracColls);
		if (attracColl.has_value())
		{
			auto context = ClimbContextData{};
			context.Attractor = attracColl->Attractor;
			context.PathDistance = attracColl->PathDistance;
			context.RelPosOffset = Vector3(0.0f, VERTICAL_OFFSET, -coll.Setup.Radius);
			context.RelOrientOffset = EulerAngles::Identity;
			context.TargetStateID = LS_STAND_VAULT_3_STEPS_UP;
			context.AlignType = ClimbContextAlignType::AttractorParent;
			context.IsJump = false;

			return context;
		}

		return std::nullopt;
	}

	static std::optional<ClimbContextData> GetStandVault1StepUpToCrouchClimbContext(const ItemInfo& item, const CollisionInfo& coll,
																					const std::vector<AttractorCollisionData>& attracColls)
	{
		constexpr auto SETUP = EdgeVaultClimbSetupData
		{
			0, -STEPUP_HEIGHT,				// Edge height bounds.
			-CLICK(1),						// Edge-to-ceil height lower bound.
			LARA_HEIGHT_CRAWL, LARA_HEIGHT, // Destination floor-to-ceil range.
			false,							// Find highest.
			false,							// Test swamp depth.
			true,							// Test edge front.
			true,							// Test ledge heights.
			true							// Test ledge steep floor.
		};
		constexpr auto VERTICAL_OFFSET = CLICK(1);

		// Get standing vault climb context.
		auto attracColl = GetEdgeVaultClimbAttractorCollision(item, coll, SETUP, attracColls);
		if (attracColl.has_value())
		{
			auto context = ClimbContextData{};
			context.Attractor = attracColl->Attractor;
			context.PathDistance = attracColl->PathDistance;
			context.RelPosOffset = Vector3(0.0f, VERTICAL_OFFSET, -coll.Setup.Radius);
			context.RelOrientOffset = EulerAngles::Identity;
			context.TargetStateID = LS_STAND_VAULT_1_STEP_UP_TO_CROUCH;
			context.AlignType = ClimbContextAlignType::AttractorParent;
			context.IsJump = false;

			return context;
		}

		return std::nullopt;
	}

	static std::optional<ClimbContextData> GetStandVault2StepsUpToCrouchClimbContext(const ItemInfo& item, const CollisionInfo& coll,
																					 const std::vector<AttractorCollisionData>& attracColls)
	{
		constexpr auto SETUP = EdgeVaultClimbSetupData
		{
			-STEPUP_HEIGHT, -(int)CLICK(2.5f), // Edge height bounds.
			-CLICK(1),						   // Edge-to-ceil height lower bound.
			LARA_HEIGHT_CRAWL, LARA_HEIGHT,	   // Destination floor-to-ceil range.
			false,							   // Find highest.
			false,							   // Test swamp depth.
			true,							   // Test edge front.
			true,							   // Test ledge heights.
			true							   // Test ledge steep floor.
		};
		constexpr auto VERTICAL_OFFSET = CLICK(2);

		// Get standing vault climb context.
		auto attracColl = GetEdgeVaultClimbAttractorCollision(item, coll, SETUP, attracColls);
		if (attracColl.has_value())
		{
			auto context = ClimbContextData{};
			context.Attractor = attracColl->Attractor;
			context.PathDistance = attracColl->PathDistance;
			context.RelPosOffset = Vector3(0.0f, VERTICAL_OFFSET, -coll.Setup.Radius);
			context.RelOrientOffset = EulerAngles::Identity;
			context.TargetStateID = LS_STAND_VAULT_2_STEPS_UP_TO_CROUCH;
			context.AlignType = ClimbContextAlignType::AttractorParent;
			context.IsJump = false;

			return context;
		}

		return std::nullopt;
	}

	static std::optional<ClimbContextData> GetStandVault3StepsUpToCrouchClimbContext(const ItemInfo& item, const CollisionInfo& coll,
																					 const std::vector<AttractorCollisionData>& attracColls)
	{
		constexpr auto SETUP = EdgeVaultClimbSetupData
		{
			-(int)CLICK(2.5f), -(int)CLICK(3.5f), // Edge height bounds.
			-CLICK(1),							  // Edge-to-ceil height lower bound.
			LARA_HEIGHT_CRAWL, LARA_HEIGHT,		  // Destination floor-to-ceil range.
			false,								  // Find highest.
			false,								  // Test swamp depth.
			true,								  // Test edge front.
			true,								  // Test ledge heights.
			true								  // Test ledge steep floor.
		};
		constexpr auto VERTICAL_OFFSET = CLICK(3);

		// Get standing vault climb context.
		auto attracColl = GetEdgeVaultClimbAttractorCollision(item, coll, SETUP, attracColls);
		if (attracColl.has_value())
		{
			auto context = ClimbContextData{};
			context.Attractor = attracColl->Attractor;
			context.PathDistance = attracColl->PathDistance;
			context.RelPosOffset = Vector3(0.0f, VERTICAL_OFFSET, -coll.Setup.Radius);
			context.RelOrientOffset = EulerAngles::Identity;
			context.TargetStateID = LS_STAND_VAULT_3_STEPS_UP_TO_CROUCH;
			context.AlignType = ClimbContextAlignType::AttractorParent;
			context.IsJump = false;

			return context;
		}

		return std::nullopt;
	}

	static std::optional<ClimbContextData> GetAutoJumpClimbContext(const ItemInfo& item, const CollisionInfo& coll,
																   const std::vector<AttractorCollisionData>& attracColls)
	{
		constexpr auto LOWER_CEIL_BOUND = -LARA_HEIGHT_MONKEY;
		constexpr auto UPPER_CEIL_BOUND = -CLICK(7);

		constexpr auto SETUP = EdgeVaultClimbSetupData
		{
			-(int)CLICK(3.5f), -(int)CLICK(7.5f), // Edge height bounds.
			-(int)CLICK(1 / 256.0f),			  // Edge-to-ceil height minumum. // TODO: Sloped ceilings are ignored.
			0, -MAX_HEIGHT,						  // Destination floor-to-ceil range.
			true,								  // Find highest.
			false,								  // Test swamp depth.
			true,								  // Test edge front.
			false,								  // Test ledge heights.
			false								  // Test ledge steep floor.
		};

		const auto& player = GetLaraInfo(item);

		// 1) Get edge auto jump climb context.
		auto attracColl = GetEdgeVaultClimbAttractorCollision(item, coll, SETUP, attracColls);
		if (attracColl.has_value())
		{
			int relEdgeHeight = attracColl->Intersection.y - item.Pose.Position.y;

			auto context = ClimbContextData{};
			context.Attractor = attracColl->Attractor;
			context.PathDistance = attracColl->PathDistance;
			context.RelPosOffset = Vector3(0.0f, -relEdgeHeight, -coll.Setup.Radius);
			context.RelOrientOffset = EulerAngles::Identity;
			context.TargetStateID = LS_AUTO_JUMP;
			context.AlignType = ClimbContextAlignType::OffsetBlend;
			context.IsJump = true;

			return context;
		}

		// Auto jump to monkey swing disabled; return nullopt.
		if (!g_Configuration.EnableAutoMonkeySwingJump)
			return std::nullopt;

		// Get point collision.
		auto pointColl = GetPointCollision(item);
		int relCeilHeight = pointColl.GetCeilingHeight() - item.Pose.Position.y;

		// 2) Get auto jump to monkey swing climb context.
		if (player.Control.CanMonkeySwing &&	// Player is standing below monkey swing.
			relCeilHeight < LOWER_CEIL_BOUND && // Ceiling height is within lower ceiling bound.
			relCeilHeight >= UPPER_CEIL_BOUND)	// Ceiling height is within upper ceiling bound.
		{
			auto context = ClimbContextData{};
			context.Attractor = nullptr;
			context.PathDistance = 0.0f;
			context.RelPosOffset = Vector3(0.0f, -relCeilHeight, 0.0f);
			context.RelOrientOffset = EulerAngles::Identity;
			context.TargetStateID = LS_AUTO_JUMP;
			context.AlignType = ClimbContextAlignType::None;
			context.IsJump = true;

			return context;
		}

		return std::nullopt;
	}

	// TODO: pass setup struct.
	static std::optional<AttractorCollisionData> GetClimbableWallMountAttractorCollision(const ItemInfo& item, const CollisionInfo& coll,
																						 const WallEdgeMountClimbSetupData& setup,
																						 const std::vector<AttractorCollisionData>& attracColls)
	{
		const auto& player = GetLaraInfo(item);

		float range2D = std::max<float>(OFFSET_RADIUS(coll.Setup.Radius), Vector2(item.Animation.Velocity.x, item.Animation.Velocity.z).Length());

		// Assess attractor collision.
		for (auto& attracColl : attracColls)
		{
			// 1) Check attractor type.
			if (attracColl.Attractor->GetType() != AttractorType::WallEdge)
				continue;

			// 2) Test if edge is within 2D range.
			if (attracColl.Distance2D > range2D)
				continue;

			// 3) Test if wall edge is steep.
			if (abs(attracColl.SlopeAngle) >= STEEP_FLOOR_SLOPE_ANGLE)
				continue;

			// 4) Test wall edge angle relation.
			if (!attracColl.IsInFront ||
				!TestPlayerInteractAngle(item.Pose.Orientation.y, attracColl.HeadingAngle))
			{
				continue;
			}

			// Get point collision behind wall edge.
			auto pointCollBack = GetPointCollision(
				attracColl.Intersection, attracColl.Attractor->GetRoomNumber(),
				attracColl.HeadingAngle, -coll.Setup.Radius);

			// TODO: Test bridge consistency below player and below edge?
			// 5) Test if edge is blocked by floor.
			//if ()
			//	continue;

			// ceiling height max.

			bool isTreadingWater = (player.Control.WaterStatus == WaterStatus::TreadWater);

			// 6) Test if relative edge height is within edge intersection bounds. NOTE: Special case for water tread.
			int relEdgeHeight = attracColl.Intersection.y - (isTreadingWater ? pointCollBack.GetWaterSurfaceHeight() : pointCollBack.GetFloorHeight());
			if (relEdgeHeight >= setup.LowerEdgeBound ||
				relEdgeHeight < setup.UpperEdgeBound)
			{
				continue;
			}

			// TODO: collect stacked WallEdge attractors.

			// 7) Test if ceiling behind is adequately higher than edge.
			int edgeToCeilHeight = pointCollBack.GetCeilingHeight() - attracColl.Intersection.y;
			if (edgeToCeilHeight > setup.LowerEdgeToCeilBound)
				continue;

			// TODO: Test front point collision to see if climbable wall is floating in air and therefore invalid.

			return attracColl;
		}

		// Collect attractor collisions ordered by height.
		// Assess 5, 6 on top one only.

		// No valid wall edge attractor collision; return nullopt.
		return std::nullopt;
	}

	static std::optional<ClimbContextData> GetClimbableWallMountClimbContext(const ItemInfo& item, const CollisionInfo& coll,
																			 const std::vector<AttractorCollisionData>& attracColls)
	{
		// TODO: Add label comments.
		constexpr auto SETUP = WallEdgeMountClimbSetupData
		{
			(int)-CLICK(3.5f), (int)-CLICK(4.5f),
			CLICK(1),
			MAX_HEIGHT							  // Ceiling height max.
		};
		constexpr auto SWAMP_DEPTH_MAX = -CLICK(3);
		constexpr auto VERTICAL_OFFSET = CLICK(4);

		const auto& player = GetLaraInfo(item);

		// 1) Test swamp depth.
		if (TestEnvironment(ENV_FLAG_SWAMP, item.RoomNumber) && player.Context.WaterSurfaceDist < SWAMP_DEPTH_MAX)
			return std::nullopt;

		// 2) Get climbable wall mount climb context.
		auto attracColl = GetClimbableWallMountAttractorCollision(item, coll, SETUP, attracColls);
		if (attracColl.has_value())
		{
			auto context = ClimbContextData{};
			context.Attractor = attracColl->Attractor;
			context.PathDistance = attracColl->PathDistance;
			context.RelPosOffset = Vector3(0.0f, VERTICAL_OFFSET, -coll.Setup.Radius);
			context.RelOrientOffset = EulerAngles::Identity;
			context.TargetStateID = LS_WALL_CLIMB_IDLE;
			context.AlignType = ClimbContextAlignType::AttractorParent;
			context.IsJump = false;

			return context;
		}

		return std::nullopt;
	}

	std::optional<ClimbContextData> GetStandVaultClimbContext(const ItemInfo& item, const CollisionInfo& coll)
	{
		constexpr auto ATTRAC_DETECT_RADIUS = BLOCK(2);

		const auto& player = GetLaraInfo(item);

		// Check hand status.
		if (player.Control.HandStatus != HandStatus::Free)
			return std::nullopt;

		// Get attractor collisions.
		auto attracColls = GetAttractorCollisions(item.Pose.Position.ToVector3(), ATTRAC_DETECT_RADIUS, item.RoomNumber, item.Pose.Orientation.y);

		auto context = std::optional<ClimbContextData>();

		// 1) Vault 1 step up to crouch.
		context = GetStandVault1StepUpToCrouchClimbContext(item, coll, attracColls);
		if (context.has_value())
		{
			if (HasStateDispatch(&item, context->TargetStateID))
				return context;
		}

		// 2) Vault 2 steps up to crouch.
		context = GetStandVault2StepsUpClimbContext(item, coll, attracColls);
		if (context.has_value())
		{
			if (HasStateDispatch(&item, context->TargetStateID))
				return context;
		}

		// 3) Vault 2 steps up.
		context = GetStandVault2StepsUpToCrouchClimbContext(item, coll, attracColls);
		if (context.has_value())
		{
			if (HasStateDispatch(&item, context->TargetStateID))
				return context;
		}

		// 4) Vault 3 steps up to crouch.
		context = GetStandVault3StepsUpToCrouchClimbContext(item, coll, attracColls);
		if (context.has_value())
		{
			if (HasStateDispatch(&item, context->TargetStateID))
				return context;
		}

		// 5) Vault 3 steps up.
		context = GetStandVault3StepsUpClimbContext(item, coll, attracColls);
		if (context.has_value())
		{
			if (HasStateDispatch(&item, context->TargetStateID))
				return context;
		}

		// 6) Mount climbable wall.
		context = GetClimbableWallMountClimbContext(item, coll, attracColls);
		if (context.has_value())
		{
			if (HasStateDispatch(&item, context->TargetStateID))
				return context;
		}

		// 7) Auto jump.
		context = GetAutoJumpClimbContext(item, coll, attracColls);
		if (context.has_value())
		{
			if (HasStateDispatch(&item, context->TargetStateID))
				return context;
		}

		return std::nullopt;
	}

	static std::optional<ClimbContextData> GetCrawlVault1StepDownClimbContext(const ItemInfo& item, const CollisionInfo& coll,
																			  const std::vector<AttractorCollisionData>& attracColls)
	{
		constexpr auto SETUP = EdgeVaultClimbSetupData
		{
			STEPUP_HEIGHT, CRAWL_STEPUP_HEIGHT, // Edge height bounds.
			-(int)CLICK(0.6f),					// Edge-to-ceil height lower bound.
			LARA_HEIGHT_CRAWL, -MAX_HEIGHT,		// Destination floor-to-ceil range.
			false,								// Find highest.
			false,								// Test swamp depth.
			false,								// Test edge front.
			true,								// Test ledge heights.
			true								// Test ledge steep floor.
		};

		// Get crawling vault climb context.
		auto attracColl = GetEdgeVaultClimbAttractorCollision(item, coll, SETUP, attracColls);
		if (attracColl.has_value())
		{
			auto context = ClimbContextData{};
			context.Attractor = attracColl->Attractor;
			context.PathDistance = attracColl->PathDistance;
			context.RelPosOffset = Vector3(0.0f, 0.0f, -coll.Setup.Radius);
			context.RelOrientOffset = EulerAngles(0, ANGLE(180.0f), 0);
			context.TargetStateID = LS_CRAWL_VAULT_1_STEP_DOWN;
			context.AlignType = ClimbContextAlignType::OffsetBlend;
			context.IsJump = false;

			return context;
		}

		return std::nullopt;
	}

	static std::optional<ClimbContextData> GetCrawlVault1StepDownToStandClimbContext(const ItemInfo& item, const CollisionInfo& coll,
																					 const std::vector<AttractorCollisionData>& attracColls)
	{
		constexpr auto SETUP = EdgeVaultClimbSetupData
		{
			STEPUP_HEIGHT, -CRAWL_STEPUP_HEIGHT, // Edge height bounds.
			-(int)CLICK(1.25f),					 // Edge-to-ceil height lower bound.
			LARA_HEIGHT, -MAX_HEIGHT,			 // Destination floor-to-ceil range.
			false,								 // Find highest.
			false,								 // Test swamp depth.
			false,								 // Test edge front.
			true,								 // Test ledge heights.
			false								 // Test ledge steep floor.
		};

		// Crouch action held; return nullopt.
		if (IsHeld(In::Crouch))
			return std::nullopt;

		// Get crawling vault climb context.
		auto attracColl = GetEdgeVaultClimbAttractorCollision(item, coll, SETUP, attracColls);
		if (attracColl.has_value())
		{
			auto context = ClimbContextData{};
			context.Attractor = attracColl->Attractor;
			context.RelPosOffset = Vector3(0.0f, 0.0f, -coll.Setup.Radius);
			context.RelOrientOffset = EulerAngles(0, ANGLE(180.0f), 0);
			context.PathDistance = attracColl->PathDistance;
			context.TargetStateID = LS_CRAWL_VAULT_1_STEP_DOWN_TO_STAND;
			context.AlignType = ClimbContextAlignType::OffsetBlend;
			context.IsJump = false;

			return context;
		}

		return std::nullopt;
	}

	static std::optional<ClimbContextData> GetCrawlVault1StepUpClimbContext(const ItemInfo& item, const CollisionInfo& coll,
																			const std::vector<AttractorCollisionData>& attracColls)
	{
		constexpr auto SETUP = EdgeVaultClimbSetupData
		{
			-CRAWL_STEPUP_HEIGHT, -STEPUP_HEIGHT, // Edge height bounds.
			-(int)CLICK(0.6f),					  // Edge-to-ceil height lower bound.
			LARA_HEIGHT_CRAWL, -MAX_HEIGHT,		  // Destination floor-to-ceil range.
			false,								  // Find highest.
			false,								  // Test swamp depth.
			true,								  // Test edge front.
			true,								  // Test ledge heights.
			true								  // Test ledge steep floor.
		};
		constexpr auto VERTICAL_OFFSET = CLICK(1);

		// Get crawling vault climb context.
		auto attracColl = GetEdgeVaultClimbAttractorCollision(item, coll, SETUP, attracColls);
		if (attracColl.has_value())

		{
			auto context = ClimbContextData{};
			context.Attractor = attracColl->Attractor;
			context.PathDistance = attracColl->PathDistance;
			context.RelPosOffset = Vector3(0.0f, VERTICAL_OFFSET, -coll.Setup.Radius);
			context.RelOrientOffset = EulerAngles::Identity;
			context.TargetStateID = LS_CRAWL_VAULT_1_STEP_UP;
			context.AlignType = ClimbContextAlignType::AttractorParent;
			context.IsJump = false;

			return context;
		}

		return std::nullopt;
	}

	static std::optional<ClimbContextData> GetCrawlVaultJumpClimbContext(const ItemInfo& item, const CollisionInfo& coll,
																		 const std::vector<AttractorCollisionData>& attracColls)
	{
		constexpr auto SETUP = EdgeVaultClimbSetupData
		{
			NO_LOWER_BOUND, STEPUP_HEIGHT, // Edge height bounds.
			-(int)CLICK(1.25f),			   // Edge-to-ceil height lower bound.
			LARA_HEIGHT, -MAX_HEIGHT,	   // Destination floor-to-ceil range.
			false,						   // Find highest.
			false,						   // Test swamp depth.
			false,						   // Test edge front.
			true,						   // Test ledge heights.
			false						   // Test ledge steep floor.
		};

		// 1) Get edge crawl vault climb context.
		auto attracColl = GetEdgeVaultClimbAttractorCollision(item, coll, SETUP, attracColls);
		if (attracColl.has_value())
		{
			auto context = ClimbContextData{};
			context.Attractor = attracColl->Attractor;
			context.RelPosOffset = Vector3(0.0f, 0.0f, -coll.Setup.Radius);
			context.RelOrientOffset = EulerAngles(0, ANGLE(180.0f), 0);
			context.PathDistance = attracColl->PathDistance;
			context.TargetStateID = IsHeld(In::Walk) ? LS_CRAWL_VAULT_JUMP_FLIP : LS_CRAWL_VAULT_JUMP;
			context.AlignType = ClimbContextAlignType::OffsetBlend;
			context.IsJump = false;

			return context;
		}

		auto pointCollCenter = GetPointCollision(item);
		auto pointCollFront = GetPointCollision(item, item.Pose.Orientation.y, BLOCK(0.25f), -coll.Setup.Height);
		int relFloorHeight = pointCollFront.GetFloorHeight() - item.Pose.Position.y;

		// TODO
		// 2) Get steep floor crawl vault climb context (special case).
		if (pointCollFront.IsSteepFloor() &&
			true
			/*relFloorHeight <= SETUP.LowerEdgeBound &&							// Within lower floor bound.
			relFloorHeight >= SETUP.UpperEdgeBound &&							// Within upper floor bound.

			abs(pointCollFront.GetCeilingHeight() - pointCollFront.GetFloorHeight()) > testSetup.ClampMin &&		// Crossing clamp limit.
			abs(pointCollCenter.GetCeilingHeight() - probeB.GetFloorHeight()) > testSetup.ClampMin &&		// Destination clamp limit.
			abs(probeMiddle.GetCeilingHeight() - pointCollFront.GetFloorHeight()) >= testSetup.GapMin &&	// Gap is optically permissive (going up).
			abs(probeA.GetCeilingHeight() - probeMiddle.GetFloorHeight()) >= testSetup.GapMin &&	// Gap is optically permissive (going down).
			abs(probeA.GetFloorHeight() - probeB.GetFloorHeight()) <= testSetup.FloorBound &&		// Crossing/destination floor height difference suggests continuous crawl surface.
			(probeA.GetCeilingHeight() - y) < -testSetup.GapMin*/)									// Ceiling height is permissive.
		{
			auto context = ClimbContextData{};
			context.Attractor = nullptr;
			context.PathDistance = 0.0f;
			context.RelPosOffset = Vector3::Zero;
			context.RelOrientOffset = EulerAngles::Identity;
			context.TargetStateID = LS_CRAWL_VAULT_JUMP;
			context.AlignType = ClimbContextAlignType::None;
			context.IsJump = false;

			return context;
		}

		return std::nullopt;
	}

	std::optional<ClimbContextData> GetCrawlVaultClimbContext(const ItemInfo& item, const CollisionInfo& coll)
	{
		constexpr auto ATTRAC_DETECT_RADIUS = BLOCK(0.5f);

		const auto& player = GetLaraInfo(item);

		// Extended crawl moveset disabled; return nullopt.
		if (!g_GameFlow->HasCrawlExtended())
			return std::nullopt;

		// Get attractor collisions.
		auto attracColls = GetAttractorCollisions(item.Pose.Position.ToVector3(), ATTRAC_DETECT_RADIUS, item.RoomNumber, item.Pose.Orientation.y);

		auto context = std::optional<ClimbContextData>();

		// 1) Vault down 1 step to stand.
		context = GetCrawlVault1StepDownToStandClimbContext(item, coll, attracColls);
		if (context.has_value())
		{
			if (HasStateDispatch(&item, context->TargetStateID))
				return context;
		}

		// 2) Vault down 1 step.
		context = GetCrawlVault1StepDownClimbContext(item, coll, attracColls);
		if (context.has_value())
		{
			if (HasStateDispatch(&item, context->TargetStateID))
				return context;
		}

		// 3) Vault up 1 step.
		context = GetCrawlVault1StepUpClimbContext(item, coll, attracColls);
		if (context.has_value())
		{
			if (HasStateDispatch(&item, context->TargetStateID))
				return context;
		}

		// 4) Jump off.
		if (IsHeld(In::Jump))
		{
			context = GetCrawlVaultJumpClimbContext(item, coll, attracColls);
			if (context.has_value())
			{
				if (HasStateDispatch(&item, context->TargetStateID))
					return context;
			}
		}

		return std::nullopt;
	}

	static std::optional<ClimbContextData> GetTreadWaterVault1StepDownToStandClimbContext(const ItemInfo& item, const CollisionInfo& coll,
																						  const std::vector<AttractorCollisionData>& attracColls)
	{
		constexpr auto SETUP = EdgeVaultClimbSetupData
		{
			STEPUP_HEIGHT, (int)CLICK(0.5f), // Edge height bounds.
			-CLICK(1),						 // Edge-to-ceil height lower bound.
			LARA_HEIGHT, -MAX_HEIGHT,		 // Destination floor-to-ceil range.
			false,							 // Find highest.
			false,							 // Test swamp depth.
			true,							 // Test edge front.
			true,							 // Test ledge heights.
			true							 // Test ledge steep floor.
		};
		constexpr auto VERTICAL_OFFSET = -CLICK(1);

		// Crouch action held and extended crawl moveset enabled; return nullopt.
		if (IsHeld(In::Crouch) && g_GameFlow->HasCrawlExtended())
			return std::nullopt;

		// Get tread water vault climb context.
		auto attracColl = GetEdgeVaultClimbAttractorCollision(item, coll, SETUP, attracColls);
		if (attracColl.has_value())
		{
			auto context = ClimbContextData{};
			context.Attractor = attracColl->Attractor;
			context.PathDistance = attracColl->PathDistance;
			context.RelPosOffset = Vector3(0.0f, VERTICAL_OFFSET, -coll.Setup.Radius);
			context.RelOrientOffset = EulerAngles::Identity;
			context.TargetStateID = LS_TREAD_WATER_VAULT_1_STEP_DOWN_TO_STAND;
			context.AlignType = ClimbContextAlignType::AttractorParent;
			context.IsJump = false;

			return context;
		}

		return std::nullopt;
	}

	static std::optional<ClimbContextData> GetTreadWaterVault0StepsToStandClimbContext(const ItemInfo& item, const CollisionInfo& coll,
																					   const std::vector<AttractorCollisionData>& attracColls)
	{
		constexpr auto SETUP = EdgeVaultClimbSetupData
		{
			(int)CLICK(0.5f), -(int)CLICK(0.5f), // Edge height bounds.
			-CLICK(1),							 // Edge-to-ceil height lower bound.
			LARA_HEIGHT, -MAX_HEIGHT,			 // Destination floor-to-ceil range.
			false,								 // Find highest.
			false,								 // Test swamp depth.
			true,								 // Test edge front.
			true,								 // Test ledge heights.
			true								 // Test ledge steep floor.
		};

		// Crouch action held and extended crawl moveset enabled; return nullopt.
		if (IsHeld(In::Crouch) && g_GameFlow->HasCrawlExtended())
			return std::nullopt;

		// Get tread water vault climb context.
		auto attracColl = GetEdgeVaultClimbAttractorCollision(item, coll, SETUP, attracColls);
		if (attracColl.has_value())
		{
			auto context = ClimbContextData{};
			context.Attractor = attracColl->Attractor;
			context.PathDistance = attracColl->PathDistance;
			context.RelPosOffset = Vector3(0.0f, 0.0f, -coll.Setup.Radius);
			context.RelOrientOffset = EulerAngles::Identity;
			context.TargetStateID = LS_TREAD_WATER_VAULT_0_STEPS_TO_STAND;
			context.AlignType = ClimbContextAlignType::AttractorParent;
			context.IsJump = false;

			return context;
		}

		return std::nullopt;
	}

	static std::optional<ClimbContextData> GetTreadWaterVault1StepUpToStandClimbContext(const ItemInfo& item, const CollisionInfo& coll,
																						const std::vector<AttractorCollisionData>& attracColls)
	{
		constexpr auto SETUP = EdgeVaultClimbSetupData
		{
			-(int)CLICK(0.5f), -STEPUP_HEIGHT, // Edge height bounds.
			-CLICK(1),						   // Edge-to-ceil height lower bound.
			LARA_HEIGHT, -MAX_HEIGHT,		   // Destination floor-to-ceil range.
			false,							   // Find highest.
			false,							   // Test swamp depth.
			true,							   // Test edge front.
			true,							   // Test ledge heights.
			true							   // Test ledge steep floor.
		};
		constexpr auto VERTICAL_OFFSET = CLICK(1);

		// Crouch action held and extended crawl moveset enabled; return nullopt.
		if (IsHeld(In::Crouch) && g_GameFlow->HasCrawlExtended())
			return std::nullopt;

		// Get tread water vault climb context.
		auto attracColl = GetEdgeVaultClimbAttractorCollision(item, coll, SETUP, attracColls);
		if (attracColl.has_value())
		{
			auto context = ClimbContextData{};
			context.Attractor = attracColl->Attractor;
			context.PathDistance = attracColl->PathDistance;
			context.RelPosOffset = Vector3(0.0f, VERTICAL_OFFSET, -coll.Setup.Radius);
			context.RelOrientOffset = EulerAngles::Identity;
			context.TargetStateID = LS_TREAD_WATER_VAULT_1_STEP_UP_TO_STAND;
			context.AlignType = ClimbContextAlignType::AttractorParent;
			context.IsJump = false;

			return context;
		}

		return std::nullopt;
	}

	static std::optional<ClimbContextData> GetTreadWaterVault1StepDownToCrouchClimbContext(const ItemInfo& item, const CollisionInfo& coll,
																						   const std::vector<AttractorCollisionData>& attracColls)
	{
		constexpr auto SETUP = EdgeVaultClimbSetupData
		{
			STEPUP_HEIGHT, (int)CLICK(0.5f), // Edge height bounds.
			-(int)CLICK(0.6f),				 // Edge-to-ceil height lower bound.
			LARA_HEIGHT_CRAWL, -MAX_HEIGHT,	 // Destination floor-to-ceil range.
			false,							 // Find highest.
			false,							 // Test swamp depth.
			true,							 // Test edge front.
			true,							 // Test ledge heights.
			true							 // Test ledge steep floor.
		};
		constexpr auto VERTICAL_OFFSET = -CLICK(1);

		// Extended crawl moveset disabled; return nullopt.
		if (!g_GameFlow->HasCrawlExtended())
			return std::nullopt;

		// Get tread water vault climb context.
		auto attracColl = GetEdgeVaultClimbAttractorCollision(item, coll, SETUP, attracColls);
		if (attracColl.has_value())
		{
			auto context = ClimbContextData{};
			context.Attractor = attracColl->Attractor;
			context.PathDistance = attracColl->PathDistance;
			context.RelPosOffset = Vector3(0.0f, VERTICAL_OFFSET, -coll.Setup.Radius);
			context.RelOrientOffset = EulerAngles::Identity;
			context.TargetStateID = LS_TREAD_WATER_VAULT_1_STEP_DOWN_TO_CROUCH;
			context.AlignType = ClimbContextAlignType::AttractorParent;
			context.IsJump = false;

			return context;
		}

		return std::nullopt;
	}

	static std::optional<ClimbContextData> GetTreadWaterVault0StepsToCrouchClimbContext(const ItemInfo& item, const CollisionInfo& coll,
																						const std::vector<AttractorCollisionData>& attracColls)
	{
		constexpr auto SETUP = EdgeVaultClimbSetupData
		{
			(int)CLICK(0.5f), -(int)CLICK(0.5f), // Edge height bounds.
			-(int)CLICK(0.6f),					 // Edge-to-ceil height lower bound.
			LARA_HEIGHT_CRAWL, -MAX_HEIGHT,		 // Destination floor-to-ceil range.
			false,								 // Find highest.
			false,								 // Test swamp depth.
			true,								 // Test edge front.
			true,								 // Test ledge heights.
			true								 // Test ledge steep floor.
		};

		// Extended crawl moveset disabled; return nullopt.
		if (!g_GameFlow->HasCrawlExtended())
			return std::nullopt;

		// Get tread water vault climb context.
		auto attracColl = GetEdgeVaultClimbAttractorCollision(item, coll, SETUP, attracColls);
		if (attracColl.has_value())
		{
			auto context = ClimbContextData{};
			context.Attractor = attracColl->Attractor;
			context.PathDistance = attracColl->PathDistance;
			context.RelPosOffset = Vector3(0.0f, 0.0f, -coll.Setup.Radius);
			context.RelOrientOffset = EulerAngles::Identity;
			context.TargetStateID = LS_TREAD_WATER_VAULT_0_STEPS_TO_CROUCH;
			context.AlignType = ClimbContextAlignType::AttractorParent;
			context.IsJump = false;

			return context;
		}

		return std::nullopt;
	}

	static std::optional<ClimbContextData> GetTreadWaterVault1StepUpToCrouchClimbContext(const ItemInfo& item, const CollisionInfo& coll,
																						 const std::vector<AttractorCollisionData>& attracColls)
	{
		constexpr auto SETUP = EdgeVaultClimbSetupData
		{
			-CLICK(1), -STEPUP_HEIGHT,		// Edge height bounds.
			-(int)CLICK(0.6f),				// Edge-to-ceil height lower bound.
			LARA_HEIGHT_CRAWL, -MAX_HEIGHT, // Destination floor-to-ceil range.
			false,							// Find highest.
			false,							// Test swamp depth.
			true,							// Test edge front.
			true,							// Test ledge heights.
			true							// Test ledge steep floor.
		};
		constexpr auto VERTICAL_OFFSET = CLICK(1);

		// Extended crawl moveset disabled; return nullopt.
		if (!g_GameFlow->HasCrawlExtended())
			return std::nullopt;

		// Get tread water vault climb context.
		auto attracColl = GetEdgeVaultClimbAttractorCollision(item, coll, SETUP, attracColls);
		if (attracColl.has_value())
		{
			auto context = ClimbContextData{};
			context.Attractor = attracColl->Attractor;
			context.PathDistance = attracColl->PathDistance;
			context.RelPosOffset = Vector3(0.0f, VERTICAL_OFFSET, -coll.Setup.Radius);
			context.RelOrientOffset = EulerAngles::Identity;
			context.TargetStateID = LS_TREAD_WATER_VAULT_1_STEP_UP_TO_CROUCH;
			context.AlignType = ClimbContextAlignType::AttractorParent;
			context.IsJump = false;

			return context;
		}

		return std::nullopt;
	}

	const std::optional<ClimbContextData> GetTreadWaterClimbableWallMountClimbContext(ItemInfo& item, const CollisionInfo& coll,
																					  const std::vector<AttractorCollisionData>& attracColls)
	{
		// TODO
		constexpr auto SETUP = WallEdgeMountClimbSetupData
		{

		};

		// Get climbable wall mount climb context.
		auto attracColl = GetClimbableWallMountAttractorCollision(item, coll, SETUP, attracColls);
		if (attracColl.has_value())
		{
			auto context = ClimbContextData{};
			context.Attractor = nullptr;
			context.PathDistance = 0.0f;
			context.RelPosOffset = Vector3(0.0f, 0.0f, -coll.Setup.Radius);
			context.RelOrientOffset = EulerAngles::Identity;
			context.TargetStateID = LS_WALL_CLIMB_IDLE;
			context.AlignType = ClimbContextAlignType::AttractorParent;
			context.IsJump = false;

			return context;
		}

		return std::nullopt;
	}

	std::optional<ClimbContextData> GetTreadWaterVaultClimbContext(ItemInfo& item, const CollisionInfo& coll)
	{
		constexpr auto ATTRAC_DETECT_RADIUS = BLOCK(0.5f);

		const auto& player = GetLaraInfo(item);

		// Check hand status.
		if (player.Control.HandStatus != HandStatus::Free)
			return std::nullopt;

		// Get attractor collisions.
		auto attracColls = GetAttractorCollisions(item.Pose.Position.ToVector3(), ATTRAC_DETECT_RADIUS, item.RoomNumber, item.Pose.Orientation.y);

		auto context = std::optional<ClimbContextData>();

		// 1) Vault 1 step down to stand.
		context = GetTreadWaterVault1StepDownToStandClimbContext(item, coll, attracColls);
		if (context.has_value())
		{
			if (HasStateDispatch(&item, context->TargetStateID))
				return context;
		}

		// 2) Vault 1 step down to crouch.
		context = GetTreadWaterVault1StepDownToCrouchClimbContext(item, coll, attracColls);
		if (context.has_value())
		{
			if (HasStateDispatch(&item, context->TargetStateID))
				return context;
		}

		// 3) Vault 0 steps to stand.
		context = GetTreadWaterVault0StepsToStandClimbContext(item, coll, attracColls);
		if (context.has_value())
		{
			if (HasStateDispatch(&item, context->TargetStateID))
				return context;
		}

		// 4) Vault 0 steps to crouch.
		context = GetTreadWaterVault0StepsToCrouchClimbContext(item, coll, attracColls);
		if (context.has_value())
		{
			if (HasStateDispatch(&item, context->TargetStateID))
				return context;
		}

		// 5) Vault up 1 step up to stand.
		context = GetTreadWaterVault1StepUpToStandClimbContext(item, coll, attracColls);
		if (context.has_value())
		{
			if (HasStateDispatch(&item, context->TargetStateID))
				return context;
		}

		// 6) Vault up 1 step up to crouch.
		context = GetTreadWaterVault1StepUpToCrouchClimbContext(item, coll, attracColls);
		if (context.has_value())
		{
			if (HasStateDispatch(&item, context->TargetStateID))
				return context;
		}

		// 7) Mount climbable wall.
		context = GetTreadWaterClimbableWallMountClimbContext(item, coll, attracColls);
		if (context.has_value())
		{
			if (HasStateDispatch(&item, context->TargetStateID))
				return context;
		}

		// No valid edge attractor collision; return nullopt.
		return std::nullopt;
	}

	std::optional<WaterTreadStepOutContextData> GetTreadWaterStepOutContext(const ItemInfo& item)
	{
		auto& player = GetLaraInfo(item);

		// Get point collision.
		auto pointColl = GetPointCollision(item);
		int vPos = item.Pose.Position.y;

		// Assess water height.
		if ((pointColl.GetFloorHeight() - item.Pose.Position.y) >= SWIM_WATER_DEPTH)
			return std::nullopt;

		if ((pointColl.GetFloorHeight() - vPos) <= 0)
			return std::nullopt;

		if ((pointColl.GetFloorHeight() - vPos) >= CLICK(1))
		{
			auto context = WaterTreadStepOutContextData{};
			context.FloorHeight = pointColl.GetFloorHeight();
			context.AnimNumber = LA_STAND_IDLE;

			return context;
		}
		else
		{
			auto context = WaterTreadStepOutContextData{};
			context.FloorHeight = pointColl.GetFloorHeight();
			context.AnimNumber = LA_ONWATER_TO_WADE_1_STEP;

			return context;
		}

		return std::nullopt;
	}
}
