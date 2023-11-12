#include "framework.h"
#include "Game/Lara/lara.h"

#include "Game/animation.h"
#include "Game/camera.h"
#include "Game/collision/collide_item.h"
#include "Game/collision/floordata.h"
#include "Game/control/flipeffect.h"
#include "Game/control/volume.h"
#include "Game/effects/Hair.h"
#include "Game/effects/item_fx.h"
#include "Game/effects/tomb4fx.h"
#include "Game/Gui.h"
#include "Game/items.h"
#include "Game/Lara/lara_cheat.h"
#include "Game/Lara/lara_collide.h"
#include "Game/Lara/lara_fire.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/Lara/lara_initialise.h"
#include "Game/Lara/PlayerStateMachine.h"
#include "Game/misc.h"
#include "Game/savegame.h"
#include "Renderer/Renderer11.h"
#include "Scripting/Include/Flow/ScriptInterfaceFlowHandler.h"
#include "Scripting/Include/ScriptInterfaceLevel.h"
#include "Sound/sound.h"
#include "Specific/Input/Input.h"
#include "Specific/winmain.h"

using namespace TEN::Collision::Floordata;
using namespace TEN::Control::Volumes;
using namespace TEN::Effects::Hair;
using namespace TEN::Effects::Items;
using namespace TEN::Entities::Player;
using namespace TEN::Input;
using namespace TEN::Math;

using TEN::Renderer::g_Renderer;

LaraInfo Lara = {};
ItemInfo* LaraItem;
CollisionInfo LaraCollision = {};

//debug------
#include <OISKeyboard.h>
using namespace OIS;
//------

static void SectorDebug(const ItemInfo& item)
{
	using Vector3 = DirectX::SimpleMath::Vector3;

	constexpr auto ROT_STEP	   = ANGLE(1.5f);
	constexpr auto HEIGHT_STEP = CLICK(0.05f);

	auto pointColl = GetCollision(item);

	// Store sector.
	static FloorInfo* sectorPtr = nullptr;
	static int roomNumber = 0;
	static auto pos2D = Vector2i::Zero;
	if (KeyMap[KC_T] || sectorPtr == nullptr)
	{
		sectorPtr = pointColl.BottomBlock;
		roomNumber = item.RoomNumber;
		pos2D = Vector2i(item.Pose.Position.x, item.Pose.Position.z);
	}

	//g_Renderer.PrintDebugMessage("Wall portal: %d", sectorPtr->WallPortalRoomNumber);
	//g_Renderer.PrintDebugMessage("");

	// Draw floor surface debug.
	g_Renderer.PrintDebugMessage("FLOOR SURFACE");
	g_Renderer.PrintDebugMessage("Normal0: %.3f, %.3f, %.3f", sectorPtr->FloorSurface.Triangles[0].Plane.x, sectorPtr->FloorSurface.Triangles[0].Plane.y, sectorPtr->FloorSurface.Triangles[0].Plane.z);
	g_Renderer.PrintDebugMessage("Dist0: %.3f", sectorPtr->FloorSurface.Triangles[0].Plane.D());
	g_Renderer.PrintDebugMessage("Normal1: %.3f, %.3f, %.3f", sectorPtr->FloorSurface.Triangles[1].Plane.x, sectorPtr->FloorSurface.Triangles[1].Plane.y, sectorPtr->FloorSurface.Triangles[1].Plane.z);
	g_Renderer.PrintDebugMessage("Dist1: %.3f", sectorPtr->FloorSurface.Triangles[1].Plane.D());
	g_Renderer.PrintDebugMessage("Illegal slope angle: %.3f", TO_DEGREES(sectorPtr->GetSurfaceIllegalSlopeAngle(pos2D.x, pos2D.y, true)));
	g_Renderer.PrintDebugMessage("");

	g_Renderer.PrintDebugMessage("CEILING SURFACE");
	g_Renderer.PrintDebugMessage("Normal0: %.3f, %.3f, %.3f", sectorPtr->CeilingSurface.Triangles[0].Plane.x, sectorPtr->CeilingSurface.Triangles[0].Plane.y, sectorPtr->CeilingSurface.Triangles[0].Plane.z);
	g_Renderer.PrintDebugMessage("Dist0: %.3f", sectorPtr->CeilingSurface.Triangles[0].Plane.D());
	g_Renderer.PrintDebugMessage("Normal1: %.3f, %.3f, %.3f", sectorPtr->CeilingSurface.Triangles[1].Plane.x, sectorPtr->CeilingSurface.Triangles[1].Plane.y, sectorPtr->CeilingSurface.Triangles[1].Plane.z);
	g_Renderer.PrintDebugMessage("Dist1: %.3f", sectorPtr->CeilingSurface.Triangles[1].Plane.D());
	g_Renderer.PrintDebugMessage("");

	/*auto tilt = GetSurfaceTilt(pointColl.FloorNormal, true);
	g_Renderer.PrintDebugMessage("TILT BACKPORT");
	g_Renderer.PrintDebugMessage("tilt x: %d", tilt.x);
	g_Renderer.PrintDebugMessage("tilt z: %d", tilt.y);
	g_Renderer.PrintDebugMessage("");*/

	// Set triangle ID.
	static int triangleID = 2;
	static bool dbPlaneID = true;
	if (KeyMap[KC_H] && dbPlaneID)
	{
		triangleID++;
		if (triangleID >= 3)
			triangleID = 0;
	}
	dbPlaneID = !KeyMap[KC_H];
	g_Renderer.PrintDebugMessage("Triangle ID: %d", triangleID);

	// Update slippery slope angle.
	if (KeyMap[KC_Q])
	{
		if (triangleID == 2)
		{
			sectorPtr->FloorSurface.Triangles[0].IllegalSlopeAngle += ROT_STEP;
			sectorPtr->FloorSurface.Triangles[1].IllegalSlopeAngle += ROT_STEP;
		}
		else
		{
			sectorPtr->FloorSurface.Triangles[triangleID].IllegalSlopeAngle += ROT_STEP;
		}
	}
	else if (KeyMap[KC_W])
	{
		if (triangleID == 2)
		{
			sectorPtr->FloorSurface.Triangles[0].IllegalSlopeAngle -= ROT_STEP;
			sectorPtr->FloorSurface.Triangles[1].IllegalSlopeAngle -= ROT_STEP;
		}
		else
		{
			sectorPtr->FloorSurface.Triangles[triangleID].IllegalSlopeAngle -= ROT_STEP;
		}
	}

	// Update height.
	if (KeyMap[KC_Y])
	{
		if (triangleID == 2)
		{
			sectorPtr->FloorSurface.Triangles[0].Plane.w -= HEIGHT_STEP;
			sectorPtr->FloorSurface.Triangles[1].Plane.w -= HEIGHT_STEP;
		}
		else
		{
			sectorPtr->FloorSurface.Triangles[triangleID].Plane.w -= HEIGHT_STEP;
		}
	}
	else if (KeyMap[KC_U])
	{
		if (triangleID == 2)
		{
			sectorPtr->FloorSurface.Triangles[0].Plane.w += HEIGHT_STEP;
			sectorPtr->FloorSurface.Triangles[1].Plane.w += HEIGHT_STEP;
		}
		else
		{
			sectorPtr->FloorSurface.Triangles[triangleID].Plane.w += HEIGHT_STEP;
		}
	}

	// Set rotation.
	auto rot = EulerAngles::Zero;
	if (KeyMap[KC_I])
	{
		rot.z += ROT_STEP;
	}
	else if (KeyMap[KC_K])
	{
		rot.z -= ROT_STEP;
	}
	if (KeyMap[KC_J])
	{
		rot.x -= ROT_STEP;
	}
	else if (KeyMap[KC_L])
	{
		rot.x += ROT_STEP;
	}

	// Transform normals.
	auto rotMatrix = rot.ToRotationMatrix();
	if (triangleID == 2)
	{
		auto normal0 = Vector3::Transform(sectorPtr->FloorSurface.Triangles[0].Plane.Normal(), rotMatrix);
		normal0.Normalize();
		sectorPtr->FloorSurface.Triangles[0].Plane = Plane(normal0, sectorPtr->FloorSurface.Triangles[0].Plane.D());

		auto normal1 = Vector3::Transform(sectorPtr->FloorSurface.Triangles[1].Plane.Normal(), rotMatrix);
		normal1.Normalize();
		sectorPtr->FloorSurface.Triangles[1].Plane = Plane(normal1, sectorPtr->FloorSurface.Triangles[1].Plane.D());
	}
	else
	{
		auto normal = Vector3::Transform(sectorPtr->FloorSurface.Triangles[triangleID].Plane.Normal(), rotMatrix);
		normal.Normalize();
		sectorPtr->FloorSurface.Triangles[triangleID].Plane = Plane(normal, sectorPtr->FloorSurface.Triangles[triangleID].Plane.D());
	}

	// Get line points.
	const auto& room = g_Level.Rooms[sectorPtr->RoomNumber];
	auto roomGridCoord = GetRoomGridCoord(roomNumber, pos2D.x, pos2D.y);
	auto points0 = std::array<Vector3, 3>{};
	auto points1 = std::array<Vector3, 3>{};
	for (int i = 0; i < points0.size(); i++)
	{
		int x = BLOCK(roomGridCoord.x);
		int z = BLOCK(roomGridCoord.y);

		if (sectorPtr->FloorSurface.SplitAngle == SectorSurfaceData::SPLIT_ANGLE_0)
		{
			if (i == 0)
			{
				x += 1;
				z += 1;
			}
			else if (i == 1)
			{
				x += BLOCK(1) - 1;
				z += BLOCK(1) - 1;
			}
			else if (i == 2)
			{
				x += 1;
				z += BLOCK(1) - 1;
			}
		}
		else
		{
			if (i == 0)
			{
				x += BLOCK(1) - 1;
				z += 1;
			}
			else if (i == 1)
			{
				x += BLOCK(1) - 1;
				z += BLOCK(1) - 1;
			}
			else if (i == 2)
			{
				x += 1;
				z += BLOCK(1) - 1;
			}
		}

		points0[i] = Vector3(x + room.x, sectorPtr->GetSurfaceHeight(0, x, z, true, true), z + room.z);
	}
	for (int i = 0; i < points1.size(); i++)
	{
		int x = BLOCK(roomGridCoord.x);
		int z = BLOCK(roomGridCoord.y);

		if (sectorPtr->FloorSurface.SplitAngle == SectorSurfaceData::SPLIT_ANGLE_0)
		{
			if (i == 0)
			{
				x += 1;
				z += 1;
			}
			else if (i == 1)
			{
				x += BLOCK(1) - 1;
				z += 1;
			}
			else if (i == 2)
			{
				x += BLOCK(1) - 1;
				z += BLOCK(1) - 1;
			}
		}
		else
		{
			if (i == 0)
			{
				x += 1;
				z += 1;
			}
			else if (i == 1)
			{
				x += BLOCK(1) - 1;
				z += 1;
			}
			else if (i == 2)
			{
				x += 1;
				z += BLOCK(1) - 1;
			}
		}

		points1[i] = Vector3(x + room.x, sectorPtr->GetSurfaceHeight(1, x, z, true, true), z + room.z);
	}

	// Draw outlines.
	g_Renderer.AddLine3D(points0[0], points0[1], Vector4::One);
	g_Renderer.AddLine3D(points0[1], points0[2], Vector4::One);
	g_Renderer.AddLine3D(points0[2], points0[0], Vector4::One);

	g_Renderer.AddLine3D(points1[0], points1[1], Vector4::One);
	g_Renderer.AddLine3D(points1[1], points1[2], Vector4::One);
	g_Renderer.AddLine3D(points1[2], points1[0], Vector4::One);

	// Draw lines to floor.
	for (const auto& point : points0)
		g_Renderer.AddLine3D(point, point + Vector3(0.0f, BLOCK(8), 0.0f), Vector4::One);
	for (const auto& point : points1)
		g_Renderer.AddLine3D(point, point + Vector3(0.0f, BLOCK(8), 0.0f), Vector4::One);
}

void LaraControl(ItemInfo* item, CollisionInfo* coll)
{
	auto& player = GetLaraInfo(*item);

	SectorDebug(*item);

	// Alert nearby creatures.
	if (player.Control.Weapon.HasFired)
	{
		AlertNearbyGuards(item);
		player.Control.Weapon.HasFired = false;
	}

	// Handle object interation adjustment parameters.
	if (player.Control.IsMoving)
	{
		if (player.Control.Count.PositionAdjust > LARA_POSITION_ADJUST_MAX_TIME)
		{
			player.Control.IsMoving = false;
			player.Control.HandStatus = HandStatus::Free;
		}

		++player.Control.Count.PositionAdjust;
	}
	else
	{
		player.Control.Count.PositionAdjust = 0;
	}

	if (!player.Control.Locked)
		player.LocationPad = -1;

	// FAILSAFE: Force hand status reset.
	if (item->Animation.AnimNumber == LA_STAND_IDLE &&
		item->Animation.ActiveState == LS_IDLE &&
		item->Animation.TargetState == LS_IDLE &&
		!item->Animation.IsAirborne &&
		player.Control.HandStatus == HandStatus::Busy)
	{
		player.Control.HandStatus = HandStatus::Free;
	}

	HandlePlayerQuickActions(*item);
	RumbleLaraHealthCondition(item);

	auto water = GetPlayerWaterData(*item);
	player.Context.WaterSurfaceDist = -water.HeightFromWater;

	if (player.Context.Vehicle == NO_ITEM)
		SpawnPlayerSplash(*item, water.WaterHeight, water.WaterDepth);

	bool isWaterOnHeadspace = false;

	// TODO: Move unrelated handling elsewhere.
	// Handle environment state transition.
	if (player.Context.Vehicle == NO_ITEM && player.ExtraAnim == NO_ITEM)
	{
		switch (player.Control.WaterStatus)
		{
		case WaterStatus::Dry:
			for (int i = 0; i < NUM_LARA_MESHES; i++)
				player.Effect.BubbleNodes[i] = 0.0f;

			if (water.HeightFromWater == NO_HEIGHT || water.HeightFromWater < WADE_WATER_DEPTH)
				break;

			Camera.targetElevation = ANGLE(-22.0f);

			// Water is at swim depth; dispatch dive.
			if (water.WaterDepth >= SWIM_WATER_DEPTH && !water.IsSwamp)
			{
				if (water.IsWater)
				{
					item->Pose.Position.y += CLICK(0.5f) - 28; // TODO: Demagic.
					item->Animation.IsAirborne = false;
					player.Control.WaterStatus = WaterStatus::Underwater;
					player.Status.Air = LARA_AIR_MAX;

					for (int i = 0; i < NUM_LARA_MESHES; i++)
						player.Effect.BubbleNodes[i] = PLAYER_BUBBLE_NODE_MAX;

					UpdateLaraRoom(item, 0);
					StopSoundEffect(SFX_TR4_LARA_FALL);

					if (item->Animation.ActiveState == LS_SWAN_DIVE)
					{
						SetAnimation(item, LA_SWANDIVE_DIVE);
						item->Animation.Velocity.y /= 2;
						item->Pose.Orientation.x = ANGLE(-45.0f);
						player.Control.HandStatus = HandStatus::Free;
					}
					else if (item->Animation.ActiveState == LS_FREEFALL_DIVE)
					{
						SetAnimation(item, LA_SWANDIVE_DIVE);
						item->Animation.Velocity.y /= 2;
						item->Pose.Orientation.x = ANGLE(-85.0f);
						player.Control.HandStatus = HandStatus::Free;
					}
					else
					{
						SetAnimation(item, LA_FREEFALL_DIVE);
						item->Animation.Velocity.y = item->Animation.Velocity.y * (3 / 8.0f);
						item->Pose.Orientation.x = ANGLE(-45.0f);
					}

					ResetPlayerFlex(item);
					Splash(item);
				}
			}
			// Water is at wade depth; update water status and do special handling.
			else if (water.HeightFromWater >= WADE_WATER_DEPTH)
			{
				player.Control.WaterStatus = WaterStatus::Wade;

				// Make splash ONLY within this particular threshold before swim depth while airborne (SpawnPlayerSplash() above interferes otherwise).
				if (water.WaterDepth > (SWIM_WATER_DEPTH - CLICK(1)) &&
					item->Animation.IsAirborne && !water.IsSwamp)
				{
					item->Animation.TargetState = LS_IDLE;
					Splash(item);
				}
				// Player is grounded; don't splash again.
				else if (!item->Animation.IsAirborne)
				{
					item->Animation.TargetState = LS_IDLE;
				}
				else if (water.IsSwamp)
				{
					if (item->Animation.ActiveState == LS_SWAN_DIVE ||
						item->Animation.ActiveState == LS_FREEFALL_DIVE)
					{
						item->Pose.Position.y = water.WaterHeight + (BLOCK(1) - 24); // TODO: Demagic.
					}

					SetAnimation(item, LA_WADE);
				}
			}

			break;

		case WaterStatus::Underwater:
			// Disable potential player resurfacing if health is <= 0.
			// Originals worked without this condition, but TEN does not. -- Lwmte, 11.08.22
			if (item->HitPoints <= 0)
				break;

			// Determine if player's head is above water surface. Needed to prevent
			// pre-TR5 bug where player would keep submerged until root mesh was above water level.
			isWaterOnHeadspace = TestEnvironment(
				ENV_FLAG_WATER, item->Pose.Position.x, item->Pose.Position.y - CLICK(1), item->Pose.Position.z,
				GetCollision(item->Pose.Position.x, item->Pose.Position.y - CLICK(1), item->Pose.Position.z, item->RoomNumber).RoomNumber);

			if (water.WaterDepth == NO_HEIGHT || abs(water.HeightFromWater) >= CLICK(1) || isWaterOnHeadspace ||
				item->Animation.AnimNumber == LA_UNDERWATER_RESURFACE || item->Animation.AnimNumber == LA_ONWATER_DIVE)
			{
				if (!water.IsWater)
				{
					if (water.WaterDepth == NO_HEIGHT || abs(water.HeightFromWater) >= CLICK(1))
					{
						SetAnimation(item, LA_FALL_START);
						ResetPlayerLean(item);
						ResetPlayerFlex(item);
						item->Animation.IsAirborne = true;
						item->Animation.Velocity.z = item->Animation.Velocity.y;
						item->Animation.Velocity.y = 0.0f;
						player.Control.WaterStatus = WaterStatus::Dry;
					}
					else
					{
						SetAnimation(item, LA_UNDERWATER_RESURFACE);
						ResetPlayerLean(item);
						ResetPlayerFlex(item);
						item->Animation.Velocity.y = 0.0f;
						item->Pose.Position.y = water.WaterHeight;
						player.Control.WaterStatus = WaterStatus::TreadWater;

						UpdateLaraRoom(item, -(STEPUP_HEIGHT - 3));
					}
				}
			}
			else
			{
				SetAnimation(item, LA_UNDERWATER_RESURFACE);
				ResetPlayerLean(item);
				ResetPlayerFlex(item);
				item->Animation.Velocity.y = 0.0f;
				item->Pose.Position.y = water.WaterHeight + 1;
				player.Control.WaterStatus = WaterStatus::TreadWater;

				UpdateLaraRoom(item, 0);
			}

			break;

		case WaterStatus::TreadWater:
			if (!water.IsWater)
			{
				if (water.HeightFromWater <= WADE_WATER_DEPTH)
				{
					SetAnimation(item, LA_FALL_START);
					item->Animation.IsAirborne = true;
					item->Animation.Velocity.z = item->Animation.Velocity.y;
					player.Control.WaterStatus = WaterStatus::Dry;
				}
				else
				{
					SetAnimation(item, LA_STAND_IDLE);
					player.Control.WaterStatus = WaterStatus::Wade;
				}

				ResetPlayerLean(item);
				ResetPlayerFlex(item);
				item->Animation.Velocity.y = 0.0f;
			}

			break;

		case WaterStatus::Wade:
			Camera.targetElevation = ANGLE(-22.0f);

			if (water.HeightFromWater >= WADE_WATER_DEPTH)
			{
				if (water.HeightFromWater > SWIM_WATER_DEPTH && !water.IsSwamp)
				{
					SetAnimation(item, LA_ONWATER_IDLE);
					ResetPlayerLean(item);
					ResetPlayerFlex(item);
					item->Animation.IsAirborne = false;
					item->Animation.Velocity.y = 0.0f;
					item->Pose.Position.y += 1 - water.HeightFromWater;
					player.Control.WaterStatus = WaterStatus::TreadWater;

					UpdateLaraRoom(item, 0);
				}
			}
			else
			{
				player.Control.WaterStatus = WaterStatus::Dry;

				if (item->Animation.ActiveState == LS_WADE_FORWARD)
					item->Animation.TargetState = LS_RUN_FORWARD;
			}

			break;
		}
	}

	HandlePlayerStatusEffects(*item, player.Control.WaterStatus, water);

	auto prevPos = item->Pose.Position;

	// Handle environment state.
	switch (player.Control.WaterStatus)
	{
	case WaterStatus::Dry:
	case WaterStatus::Wade:
		LaraAboveWater(item, coll);
		break;

	case WaterStatus::Underwater:
		LaraUnderwater(item, coll);
		break;

	case WaterStatus::TreadWater:
		LaraWaterSurface(item, coll);
		break;

	case WaterStatus::FlyCheat:
		LaraCheat(item, coll);
		break;
	}

	using Vector3 = DirectX::SimpleMath::Vector3;
	Statistics.Game.Distance += (int)round(Vector3::Distance(prevPos.ToVector3(), item->Pose.Position.ToVector3()));

	if (DebugMode)
	{
		DrawNearbyPathfinding(GetCollision(item).BottomBlock->Box);
		DrawNearbySectorFlags(*item);
	}
}

void LaraAboveWater(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	// Reset collision setup.
	coll->Setup.Mode = CollisionProbeMode::Quadrants;
	coll->Setup.Radius = LARA_RADIUS;
	coll->Setup.Height = LARA_HEIGHT;
	coll->Setup.UpperCeilingBound = NO_UPPER_BOUND;
	coll->Setup.BlockFloorSlopeUp = false;
	coll->Setup.BlockFloorSlopeDown = false;
	coll->Setup.BlockCeilingSlope = false;
	coll->Setup.BlockDeathFloorDown = false;
	coll->Setup.BlockMonkeySwingEdge = false;
	coll->Setup.EnableObjectPush = true;
	coll->Setup.EnableSpasm = true;
	coll->Setup.PrevPosition = item->Pose.Position;
	coll->Setup.PrevAnimObjectID = item->Animation.AnimObjectID;
	coll->Setup.PrevAnimNumber = item->Animation.AnimNumber;
	coll->Setup.PrevFrameNumber = item->Animation.FrameNumber;
	coll->Setup.PrevState = item->Animation.ActiveState;

	UpdateLaraRoom(item, -LARA_HEIGHT / 2);

	// Handle look-around.
	if (((IsHeld(In::Look) && lara->Control.Look.Mode != LookMode::None) ||
			(lara->Control.Look.IsUsingBinoculars || lara->Control.Look.IsUsingLasersight)) &&
		lara->ExtraAnim == NO_ITEM)
	{
		HandlePlayerLookAround(*item);
	}
	else
	{
		// TODO: Extend ResetLaraFlex() to be a catch-all function.
		ResetPlayerLookAround(*item);
	}
	lara->Control.Look.Mode = LookMode::None;

	// Process vehicles.
	if (HandleLaraVehicle(item, coll))
		return;

	// Handle player behavior state control.
	HandlePlayerBehaviorState(*item, *coll, PlayerBehaviorStateRoutineType::Control);

	HandleLaraMovementParameters(item, coll);
	AnimateItem(item);

	if (lara->ExtraAnim == NO_ITEM)
	{
		// Check for collision with items.
		DoObjectCollision(item, coll);

		// Handle player behavior state collision.
		if (lara->Context.Vehicle == NO_ITEM)
			HandlePlayerBehaviorState(*item, *coll, PlayerBehaviorStateRoutineType::Collision);
	}

	// Handle weapons.
	HandleWeapon(*item);

	// Handle breath.
	LaraBreath(item);

	// Test for flags and triggers.
	ProcessSectorFlags(item);
	TestTriggers(item, false);
	TestVolumes(item->Index, &coll->Setup);
}

void LaraWaterSurface(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	lara->Control.IsLow = false;

	Camera.targetElevation = -ANGLE(22.0f);

	// Reset collision setup.
	coll->Setup.Mode = CollisionProbeMode::FreeForward;
	coll->Setup.Radius = LARA_RADIUS;
	coll->Setup.Height = LARA_HEIGHT_SURFACE;
	coll->Setup.LowerFloorBound = NO_LOWER_BOUND;
	coll->Setup.UpperFloorBound = -CLICK(0.5f);
	coll->Setup.LowerCeilingBound = LARA_RADIUS;
	coll->Setup.UpperCeilingBound = NO_UPPER_BOUND;
	coll->Setup.BlockFloorSlopeUp = false;
	coll->Setup.BlockFloorSlopeDown = false;
	coll->Setup.BlockCeilingSlope = false;
	coll->Setup.BlockDeathFloorDown = false;
	coll->Setup.BlockMonkeySwingEdge = false;
	coll->Setup.EnableObjectPush = true;
	coll->Setup.EnableSpasm = false;
	coll->Setup.PrevPosition = item->Pose.Position;

	if (IsHeld(In::Look) && lara->Control.Look.Mode != LookMode::None)
	{
		HandlePlayerLookAround(*item);
	}
	else
	{
		ResetPlayerLookAround(*item);
	}

	lara->Control.Count.Pose = 0;

	HandlePlayerBehaviorState(*item, *coll, PlayerBehaviorStateRoutineType::Control);

	auto* level = g_GameFlow->GetLevel(CurrentLevel);

	// TODO: Subsuit gradually slows down at rate of 0.5 degrees. @Sezz 2022.06.23
	// Apply and reset turn rate.
	item->Pose.Orientation.y += lara->Control.TurnRate;
	if (!(IsHeld(In::Left) || IsHeld(In::Right)))
		lara->Control.TurnRate = 0;

	if (level->GetLaraType() == LaraType::Divesuit)
		UpdateLaraSubsuitAngles(item);

	// Reset lean.
	if (!lara->Control.IsMoving && !(IsHeld(In::Left) || IsHeld(In::Right)))
		ResetPlayerLean(item, 1 / 8.0f);

	if (lara->Context.WaterCurrentActive && lara->Control.WaterStatus != WaterStatus::FlyCheat)
		LaraWaterCurrent(item, coll);

	AnimateItem(item);
	TranslateItem(item, lara->Control.MoveAngle, item->Animation.Velocity.y);

	DoObjectCollision(item, coll);

	if (lara->Context.Vehicle == NO_ITEM)
		HandlePlayerBehaviorState(*item, *coll, PlayerBehaviorStateRoutineType::Collision);

	UpdateLaraRoom(item, LARA_RADIUS);

	HandleWeapon(*item);

	ProcessSectorFlags(item);
	TestTriggers(item, false);
	TestVolumes(item->Index);
}

void LaraUnderwater(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	lara->Control.IsLow = false;

	// Reset collision setup.
	coll->Setup.Mode = CollisionProbeMode::Quadrants;
	coll->Setup.Radius = LARA_RADIUS_UNDERWATER;
	coll->Setup.Height = LARA_HEIGHT;
	coll->Setup.LowerFloorBound = NO_LOWER_BOUND;
	coll->Setup.UpperFloorBound = -(LARA_RADIUS_UNDERWATER + (LARA_RADIUS_UNDERWATER / 3));
	coll->Setup.LowerCeilingBound = LARA_RADIUS_UNDERWATER + (LARA_RADIUS_UNDERWATER / 3);
	coll->Setup.UpperCeilingBound = NO_UPPER_BOUND;
	coll->Setup.BlockFloorSlopeUp = false;
	coll->Setup.BlockFloorSlopeDown = false;
	coll->Setup.BlockCeilingSlope = false;
	coll->Setup.BlockDeathFloorDown = false;
	coll->Setup.BlockMonkeySwingEdge = false;
	coll->Setup.EnableObjectPush = true;
	coll->Setup.EnableSpasm = false;
	coll->Setup.PrevPosition = item->Pose.Position;

	if (IsHeld(In::Look) && lara->Control.Look.Mode != LookMode::None)
	{
		HandlePlayerLookAround(*item);
	}
	else
	{
		ResetPlayerLookAround(*item);
	}

	lara->Control.Count.Pose = 0;

	HandlePlayerBehaviorState(*item, *coll, PlayerBehaviorStateRoutineType::Control);

	auto* level = g_GameFlow->GetLevel(CurrentLevel);

	// TODO: Subsuit gradually slowed down at rate of 0.5 degrees. @Sezz 2022.06.23
	// Apply and reset turn rate.
	item->Pose.Orientation.y += lara->Control.TurnRate;
	if (!(IsHeld(In::Left) || IsHeld(In::Right)))
		lara->Control.TurnRate = 0;

	if (level->GetLaraType() == LaraType::Divesuit)
		UpdateLaraSubsuitAngles(item);

	if (!lara->Control.IsMoving && !(IsHeld(In::Left) || IsHeld(In::Right)))
		ResetPlayerLean(item, 1 / 8.0f, true, false);

	if (item->Pose.Orientation.x < -ANGLE(85.0f))
		item->Pose.Orientation.x = -ANGLE(85.0f);
	else if (item->Pose.Orientation.x > ANGLE(85.0f))
		item->Pose.Orientation.x = ANGLE(85.0f);

	if (level->GetLaraType() == LaraType::Divesuit)
	{
		if (item->Pose.Orientation.z > ANGLE(44.0f))
			item->Pose.Orientation.z = ANGLE(44.0f);
		else if (item->Pose.Orientation.z < -ANGLE(44.0f))
			item->Pose.Orientation.z = -ANGLE(44.0f);
	}
	else
	{
		if (item->Pose.Orientation.z > ANGLE(22.0f))
			item->Pose.Orientation.z = ANGLE(22.0f);
		else if (item->Pose.Orientation.z < -ANGLE(22.0f))
			item->Pose.Orientation.z = -ANGLE(22.0f);
	}

	if (lara->Context.WaterCurrentActive && lara->Control.WaterStatus != WaterStatus::FlyCheat)
		LaraWaterCurrent(item, coll);

	AnimateItem(item);
	TranslateItem(item, item->Pose.Orientation, item->Animation.Velocity.y);

	DoObjectCollision(item, coll);

	if (/*lara->ExtraAnim == -1 &&*/ lara->Context.Vehicle == NO_ITEM)
		HandlePlayerBehaviorState(*item, *coll, PlayerBehaviorStateRoutineType::Collision);

	UpdateLaraRoom(item, 0);

	HandleWeapon(*item);

	ProcessSectorFlags(item);
	TestTriggers(item, false);
	TestVolumes(item->Index);
}

void LaraCheat(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	item->HitPoints = LARA_HEALTH_MAX;
	lara->Status.Air = LARA_AIR_MAX;
	lara->Status.Exposure = LARA_EXPOSURE_MAX;
	lara->Status.Poison = 0;
	lara->Status.Stamina = LARA_STAMINA_MAX;
	
	LaraUnderwater(item, coll);

	if (IsHeld(In::Walk) && !IsHeld(In::Look))
	{
		if (TestEnvironment(ENV_FLAG_WATER, item) || (lara->Context.WaterSurfaceDist > 0 && lara->Context.WaterSurfaceDist != NO_HEIGHT))
		{
			SetAnimation(item, LA_UNDERWATER_IDLE);
			ResetPlayerFlex(item);
			lara->Control.WaterStatus = WaterStatus::Underwater;
		}
		else
		{
			SetAnimation(item, LA_STAND_SOLID);
			item->Pose.Orientation.x = 0;
			item->Pose.Orientation.z = 0;
			ResetPlayerFlex(item);
			lara->Control.WaterStatus = WaterStatus::Dry;
		}

		InitializeLaraMeshes(item);
		item->HitPoints = LARA_HEALTH_MAX;
		lara->Control.HandStatus = HandStatus::Free;
	}
}

void UpdateLara(ItemInfo* item, bool isTitle)
{
	if (isTitle && !g_GameFlow->IsLaraInTitleEnabled())
		return;

	// HACK: backup controls until proper control lock 
	// is implemented -- Lwmte, 07.12.22

	auto actionMap = ActionMap;

	if (isTitle)
		ClearAllActions();

	// Control Lara.
	InItemControlLoop = true;
	LaraControl(item, &LaraCollision);
	LaraCheatyBits(item);
	InItemControlLoop = false;
	KillMoveItems();

	if (isTitle)
		ActionMap = actionMap;

	if (g_Gui.GetInventoryItemChosen() != NO_ITEM)
	{
		g_Gui.SetInventoryItemChosen(NO_ITEM);
		SayNo();
	}

	// Update player animations.
	g_Renderer.UpdateLaraAnimations(true);

	// Update player effects.
	HairEffect.Update(*item, g_GameFlow->GetLevel(CurrentLevel)->GetLaraType() == LaraType::Young);
	HandlePlayerWetnessDrips(*item);
	HandlePlayerDiveBubbles(*item);
	ProcessEffects(item);
}

// Offset values may be used to account for the quirk of room traversal only being able to occur at portals.
bool UpdateLaraRoom(ItemInfo* item, int height, int xOffset, int zOffset)
{
	auto point = Geometry::TranslatePoint(item->Pose.Position, item->Pose.Orientation.y, zOffset, height, xOffset);

	// Hacky L-shaped Location traversal.
	item->Location = GetRoom(item->Location, point);
	item->Location = GetRoom(item->Location, Vector3i(item->Pose.Position.x, point.y, item->Pose.Position.z));
	item->Floor = GetFloorHeight(item->Location, item->Pose.Position.x, item->Pose.Position.z).value_or(NO_HEIGHT);

	if (item->RoomNumber != item->Location.roomNumber)
	{
		ItemNewRoom(item->Index, item->Location.roomNumber);
		return true;
	}

	return false;
}
