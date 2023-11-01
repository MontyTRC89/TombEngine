#include "framework.h"
#include "Game/Lara/lara.h"

#include "Game/Lara/lara_basic.h"
#include "Game/Lara/lara_cheat.h"
#include "Game/Lara/lara_climb.h"
#include "Game/Lara/lara_collide.h"
#include "Game/Lara/lara_crawl.h"
#include "Game/Lara/lara_fire.h"
#include "Game/Lara/lara_hang.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/Lara/lara_initialise.h"
#include "Game/Lara/lara_jump.h"
#include "Game/Lara/lara_monkey.h"
#include "Game/Lara/lara_objects.h"
#include "Game/Lara/lara_one_gun.h"
#include "Game/Lara/lara_overhang.h"
#include "Game/Lara/lara_slide.h"
#include "Game/Lara/lara_surface.h"
#include "Game/Lara/lara_swim.h"
#include "Game/Lara/lara_tests.h"

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
using namespace TEN::Input;
using namespace TEN::Math;

using TEN::Renderer::g_Renderer;

using PlayerStateRoutine = std::function<void(ItemInfo* item, CollisionInfo* coll)>;

LaraInfo Lara = {};
ItemInfo* LaraItem;
CollisionInfo LaraCollision = {};

auto PlayerStateControlRoutines = std::array<PlayerStateRoutine, NUM_LARA_STATES + 1>
{
	lara_as_walk_forward,
	lara_as_run_forward,
	lara_as_idle,
	lara_as_jump_forward,//33
	lara_as_pose,//4
	lara_as_run_back,//5
	lara_as_turn_right_slow,//6
	lara_as_turn_left_slow,//7
	lara_as_death,//8
	lara_as_freefall,//9
	lara_as_hang,
	lara_as_reach,
	lara_as_splat,
	lara_as_underwater_idle,//13
	lara_void_func,//14
	lara_as_jump_prepare,//15
	lara_as_walk_back,//16
	lara_as_underwater_swim_forward,//17
	lara_as_underwater_inertia,//18
	lara_as_controlled_no_look,//19
	lara_as_turn_right_fast,//20
	lara_as_step_right,//21
	lara_as_step_left,//22
	lara_as_roll_180_back,//23
	lara_as_slide_forward,//24
	lara_as_jump_back,//25
	lara_as_jump_right,//26
	lara_as_jump_left,//27
	lara_as_jump_up,//28
	lara_as_fall_back,//29
	lara_as_shimmy_left,//30
	lara_as_shimmy_right,//31
	lara_as_slide_back,//32
	lara_as_surface_idle,//33
	lara_as_surface_swim_forward,//34
	lara_as_surface_dive,//35
	lara_as_pushable_push,//36
	lara_as_pushable_pull,//37
	lara_as_pushable_grab,//38
	lara_as_pickup,//39
	lara_as_switch_on,//40
	lara_as_switch_off,//41
	lara_as_use_key,//42
	lara_as_use_puzzle,//43
	lara_as_underwater_death,//44
	lara_as_roll_180_forward,//45
	lara_as_special,//46
	lara_as_surface_swim_back,//47
	lara_as_surface_swim_left,//48
	lara_as_surface_swim_right,//49
	lara_void_func,//50
	lara_void_func,//51
	lara_as_swan_dive,//52
	lara_as_freefall_dive,//53
	lara_as_handstand,//54
	lara_as_surface_climb_out,//55
	lara_as_climb_idle,//56
	lara_as_climb_up,//57
	lara_as_climb_left,//58
	lara_as_climb_end,//59
	lara_as_climb_right,//60
	lara_as_climb_down,//61
	lara_as_auto_jump,//62
	lara_void_func,//63
	lara_void_func,//64
	lara_as_wade_forward,//65
	lara_as_underwater_roll_180,//66
	lara_as_pickup_flare,//67
	lara_void_func,//68
	lara_void_func,//69
	lara_as_zip_line,//70
	lara_as_crouch_idle,//71
	lara_as_crouch_roll,//72
	lara_as_sprint,//73
	lara_as_sprint_dive,//74
	lara_as_monkey_idle,//75
	lara_as_monkey_forward,//76
	lara_as_monkey_shimmy_left,//77
	lara_as_monkey_shimmy_right,//78
	lara_as_monkey_turn_180,//79
	lara_as_crawl_idle,//80
	lara_as_crawl_forward,//81
	lara_as_monkey_turn_left,//82
	lara_as_monkey_turn_right,//83
	lara_as_crawl_turn_left,//84
	lara_as_crawl_turn_right,//85
	lara_as_crawl_back,//86
	lara_as_controlled_no_look,
	lara_as_controlled_no_look,
	lara_as_controlled,
	lara_as_rope_turn_clockwise,
	lara_as_rope_turn_counter_clockwise,
	lara_as_controlled,
	lara_as_controlled,
	lara_as_controlled,
	lara_as_controlled_no_look,
	lara_as_controlled_no_look,
	lara_as_controlled,
	lara_as_pickup,//98
	lara_as_pole_idle,//99
	lara_as_pole_up,//100
	lara_as_pole_down,//101
	lara_as_pole_turn_clockwise,//102
	lara_as_pole_turn_counter_clockwise,//103
	lara_as_pulley,//104
	lara_as_crouch_turn_left,//105
	lara_as_crouch_turn_right,//106
	lara_as_shimmy_corner,//107
	lara_as_shimmy_corner,//108
	lara_as_shimmy_corner,//109
	lara_as_shimmy_corner,//110
	lara_as_rope_idle,//111
	lara_as_rope_up,//112
	lara_as_rope_down,//113
	lara_as_rope_idle,//114
	lara_as_rope_idle,//115
	lara_void_func,
	lara_as_controlled,
	lara_as_swimcheat,
	lara_as_tightrope_idle,//119
	lara_as_controlled_no_look,//120
	lara_as_tightrope_walk,//121
	lara_as_tightrope_fall,//122
	lara_as_tightrope_fall,//123
	lara_as_null,//124
	lara_as_tightrope_dismount,//125
	lara_as_switch_on,//126
	lara_as_null,//127
	lara_as_horizontal_bar_swing,//128
	lara_as_horizontal_bar_leap,//129
	lara_as_null,//130
	lara_as_controlled_no_look,//131
	lara_as_controlled_no_look,//132
	lara_as_null,//133
	lara_as_null,//134
	lara_as_null,//135
	lara_as_null,//136
	lara_as_null,//137
	lara_as_null,//138
	lara_as_null,//139
	lara_as_null,//140
	lara_as_null,//141
	lara_as_null,//142
	lara_as_slopeclimb,//143
	lara_as_slopeclimbup,//144
	lara_as_slopeclimbdown,//145
	lara_as_controlled_no_look,//146
	lara_as_null,//147
	lara_as_null,//148
	lara_as_slopefall,//149
	lara_as_climb_stepoff_left,
	lara_as_climb_stepoff_right,
	lara_as_turn_left_fast,
	lara_as_controlled,
	lara_as_controlled,
	lara_as_controlled,//155
	lara_as_slopehang,
	lara_as_slopeshimmy,
	lara_as_sclimbstart,
	lara_as_sclimbstop,
	lara_as_sclimbend,
	lara_as_null,//161
	lara_as_null,//162
	lara_as_monkey_back,//163
	lara_as_vault,//164
	lara_as_vault,//165
	lara_as_vault,//166
	lara_as_vault,//167
	lara_as_vault,//168
	lara_as_vault,//169
	lara_as_idle,//170
	lara_as_crouch_turn_180,//171
	lara_as_crawl_turn_180,//172
	lara_as_turn_180,//173
	lara_as_null,
	lara_as_null,
	lara_as_null,
	lara_as_null,
	lara_as_null,
	lara_as_null,
	lara_as_null,
	lara_as_null,
	lara_as_null,
	lara_as_null,
	lara_as_null,
	lara_as_null,
	lara_as_null,
	lara_as_null,
	lara_as_null,
	lara_as_use_puzzle,//189
	lara_as_pushable_edge_slip,//190
	lara_as_sprint_slide,//191
};

auto PlayerStateCollisionRoutines = std::array<PlayerStateRoutine, NUM_LARA_STATES + 1>
{
	lara_col_walk_forward,
	lara_col_run_forward,
	lara_col_idle,
	lara_col_jump_forward,//3
	lara_col_idle,//4
	lara_col_run_back,
	lara_col_turn_right_slow,
	lara_col_turn_left_slow,
	lara_col_death,
	lara_col_freefall,//9
	lara_col_hang,
	lara_col_reach,
	lara_col_splat,
	lara_col_underwater_idle,
	lara_col_land,
	lara_col_jump_prepare,//15
	lara_col_walk_back,
	lara_col_underwater_swim_forward,
	lara_col_underwater_inertia,
	lara_default_col,//19
	lara_col_turn_right_fast,
	lara_col_step_right,
	lara_col_step_left,
	lara_col_roll_180_back,
	lara_col_slide_forward,//24
	lara_col_jump_back,//25
	lara_col_jump_right,//26
	lara_col_jump_left,//27
	lara_col_jump_up,//28
	lara_col_fall_back,//29
	lara_col_shimmy_left,
	lara_col_shimmy_right,
	lara_col_slide_back,//32
	lara_col_surface_idle,//33
	lara_col_surface_swim_forward,//34
	lara_col_surface_dive,//35
	lara_void_func,//36
	lara_void_func,//37
	lara_default_col,
	lara_default_col,
	lara_default_col,
	lara_default_col,
	lara_default_col,
	lara_default_col,
	lara_col_underwater_death,//44
	lara_col_roll_180_forward,//45
	lara_void_func,//46
	lara_col_surface_swim_back,//47
	lara_col_surface_swim_left,//48
	lara_col_surface_swim_right,//49
	lara_void_func,
	lara_void_func,
	lara_col_swan_dive,//52
	lara_col_freefall_dive,//53
	lara_default_col,
	lara_default_col,//55
	lara_col_climb_idle,
	lara_col_climb_up,
	lara_col_climb_left,
	lara_col_climb_end,
	lara_col_climb_right,
	lara_col_climb_down,
	lara_col_jump_prepare,//62
	lara_void_func,
	lara_void_func,
	lara_col_wade_forward,
	lara_col_underwater_roll_180,
	lara_default_col,
	lara_void_func,
	lara_void_func,
	lara_void_func,
	lara_col_crouch_idle,
	lara_col_crouch_roll,
	lara_col_sprint,
	lara_col_sprint_dive,
	lara_col_monkey_idle,
	lara_col_monkey_forward,//76
	lara_col_monkey_shimmy_left,//77
	lara_col_monkey_shimmy_right,//78
	lara_col_monkey_turn_180,//79
	lara_col_crawl_idle,
	lara_col_crawl_forward,//81
	lara_col_monkey_turn_left,//82
	lara_col_monkey_turn_right,
	lara_col_crawl_turn_left,
	lara_col_crawl_turn_right,
	lara_col_crawl_back,
	lara_void_func,
	lara_col_crawl_to_hang,
	lara_default_col,
	lara_void_func,
	lara_void_func,
	lara_default_col,
	lara_void_func,
	lara_void_func,
	lara_col_turn_switch,
	lara_void_func,
	lara_void_func,
	lara_default_col,
	lara_col_pole_idle,
	lara_col_pole_up,
	lara_col_pole_down,
	lara_col_pole_turn_clockwise,
	lara_col_pole_turn_counter_clockwise,
	lara_default_col,
	lara_col_crouch_turn_left,
	lara_col_crouch_turn_right,
	lara_as_null,
	lara_as_null,
	lara_as_null,
	lara_as_null,
	lara_col_rope_idle,
	lara_void_func,
	lara_void_func,
	lara_col_rope_swing,
	lara_col_rope_swing,
	lara_void_func,
	lara_void_func,
	lara_col_underwater_swim_forward,
	lara_default_col,
	lara_default_col,
	lara_default_col,
	lara_default_col,
	lara_default_col,
	lara_default_col,
	lara_default_col,
	lara_default_col,
	lara_default_col,
	lara_default_col,
	lara_default_col,
	lara_void_func,
	lara_void_func,
	lara_void_func,
	lara_void_func,
	lara_void_func,
	lara_void_func,
	lara_void_func,
	lara_void_func,
	lara_void_func,
	lara_void_func,
	lara_void_func,
	lara_void_func,
	lara_void_func,
	lara_col_slopeclimb,
	lara_default_col,     // lara_col_slopeclimbup
	lara_default_col,     // lara_col_slopeclimbdown
	lara_void_func,
	lara_void_func,
	lara_void_func,
	lara_default_col,     // lara_col_slopefall
	lara_default_col,
	lara_default_col,
	lara_col_turn_left_fast,
	lara_default_col,
	lara_default_col,
	lara_default_col,
	lara_col_slopehang,	  // lara_col_slopehang
	lara_col_slopeshimmy, // lara_col_slopeshimmy
	lara_default_col,	  // lara_col_sclimbstart
	lara_default_col,     // lara_col_sclimbstop
	lara_default_col,	  // lara_col_sclimbend
	lara_void_func,//161
	lara_void_func,//162
	lara_col_monkey_back,//163
	lara_void_func,//164
	lara_void_func,//165
	lara_void_func,//166
	lara_void_func,//167
	lara_void_func,//168
	lara_void_func,//169
	lara_col_idle,//170
	lara_col_crouch_turn_180,//171
	lara_col_crawl_turn_180,//172
	lara_col_turn_180,//173
	lara_void_func,
	lara_void_func,
	lara_void_func,
	lara_void_func,
	lara_void_func,
	lara_void_func,
	lara_void_func,
	lara_void_func,
	lara_void_func,
	lara_void_func,
	lara_void_func,
	lara_void_func,
	lara_void_func,
	lara_void_func,
	lara_void_func,
	lara_default_col,//189
	lara_void_func,//190
	lara_col_sprint_slide,//191
};

void LaraControl(ItemInfo* item, CollisionInfo* coll)
{
	auto& player = GetLaraInfo(*item);

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
						item->Animation.Velocity.y = (item->Animation.Velocity.y / 8) * 3;
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

	// Handle player state.
	PlayerStateControlRoutines[item->Animation.ActiveState](item, coll);

	HandleLaraMovementParameters(item, coll);
	AnimateItem(item);

	if (lara->ExtraAnim == NO_ITEM)
	{
		// Check for collision with items.
		DoObjectCollision(item, coll);

		// Handle player state collision.
		if (lara->Context.Vehicle == NO_ITEM)
			PlayerStateCollisionRoutines[item->Animation.ActiveState](item, coll);
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

	PlayerStateControlRoutines[item->Animation.ActiveState](item, coll);

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
		PlayerStateCollisionRoutines[item->Animation.ActiveState](item, coll);

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

	PlayerStateControlRoutines[item->Animation.ActiveState](item, coll);

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
		PlayerStateCollisionRoutines[item->Animation.ActiveState](item, coll);

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
