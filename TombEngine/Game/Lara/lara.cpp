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
#include "Game/effects/hair.h"
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

using namespace TEN::Control::Volumes;
using namespace TEN::Effects::Items;
using namespace TEN::Floordata;
using namespace TEN::Input;
using namespace TEN::Math;

using TEN::Renderer::g_Renderer;

LaraInfo Lara = {};
ItemInfo* LaraItem;
CollisionInfo LaraCollision = {};

std::function<LaraRoutineFunction> lara_control_routines[NUM_LARA_STATES + 1] =
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
	lara_void_func,//124
#ifdef NEW_TIGHTROPE
	lara_as_tightrope_dismount,//125
#else // !NEW_TIGHTROPE
	lara_void_func,//125
#endif
	lara_as_switch_on,//126
	lara_void_func,//127
	lara_as_horizontal_bar_swing,//128
	lara_as_horizontal_bar_leap,//129
	lara_void_func,//130
	lara_as_controlled_no_look,//131
	lara_as_controlled_no_look,//132
	lara_void_func,//133
	lara_void_func,//134
	lara_void_func,//135
	lara_void_func,//136
	lara_void_func,//137
	lara_void_func,//138
	lara_void_func,//139
	lara_void_func,//140
	lara_void_func,//141
	lara_void_func,//142
	lara_as_slopeclimb,//143
	lara_as_slopeclimbup,//144
	lara_as_slopeclimbdown,//145
	lara_as_controlled_no_look,//146
	lara_void_func,//147
	lara_void_func,//148
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
	lara_void_func,//161
	lara_void_func,//162
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
	lara_as_ladder_idle,//174
	lara_as_ladder_up,//175
	lara_as_ladder_down,//176
	lara_void_func,//177
	lara_void_func,//178
	lara_void_func,//179
	lara_void_func,//180
	lara_void_func,//181
	lara_void_func,//182
	lara_void_func,//183
	lara_void_func,//184
	lara_void_func,//185
	lara_void_func,//186
	lara_void_func,//187
	lara_void_func,//188
};

std::function<LaraRoutineFunction> lara_collision_routines[NUM_LARA_STATES + 1] =
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
	lara_default_col,
	lara_default_col,
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
	lara_void_func,
	lara_void_func,
	lara_void_func,
	lara_void_func,
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
	lara_col_ladder_idle,//174
	lara_col_ladder_up,//175
	lara_col_ladder_down,//176
	lara_void_func,//177
	lara_void_func,//178
	lara_void_func,//179
	lara_void_func,//180
	lara_void_func,//181
	lara_void_func,//182
	lara_void_func,//183
	lara_void_func,//184
	lara_void_func,//185
	lara_void_func,//186
	lara_void_func,//187
	lara_void_func,//188
};

void LaraControl(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	if (lara->Control.Weapon.HasFired)
	{
		AlertNearbyGuards(item);
		lara->Control.Weapon.HasFired = false;
	}

	if (lara->PoisonPotency)
	{
		if (lara->PoisonPotency > LARA_POISON_POTENCY_MAX)
			lara->PoisonPotency = LARA_POISON_POTENCY_MAX;

		if (!(Wibble & 0xFF))
			item->HitPoints -= lara->PoisonPotency;
	}

	if (lara->Control.IsMoving)
	{
		if (lara->Control.Count.PositionAdjust > LARA_POSITION_ADJUST_MAX_TIME)
		{
			lara->Control.IsMoving = false;
			lara->Control.HandStatus = HandStatus::Free;
		}

		++lara->Control.Count.PositionAdjust;
	}
	else
	{
		lara->Control.Count.PositionAdjust = 0;
	}

	if (!lara->Control.Locked)
		lara->LocationPad = -1;

	auto prevPos = item->Pose.Position;

	if (lara->Control.HandStatus == HandStatus::Busy &&
		item->Animation.AnimNumber == LA_STAND_IDLE &&
		item->Animation.ActiveState == LS_IDLE &&
		item->Animation.TargetState == LS_IDLE &&
		!item->Animation.IsAirborne)
	{
		lara->Control.HandStatus = HandStatus::Free;
	}

	if (lara->SprintEnergy < LARA_SPRINT_ENERGY_MAX && item->Animation.ActiveState != LS_SPRINT)
		lara->SprintEnergy++;

	RumbleLaraHealthCondition(item);

	bool isWater = TestEnvironment(ENV_FLAG_WATER, item);
	bool isSwamp = TestEnvironment(ENV_FLAG_SWAMP, item);

	bool isWaterOnHeadspace = false;

	int waterDepth = GetWaterDepth(item);
	int waterHeight = GetWaterHeight(item);

	int heightFromWater;
	if (waterHeight != NO_HEIGHT)
		heightFromWater = item->Pose.Position.y - waterHeight;
	else
		heightFromWater = NO_HEIGHT;
	lara->WaterSurfaceDist = -heightFromWater;

	if (lara->Vehicle == NO_ITEM)
		WadeSplash(item, waterHeight, waterDepth);

	if (lara->Vehicle == NO_ITEM && lara->ExtraAnim == NO_ITEM)
	{
		switch (lara->Control.WaterStatus)
		{
		case WaterStatus::Dry:
			for (int i = 0; i < NUM_LARA_MESHES; i++)
				lara->Effect.BubbleNodes[i] = 0.0f;

			if (heightFromWater == NO_HEIGHT || heightFromWater < WADE_DEPTH)
				break;

			Camera.targetElevation = ANGLE(-22.0f);

			// Water is deep enough to swim; dispatch dive.
			if (waterDepth >= SWIM_DEPTH && !isSwamp)
			{
				if (isWater)
				{
					item->Pose.Position.y += CLICK(0.5f) - 28;
					item->Animation.IsAirborne = false;
					lara->Control.WaterStatus = WaterStatus::Underwater;
					lara->Air = LARA_AIR_MAX;

					for (int i = 0; i < NUM_LARA_MESHES; i++)
						lara->Effect.BubbleNodes[i] = PLAYER_BUBBLE_NODE_MAX;

					UpdateLaraRoom(item, 0);
					StopSoundEffect(SFX_TR4_LARA_FALL);

					if (item->Animation.ActiveState == LS_SWAN_DIVE)
					{
						SetAnimation(item, LA_SWANDIVE_DIVE);
						item->Animation.Velocity.y /= 2.0f;
						item->Pose.Orientation.x = -ANGLE(45.0f);
						lara->Control.HandStatus = HandStatus::Free;
					}
					else if (item->Animation.ActiveState == LS_FREEFALL_DIVE)
					{
						SetAnimation(item, LA_SWANDIVE_DIVE);
						item->Animation.Velocity.y /= 2.0f;
						item->Pose.Orientation.x = -ANGLE(85.0f);
						lara->Control.HandStatus = HandStatus::Free;
					}
					else
					{
						SetAnimation(item, LA_FREEFALL_DIVE);
						item->Animation.Velocity.y = (item->Animation.Velocity.y / 8.0f) * 3.0f;
						item->Pose.Orientation.x = -ANGLE(45.0f);
					}

					ResetLaraFlex(item);
					Splash(item);
				}
			}
			// Water is at wade depth; update water status and do special handling.
			else if (heightFromWater >= WADE_DEPTH)
			{
				lara->Control.WaterStatus = WaterStatus::Wade;

				// Make splash ONLY within this particular threshold before swim depth while airborne (WadeSplash() above interferes otherwise).
				if (waterDepth > (SWIM_DEPTH - CLICK(1)) &&
					item->Animation.IsAirborne && !isSwamp)
				{
					item->Animation.TargetState = LS_IDLE;
					Splash(item);
				}
				// Player is grounded; don't splash again.
				else if (!item->Animation.IsAirborne)
				{
					item->Animation.TargetState = LS_IDLE;
				}
				else if (isSwamp)
				{
					if (item->Animation.ActiveState == LS_SWAN_DIVE ||
						item->Animation.ActiveState == LS_FREEFALL_DIVE)
					{
						item->Pose.Position.y = waterHeight + (SECTOR(1) - 24);
					}

					SetAnimation(item, LA_WADE);
				}
			}

			break;

		case WaterStatus::Underwater:

			// Disable potential Lara resurfacing if her health is zero or below.
			// For some reason, originals worked without this condition, but TEN does not. -- Lwmte, 11.08.22

			if (item->HitPoints <= 0)
				break;

			// Determine if Lara's head is above water surface. This is needed to prevent
			// pre-TR5 bug where Lara would keep submerged until her root mesh (butt) is above water level.

			isWaterOnHeadspace = TestEnvironment(ENV_FLAG_WATER, item->Pose.Position.x, item->Pose.Position.y - CLICK(1), item->Pose.Position.z,
					 GetCollision(item->Pose.Position.x, item->Pose.Position.y - CLICK(1), item->Pose.Position.z, item->RoomNumber).RoomNumber);

			if (waterDepth == NO_HEIGHT || abs(heightFromWater) >= CLICK(1) || isWaterOnHeadspace ||
				item->Animation.AnimNumber == LA_UNDERWATER_RESURFACE || item->Animation.AnimNumber == LA_ONWATER_DIVE)
			{
				if (!isWater)
				{
					if (waterDepth == NO_HEIGHT || abs(heightFromWater) >= CLICK(1))
					{
						SetAnimation(item, LA_FALL_START);
						ResetLaraLean(item);
						ResetLaraFlex(item);
						item->Animation.IsAirborne = true;
						item->Animation.Velocity.z = item->Animation.Velocity.y;
						item->Animation.Velocity.y = 0.0f;
						lara->Control.WaterStatus = WaterStatus::Dry;
					}
					else
					{
						SetAnimation(item, LA_UNDERWATER_RESURFACE);
						ResetLaraLean(item);
						ResetLaraFlex(item);
						item->Animation.Velocity.y = 0.0f;
						item->Pose.Position.y = waterHeight;
						lara->Control.WaterStatus = WaterStatus::TreadWater;

						UpdateLaraRoom(item, -(STEPUP_HEIGHT - 3));
					}
				}
			}
			else
			{
				SetAnimation(item, LA_UNDERWATER_RESURFACE);
				ResetLaraLean(item);
				ResetLaraFlex(item);
				item->Animation.Velocity.y = 0.0f;
				item->Pose.Position.y = waterHeight + 1;
				lara->Control.WaterStatus = WaterStatus::TreadWater;

				UpdateLaraRoom(item, 0);
			}

			break;

		case WaterStatus::TreadWater:
			if (!isWater)
			{
				if (heightFromWater <= WADE_DEPTH)
				{
					SetAnimation(item, LA_FALL_START);
					item->Animation.IsAirborne = true;
					item->Animation.Velocity.z = item->Animation.Velocity.y;
					lara->Control.WaterStatus = WaterStatus::Dry;
				}
				else
				{
					SetAnimation(item, LA_STAND_IDLE);
					lara->Control.WaterStatus = WaterStatus::Wade;
				}

				ResetLaraLean(item);
				ResetLaraFlex(item);
				item->Animation.Velocity.y = 0.0f;
			}

			break;

		case WaterStatus::Wade:
			Camera.targetElevation = -ANGLE(22.0f);

			if (heightFromWater >= WADE_DEPTH)
			{
				if (heightFromWater > SWIM_DEPTH && !isSwamp)
				{
					SetAnimation(item, LA_ONWATER_IDLE);
					ResetLaraLean(item);
					ResetLaraFlex(item);
					item->Animation.IsAirborne = false;
					item->Animation.Velocity.y = 0.0f;
					item->Pose.Position.y += 1 - heightFromWater;
					lara->Control.WaterStatus = WaterStatus::TreadWater;

					UpdateLaraRoom(item, 0);
				}
			}
			else
			{
				lara->Control.WaterStatus = WaterStatus::Dry;

				if (item->Animation.ActiveState == LS_WADE_FORWARD)
					item->Animation.TargetState = LS_RUN_FORWARD;
			}

			break;
		}
	}

	if (TestEnvironment(ENV_FLAG_DAMAGE, item) && item->HitPoints > 0)
	{
		item->HitPoints--;
	}

	if (item->HitPoints <= 0)
	{
		item->HitPoints = -1;

		if (lara->Control.Count.Death == 0)
			StopSoundTracks();

		lara->Control.Count.Death++;
		if ((item->Flags & IFLAG_INVISIBLE))
		{
			lara->Control.Count.Death++;
			return;
		}
	}

	switch (lara->Control.WaterStatus)
	{
	case WaterStatus::Dry:
	case WaterStatus::Wade:
		if (isSwamp	&& lara->WaterSurfaceDist < -(LARA_HEIGHT + 8)) // TODO: Find best height. @Sezz 2021.11.10
		{
			if (item->HitPoints >= 0)
			{
				lara->Air -= 6;
				if (lara->Air < 0)
				{
					lara->Air = -1;
					item->HitPoints -= 10;
				}
			}
		}
		else if (lara->Air < LARA_AIR_MAX && item->HitPoints >= 0)
		{
			if (lara->Vehicle == NO_ITEM) // Only for UPV.
			{
				lara->Air += 10;
				if (lara->Air > LARA_AIR_MAX)
					lara->Air = LARA_AIR_MAX;
			}
		}

		LaraAboveWater(item, coll);
		break;

	case WaterStatus::Underwater:
		if (item->HitPoints >= 0)
		{
			auto level = g_GameFlow->GetLevel(CurrentLevel);
			if (level->GetLaraType() != LaraType::Divesuit)
				lara->Air--;

			if (lara->Air < 0)
			{
				item->HitPoints -= 5;
				lara->Air = -1;
			}
		}

		LaraUnderwater(item, coll);
		break;

	case WaterStatus::TreadWater:
		if (item->HitPoints >= 0)
		{
			lara->Air += 10;
			if (lara->Air > LARA_AIR_MAX)
				lara->Air = LARA_AIR_MAX;
		}

		LaraWaterSurface(item, coll);
		break;

	case WaterStatus::FlyCheat:
		LaraCheat(item, coll);
		break;
	}

	Statistics.Game.Distance += (int)round(Vector3::Distance(prevPos.ToVector3(), item->Pose.Position.ToVector3()));
}

void LaraAboveWater(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	coll->Setup.Mode = CollisionProbeMode::Quadrants;
	// TODO: Move radius and height resets here when look feature is refactored. @Sezz 2022.03.29

	coll->Setup.UpperCeilingBound = NO_UPPER_BOUND;

	coll->Setup.BlockFloorSlopeUp = false;
	coll->Setup.BlockFloorSlopeDown = false;
	coll->Setup.BlockCeilingSlope = false;
	coll->Setup.BlockDeathFloorDown = false;
	coll->Setup.BlockMonkeySwingEdge = false;
	coll->Setup.EnableObjectPush = true;
	coll->Setup.EnableSpasm = true;

	coll->Setup.OldPosition = item->Pose.Position;
	coll->Setup.OldAnimNumber = item->Animation.AnimNumber;
	coll->Setup.OldFrameNumber = item->Animation.FrameNumber;
	coll->Setup.OldState = item->Animation.ActiveState;

	if (TrInput & IN_LOOK && lara->Control.CanLook &&
		lara->ExtraAnim == NO_ITEM)
	{
		if (BinocularOn)
			LookUpDown(item);

		LookLeftRight(item);
	}
	else if (coll->Setup.Height > LARA_HEIGHT - LARA_HEADROOM) // TEMP HACK: Look feature will need a dedicated refactor; ResetLook() interferes with crawl flexing. @Sezz 2021.12.10
		ResetLook(item);

	coll->Setup.Radius = LARA_RADIUS;
	coll->Setup.Height = LARA_HEIGHT;
	lara->Control.CanLook = true;

	UpdateLaraRoom(item, -LARA_HEIGHT / 2);

	// Process vehicles.
	if (HandleLaraVehicle(item, coll))
		return;

	// Handle current Lara status.
	lara_control_routines[item->Animation.ActiveState](item, coll);
	HandleLaraMovementParameters(item, coll);
	AnimateLara(item);

	if (lara->ExtraAnim == NO_ITEM)
	{
		// Check for collision with items.
		DoObjectCollision(item, coll);

		// Handle Lara collision.
		if (lara->Vehicle == NO_ITEM)
			lara_collision_routines[item->Animation.ActiveState](item, coll);
	}

	// Handle weapons.
	HandleWeapon(item);

	// Handle breath.
	LaraBreath(item);

	// Test for flags and triggers.
	ProcessSectorFlags(item);
	TestTriggers(item, false);
	TestVolumes(Lara.ItemNumber, &coll->Setup);

	DrawNearbyPathfinding(GetCollision(item).BottomBlock->Box);
}

void LaraWaterSurface(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	lara->Control.IsLow = false;

	Camera.targetElevation = -ANGLE(22.0f);

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
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpasm = false;

	coll->Setup.OldPosition = item->Pose.Position;

	if (TrInput & IN_LOOK && lara->Control.CanLook)
		LookLeftRight(item);
	else
		ResetLook(item);

	lara->Control.CanLook = true;
	lara->Control.Count.Pose = 0;

	lara_control_routines[item->Animation.ActiveState](item, coll);

	auto level = g_GameFlow->GetLevel(CurrentLevel);

	// TODO: Subsuit gradually slows down at rate of 0.5 degrees. @Sezz 2022.06.23
	// Apply and reset turn rate.
	item->Pose.Orientation.y += lara->Control.TurnRate;
	if (!(TrInput & (IN_LEFT | IN_RIGHT)))
		lara->Control.TurnRate = 0;

	if (level->GetLaraType() == LaraType::Divesuit)
		UpdateLaraSubsuitAngles(item);

	// Reset lean.
	if (!lara->Control.IsMoving && !(TrInput & (IN_LEFT | IN_RIGHT)))
		ResetLaraLean(item, 8.0f);

	if (lara->WaterCurrentActive && lara->Control.WaterStatus != WaterStatus::FlyCheat)
		LaraWaterCurrent(item, coll);

	AnimateLara(item);
	TranslateItem(item, lara->Control.MoveAngle, item->Animation.Velocity.y);

	DoObjectCollision(item, coll);

	if (lara->Vehicle == NO_ITEM)
		lara_collision_routines[item->Animation.ActiveState](item, coll);

	UpdateLaraRoom(item, LARA_RADIUS);

	HandleWeapon(item);

	ProcessSectorFlags(item);
	TestTriggers(item, false);
	TestVolumes(Lara.ItemNumber);
}

void LaraUnderwater(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	lara->Control.IsLow = false;

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

	coll->Setup.OldPosition = item->Pose.Position;

	if (TrInput & IN_LOOK && lara->Control.CanLook)
		LookLeftRight(item);
	else
		ResetLook(item);

	lara->Control.CanLook = true;
	lara->Control.Count.Pose = 0;

	lara_control_routines[item->Animation.ActiveState](item, coll);

	auto* level = g_GameFlow->GetLevel(CurrentLevel);

	// TODO: Subsuit gradually slowed down at rate of 0.5 degrees. @Sezz 2022.06.23
	// Apply and reset turn rate.
	item->Pose.Orientation.y += lara->Control.TurnRate;
	if (!(TrInput & (IN_LEFT | IN_RIGHT)))
		lara->Control.TurnRate = 0;

	if (level->GetLaraType() == LaraType::Divesuit)
		UpdateLaraSubsuitAngles(item);

	if (!lara->Control.IsMoving && !(TrInput & (IN_LEFT | IN_RIGHT)))
		ResetLaraLean(item, 8.0f, true, false);

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

	if (lara->WaterCurrentActive && lara->Control.WaterStatus != WaterStatus::FlyCheat)
		LaraWaterCurrent(item, coll);

	AnimateLara(item);
	TranslateItem(item, item->Pose.Orientation, item->Animation.Velocity.y);

	DoObjectCollision(item, coll);

	if (/*lara->ExtraAnim == -1 &&*/ lara->Vehicle == NO_ITEM)
		lara_collision_routines[item->Animation.ActiveState](item, coll);

	UpdateLaraRoom(item, 0);

	HandleWeapon(item);

	ProcessSectorFlags(item);
	TestTriggers(item, false);
	TestVolumes(Lara.ItemNumber);
}

void LaraCheat(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	item->HitPoints = LARA_HEALTH_MAX;
	LaraUnderwater(item, coll);

	if (TrInput & IN_WALK && !(TrInput & IN_LOOK))
	{
		if (TestEnvironment(ENV_FLAG_WATER, item) || (lara->WaterSurfaceDist > 0 && lara->WaterSurfaceDist != NO_HEIGHT))
		{
			SetAnimation(item, LA_UNDERWATER_IDLE);
			ResetLaraFlex(item);
			lara->Control.WaterStatus = WaterStatus::Underwater;
		}
		else
		{
			SetAnimation(item, LA_STAND_SOLID);
			item->Pose.Orientation.x = 0;
			item->Pose.Orientation.z = 0;
			ResetLaraFlex(item);
			lara->Control.WaterStatus = WaterStatus::Dry;
		}

		InitialiseLaraMeshes(item);
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
	auto dbInput = DbInput;
	auto trInput = TrInput;

	if (isTitle)
		ClearAllActions();

	// Control Lara.
	InItemControlLoop = true;
	LaraControl(item, &LaraCollision);
	LaraCheatyBits(item);
	InItemControlLoop = false;
	KillMoveItems();

	if (isTitle)
	{
		ActionMap = actionMap;
		DbInput = dbInput;
		TrInput = trInput;
	}

	if (g_Gui.GetInventoryItemChosen() != NO_ITEM)
	{
		g_Gui.SetInventoryItemChosen(NO_ITEM);
		SayNo();
	}

	// Update player animations.
	g_Renderer.UpdateLaraAnimations(true);

	// Update player effects.
	HandlePlayerWetnessDrips(*item);
	HandlePlayerDiveBubbles(*item);
	HairControl(item, g_GameFlow->GetLevel(CurrentLevel)->GetLaraType() == LaraType::Young);
	ProcessEffects(item);
}

// Offset values may be used to account for the quirk of room traversal only being able to occur at portals.
bool UpdateLaraRoom(ItemInfo* item, int height, int xOffset, int zOffset)
{
	auto point = Geometry::TranslatePoint(item->Pose.Position, item->Pose.Orientation.y, zOffset, height, xOffset);

	// Hacky L-shaped Location traversal.
	item->Location = GetRoom(item->Location, point.x, point.y, point.z);
	item->Location = GetRoom(item->Location, item->Pose.Position.x, point.y, item->Pose.Position.z);
	item->Floor = GetFloorHeight(item->Location, item->Pose.Position.x, item->Pose.Position.z).value_or(NO_HEIGHT);

	if (item->RoomNumber != item->Location.roomNumber)
	{
		ItemNewRoom(item->Index, item->Location.roomNumber);
		return true;
	}

	return false;
}
