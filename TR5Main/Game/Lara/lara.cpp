#include "framework.h"
#include "Game/Lara/lara.h"

#include "Game/Lara/lara_basic.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/Lara/lara_jump.h"
#include "Game/Lara/lara_tests.h"
#include "Game/Lara/lara_monkey.h"
#include "Game/Lara/lara_crawl.h"
#include "Game/Lara/lara_objects.h"
#include "Game/Lara/lara_hang.h"
#include "Game/Lara/lara_slide.h"
#include "Game/Lara/lara_fire.h"
#include "Game/Lara/lara_surface.h"
#include "Game/Lara/lara_swim.h"
#include "Game/Lara/lara_one_gun.h"
#include "Game/Lara/lara_cheat.h"
#include "Game/Lara/lara_climb.h"
#include "Game/Lara/lara_overhang.h"
#include "Game/Lara/lara_initialise.h"

#include "Game/animation.h"
#include "Game/camera.h"
#include "Game/collision/collide_item.h"
#include "Game/control/flipeffect.h"
#include "Game/control/volume.h"
#include "Game/effects/lara_fx.h"
#include "Game/effects/tomb4fx.h"
#include "Game/gui.h"
#include "Game/items.h"
#include "Game/misc.h"
#include "Game/savegame.h"
#include "Scripting/GameFlowScript.h"
#include "Sound/sound.h"
#include "Renderer/Renderer11.h"

using namespace TEN::Effects::Lara;
using namespace TEN::Control::Volumes;
using std::function;
using TEN::Renderer::g_Renderer;

LaraInfo Lara;
ITEM_INFO* LaraItem;
COLL_INFO LaraCollision = {};
byte LaraNodeUnderwater[NUM_LARA_MESHES];

function<LaraRoutineFunction> lara_control_routines[NUM_LARA_STATES + 1] = 
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
	lara_as_tread,
	lara_void_func,
	lara_as_jump_prepare,//15
	lara_as_walk_back,//16
	lara_as_swim,//17
	lara_as_glide,//18
	lara_as_controlled_no_look,//19
	lara_as_turn_right_fast,//20
	lara_as_step_right,//21
	lara_as_step_left,//22
	lara_as_roll_back,
	lara_as_slide_forward,//24
	lara_as_jump_back,//25
	lara_as_jump_right,//26
	lara_as_jump_left,//27
	lara_as_jump_up,//28
	lara_as_fall_back,//29
	lara_as_hangleft,//30
	lara_as_hangright,//31
	lara_as_slide_back,//32
	lara_as_surftread,
	lara_as_surfswim,
	lara_as_dive,
	lara_as_pushblock,//36
	lara_as_pullblock,//37
	lara_as_ppready,//38
	lara_as_pickup,//39
	lara_as_switchon,//40
	lara_as_switchoff,//41
	lara_as_usekey,//42
	lara_as_usepuzzle,//43
	lara_as_uwdeath,//44
	lara_as_roll_forward,//45
	lara_as_special,//46
	lara_as_surfback,//47
	lara_as_surfleft,//48
	lara_as_surfright,//49
	lara_void_func,//50
	lara_void_func,//51
	lara_as_swan_dive,//52
	lara_as_freefall_dive,//53
	lara_as_gymnast,//54
	lara_as_waterout,//55
	lara_as_climbstnc,//56
	lara_as_climbing,//57
	lara_as_climbleft,//58
	lara_as_climbend,//59
	lara_as_climbright,//60
	lara_as_climbdown,//61
	lara_void_func,//62
	lara_void_func,//63
	lara_void_func,//64
	lara_as_wade_forward,//65
	lara_as_waterroll,//66
	lara_as_pickupflare,//67
	lara_void_func,//68
	lara_void_func,//69
	lara_as_deathslide,//70
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
	lara_as_ropel,
	lara_as_roper,
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
	lara_as_corner,//107
	lara_as_corner,//108
	lara_as_corner,//109
	lara_as_corner,//110
	lara_as_rope,//111
	lara_as_climbrope,//112
	lara_as_climbroped,//113
	lara_as_rope,//114
	lara_as_rope,//115
	lara_void_func,
	lara_as_controlled,
	lara_as_swimcheat,
	lara_as_trpose,//119
	lara_as_controlled_no_look,//120
	lara_as_trwalk,//121
	lara_as_trfall,//122
	lara_as_trfall,//123
	lara_as_null,//124
#ifdef NEW_TIGHTROPE
	lara_as_trexit,//125
#else // !NEW_TIGHTROPE
	lara_as_null,//125
#endif
	lara_as_switchon,//126
	lara_as_null,//127
	lara_as_parallelbars,//128
	lara_as_pbleapoff,//129
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
	lara_as_stepoff_left,
	lara_as_stepoff_right,
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
	lara_as_null,//164
	lara_as_null,//165
	lara_as_null,//166
	lara_as_null,//167
	lara_as_null,//168
	lara_as_null,//169
};

function<LaraRoutineFunction> lara_collision_routines[NUM_LARA_STATES + 1] =
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
	lara_col_tread,
	lara_col_land,
	lara_col_jump_prepare,//15
	lara_col_walk_back,
	lara_col_swim,
	lara_col_glide,
	lara_default_col,//19
	lara_col_turn_right_fast,
	lara_col_step_right,
	lara_col_step_left,
	lara_col_roll_back,
	lara_col_slide_forward,//24
	lara_col_jump_back,//25
	lara_col_jump_right,//26
	lara_col_jump_left,//27
	lara_col_jump_up,//28
	lara_col_fall_back,//29
	lara_col_hangleft,
	lara_col_hangright,
	lara_col_slide_back,//32
	lara_col_surftread,
	lara_col_surfswim,
	lara_col_dive,
	lara_default_col,
	lara_default_col,
	lara_default_col,
	lara_default_col,
	lara_default_col,
	lara_default_col,
	lara_default_col,
	lara_default_col,
	lara_col_uwdeath,
	lara_col_roll_forward,
	lara_void_func,
	lara_col_surfback,
	lara_col_surfleft,
	lara_col_surfright,
	lara_void_func,
	lara_void_func,
	lara_col_swan_dive,//52
	lara_col_freefall_dive,//53
	lara_default_col,
	lara_default_col,
	lara_col_climbstnc,
	lara_col_climbing,
	lara_col_climbleft,
	lara_col_climbend,
	lara_col_climbright,
	lara_col_climbdown,
	lara_col_auto_jump,//62
	lara_void_func,
	lara_void_func,
	lara_col_wade_forward,
	lara_col_waterroll,
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
	lara_col_crawl_forward,
	lara_col_monkey_turn_left,//81
	lara_col_monkey_turn_right,//82
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
	lara_col_turnswitch,
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
	lara_default_col,
	lara_default_col,
	lara_default_col,
	lara_default_col,
	lara_col_rope,
	lara_void_func,
	lara_void_func,
	lara_col_ropefwd,
	lara_col_ropefwd,
	lara_void_func,
	lara_void_func,
	lara_col_swim,
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
	lara_col_vault,//164
	lara_col_vault,//165
	lara_col_vault,//166
	lara_col_vault,//167
	lara_col_vault,//168
	lara_col_vault,//169
};

void LaraControl(ITEM_INFO* item, COLL_INFO* coll)
{
	auto info = GetLaraInfo(item);

	if (info->Control.WeaponControl.HasFired)
	{
		AlertNearbyGuards(item);
		info->Control.WeaponControl.HasFired = false;
	}

	if (info->poisoned)
	{
		if (info->poisoned > 4096)
			info->poisoned = 4096;

		if (info->poisoned >= 256 && !(Wibble & 0xFF))
			item->HitPoints -= info->poisoned >> 8;
	}

	if (info->Control.IsMoving)
	{
		if (info->Control.Count.PositionAdjust > 90)
		{
			info->Control.IsMoving = false;
			info->Control.HandStatus = HandStatus::Free;
		}

		++info->Control.Count.PositionAdjust;
	}

	if (!info->Control.Locked)
		info->locationPad = 128;

	int oldX = item->Position.xPos;
	int oldY = item->Position.yPos;
	int oldZ = item->Position.zPos;

	// Set hands free failsafe.
	if (info->Control.HandStatus == HandStatus::Busy &&
		item->ActiveState == LS_IDLE &&
		item->TargetState == LS_IDLE &&
		item->AnimNumber == LA_STAND_IDLE &&
		!item->Airborne)
	{
		info->Control.HandStatus = HandStatus::Free;
	}

	if (item->ActiveState != LS_SPRINT && info->SprintEnergy < LARA_SPRINT_MAX)
		info->SprintEnergy++;

	info->Control.IsLow = false;

	bool isWater = TestEnvironment(ENV_FLAG_WATER, item);
	bool isSwamp = TestEnvironment(ENV_FLAG_SWAMP, item);

	int waterDepth = GetWaterDepth(item);
	int waterHeight = GetWaterHeight(item);

	int heightFromWater;
	if (waterHeight != NO_HEIGHT)
		heightFromWater = item->Position.yPos - waterHeight;
	else
		heightFromWater = NO_HEIGHT;
	info->WaterSurfaceDist = -heightFromWater;

	if (info->Vehicle == NO_ITEM)
		WadeSplash(item, waterHeight, waterDepth);

	if (info->Vehicle == NO_ITEM && info->ExtraAnim == -1)
	{
		switch (info->Control.WaterStatus)
		{
		case WaterStatus::Dry:
			if (heightFromWater == NO_HEIGHT || heightFromWater < WADE_DEPTH)
				break;

			Camera.targetElevation = -ANGLE(22.0f);

			// Water is deep enough to swim; dispatch dive.
			if (waterDepth >= SWIM_DEPTH &&
				!isSwamp)
			{
				if (isWater)
				{
					info->Air = LARA_AIR_MAX;
					info->Control.WaterStatus = WaterStatus::Underwater;
					item->Airborne = false;
					item->Position.yPos += 100;

					UpdateItemRoom(item, 0);
					StopSoundEffect(SFX_TR4_LARA_FALL);

					if (item->ActiveState == LS_SWAN_DIVE)
					{
						info->Control.HandStatus = HandStatus::Free;
						item->Position.xRot = -ANGLE(45.0f);
						item->TargetState = LS_FREEFALL_DIVE;
						AnimateLara(item);
						item->VerticalVelocity *= 2;
					}
					else if (item->ActiveState == LS_FREEFALL_DIVE)
					{
						info->Control.HandStatus = HandStatus::Free;
						item->Position.xRot = -ANGLE(85.0f);
						item->TargetState = LS_FREEFALL_DIVE;
						AnimateLara(item);
						item->VerticalVelocity *= 2;
					}
					else
					{
						item->Position.xRot = -ANGLE(45.0f);
						SetAnimation(item, LA_FREEFALL_DIVE);
						item->VerticalVelocity = 3 * item->VerticalVelocity / 2;
					}

					ResetLaraFlex(item);
					Splash(item);
				}
			}
			// Water is at wade depth; update water status and do special handling.
			else if (heightFromWater >= WADE_DEPTH)
			{
				info->Control.WaterStatus = WaterStatus::Wade;

				// Make splash ONLY within this particular threshold before swim depth while Airborne (WadeSplash() above interferes otherwise).
				if (waterDepth > (SWIM_DEPTH - CLICK(1)) &&
					!isSwamp &&
					item->Airborne)
				{
					Splash(item);
					item->TargetState = LS_IDLE;
				}
				// Lara is grounded; don't splash again.
				else if (!item->Airborne)
					item->TargetState = LS_IDLE;
				else if (isSwamp)
				{
					if (item->ActiveState == LS_SWAN_DIVE ||
						item->ActiveState == LS_FREEFALL_DIVE)
					{
						item->Position.yPos = waterHeight + (WALL_SIZE - 24);
					}

					SetAnimation(item, LA_WADE);
				}
			}

			break;

		case WaterStatus::Underwater:
			if (isWater ||
				waterDepth == DEEP_WATER ||
				abs(heightFromWater) >= CLICK(1) ||
				item->AnimNumber == LA_UNDERWATER_RESURFACE ||
				item->AnimNumber == LA_ONWATER_DIVE)
			{
				if (!isWater)
				{
					if (waterDepth == DEEP_WATER || abs(heightFromWater) >= CLICK(1))
					{
						SetAnimation(item, LA_FALL_START);
						item->Velocity = item->VerticalVelocity / 4;
						item->Airborne = true;
						item->VerticalVelocity = 0;
						item->Position.zRot = 0;
						item->Position.xRot = 0;
						info->Control.WaterStatus = WaterStatus::Dry;
						ResetLaraFlex(item);
					}
					else
					{
						SetAnimation(item, LA_UNDERWATER_RESURFACE);
						item->Position.yPos = waterHeight;
						item->VerticalVelocity = 0;
						item->Position.zRot = 0;
						item->Position.xRot = 0;
						info->Control.WaterStatus = WaterStatus::TreadWater;
						info->Control.Count.Dive = 11;
						ResetLaraFlex(item);

						UpdateItemRoom(item, -(STEPUP_HEIGHT - 3));
						SoundEffect(SFX_TR4_LARA_BREATH, &item->Position, 2);
					}
				}
			}
			else
			{
				SetAnimation(item, LA_UNDERWATER_RESURFACE);
				item->Position.yPos = waterHeight + 1;
				item->VerticalVelocity = 0;
				item->Position.zRot = 0;
				item->Position.xRot = 0;
				info->Control.WaterStatus = WaterStatus::TreadWater;
				info->Control.Count.Dive = 11;
				ResetLaraFlex(item);

				UpdateItemRoom(item, 0);
				SoundEffect(SFX_TR4_LARA_BREATH, &item->Position, 2);
			}
			break;

		case WaterStatus::TreadWater:
			if (!isWater)
			{
				if (heightFromWater <= WADE_DEPTH)
				{
					SetAnimation(item, LA_FALL_START);
					item->Velocity = item->VerticalVelocity / 4;
					item->Airborne = true;
					info->Control.WaterStatus = WaterStatus::Dry;
				}
				else
				{
					SetAnimation(item, LA_STAND_IDLE);
					item->TargetState = LS_WADE_FORWARD; // TODO: Check if really needed? -- Lwmte, 10.11.21
					info->Control.WaterStatus = WaterStatus::Wade;

					AnimateItem(item);
				}

				item->VerticalVelocity = 0;
				item->Position.zRot = 0;
				item->Position.xRot = 0;
				ResetLaraFlex(item);
			}

			break;

		case WaterStatus::Wade:
			Camera.targetElevation = -ANGLE(22.0f);

			if (heightFromWater >= WADE_DEPTH)
			{
				if (heightFromWater > SWIM_DEPTH && !isSwamp)
				{
					SetAnimation(item, LA_ONWATER_IDLE);

					info->Control.WaterStatus = WaterStatus::TreadWater;
					item->Position.yPos += 1 - heightFromWater;
					item->Airborne = false;
					item->VerticalVelocity = 0;
					item->Position.zRot = 0;
					item->Position.xRot = 0;
					info->Control.Count.Dive = 0;
					ResetLaraFlex(item);

					UpdateItemRoom(item, 0);
				}
			}
			else
			{
				info->Control.WaterStatus = WaterStatus::Dry;

				if (item->ActiveState == LS_WADE_FORWARD)
					item->TargetState = LS_RUN_FORWARD;
			}

			break;
		}
	}

	if (item->HitPoints <= 0)
	{
		item->HitPoints = -1;

		if (info->Control.Count.Death == 0)
			StopSoundTracks();

		info->Control.Count.Death++;
		if ((item->Flags & 0x100))
		{
			info->Control.Count.Death++;

			return;
		}
	}

	switch (info->Control.WaterStatus)
	{
	case WaterStatus::Dry:
	case WaterStatus::Wade:
		if (isSwamp	&& info->WaterSurfaceDist < -(LARA_HEIGHT + 8)) // TODO: Find best height. @Sezz 2021.11.10
		{
			if (item->HitPoints >= 0)
			{
				info->Air -= 6;
				if (info->Air < 0)
				{
					info->Air = -1;
					item->HitPoints -= 10;
				}
			}
		}
		else if (info->Air < LARA_AIR_MAX && item->HitPoints >= 0)
		{
			if (info->Vehicle == NO_ITEM) // only for the upv !!
			{
				info->Air += 10;
				if (info->Air > LARA_AIR_MAX)
					info->Air = LARA_AIR_MAX;
			}
		}

		LaraAboveWater(item, coll);

		break;

	case WaterStatus::Underwater:
		if (item->HitPoints >= 0)
		{
			auto level = g_GameFlow->GetLevel(CurrentLevel);
			if (level->LaraType != LaraType::Divesuit)
				info->Air--;

			if (info->Air < 0)
			{
			//	if (LaraDrawType == LARA_TYPE::DIVESUIT && info->anxiety < 251)
			//		info->anxiety += 4;
				info->Air = -1;
				item->HitPoints -= 5;
			}
		}

		LaraUnderWater(item, coll);

		break;

	case WaterStatus::TreadWater:
		if (item->HitPoints >= 0)
		{
			info->Air += 10;
			if (info->Air > LARA_AIR_MAX)
				info->Air = LARA_AIR_MAX;
		}

		LaraSurface(item, coll);

		break;

	case WaterStatus::FlyCheat:
		LaraCheat(item, coll);

		break;
	}

	Statistics.Game.Distance += sqrt(
		SQUARE(item->Position.xPos - oldX) +
		SQUARE(item->Position.yPos - oldY) +
		SQUARE(item->Position.zPos - oldZ));
}

void LaraAboveWater(ITEM_INFO* item, COLL_INFO* coll)
{
	auto info = GetLaraInfo(item);

	coll->Setup.UpperCeilingBound = NO_UPPER_BOUND;

	coll->Setup.OldPosition.x = item->Position.xPos;
	coll->Setup.OldPosition.y = item->Position.yPos;
	coll->Setup.OldPosition.z = item->Position.zPos;
	coll->Setup.OldState = item->ActiveState;
	coll->Setup.OldAnimNumber = item->AnimNumber;
	coll->Setup.OldFrameNumber = item->FrameNumber;

	coll->Setup.EnableObjectPush = true;
	coll->Setup.EnableSpasm = true;
	coll->Setup.FloorSlopeIsWall = false;
	coll->Setup.FloorSlopeIsPit = false;
	coll->Setup.CeilingSlopeIsWall = false;
	coll->Setup.DeathFlagIsPit = false;
	coll->Setup.NoMonkeyFlagIsWall = false;
	coll->Setup.Mode = COLL_PROBE_MODE::QUADRANTS;

	if (TrInput & IN_LOOK && info->Control.CanLook &&
		info->ExtraAnim == NO_ITEM)
	{
		LookLeftRight();
	}
	else if (coll->Setup.Height > LARA_HEIGHT - LARA_HEADROOM) // TEMP HACK: Look feature will need a dedicated refactor; ResetLook() interferes with crawl flexing. @Sezz 2021.12.10
		ResetLook(item);

	// TODO: Move radius and height default resets above look checks when
	coll->Setup.Radius = LARA_RAD;
	coll->Setup.Height = LARA_HEIGHT;
	info->Control.CanLook = true;

	UpdateItemRoom(item, -LARA_HEIGHT / 2);

	// Process vehicles.
	HandleLaraVehicle(item, coll);

	// Temp. debug stuff
	//---

	// Kill Lara.
	if (KeyMap[DIK_D])
		item->HitPoints = 0;

	// Say no.
	static bool dbNo = false;
	if (KeyMap[DIK_N] && !dbNo)
		SayNo();
	dbNo = KeyMap[DIK_N] ? true : false;

	static PHD_3DPOS posO = item->Position;
	static short roomNumO = item->RoomNumber;
	static CAMERA_INFO camO = Camera;

	// Save position.
	if (KeyMap[DIK_Q] && TrInput & IN_WALK)
	{
		posO = item->Position;
		roomNumO = item->RoomNumber;
		camO = Camera;
	}
	
	// Restore position.
	if (KeyMap[DIK_E])
	{
		item->Position = posO;
		item->RoomNumber = roomNumO;
		Camera = camO;
	}
	
	// Forward 1 unit.
	if (KeyMap[DIK_I])
	{
		item->Position.xPos += roundf(phd_sin(item->Position.yRot));
		item->Position.zPos += roundf(phd_cos(item->Position.yRot));
	}
	// Back 1 unit.
	else if (KeyMap[DIK_K])
	{
		item->Position.xPos += roundf(phd_sin(item->Position.yRot + ANGLE(180.0f)));
		item->Position.zPos += roundf(phd_cos(item->Position.yRot + ANGLE(180.0f)));
	}
	// Left 1 unit.
	else if (KeyMap[DIK_J])
	{
		item->Position.xPos += roundf(phd_sin(item->Position.yRot - ANGLE(90.0f)));
		item->Position.zPos += roundf(phd_cos(item->Position.yRot - ANGLE(90.0f)));
	}
	// Right 1 unit.
	else if (KeyMap[DIK_L])
	{
		item->Position.xPos += roundf(phd_sin(item->Position.yRot + ANGLE(90.0f)));
		item->Position.zPos += roundf(phd_cos(item->Position.yRot + ANGLE(90.0f)));
	}

	//---

	// Temp. debug stuff.
	static bool doRoutines = true;
	static bool dbT = false;
	if (KeyMap[DIK_T] && !dbT)
		doRoutines = !doRoutines;
	dbT = KeyMap[DIK_T] ? true : false;

	static bool dbU = false;
	if (doRoutines || KeyMap[DIK_U] && !dbU)
	{
		// Handle current Lara status.
		lara_control_routines[item->ActiveState](item, coll);

		// Animate Lara.
		AnimateLara(item);

		HandleLaraMovementParameters(item, coll);

		if (info->ExtraAnim == -1)
		{
			// Check for collision with items.
			DoObjectCollision(item, coll);

			// Handle Lara collision.
			if (info->Vehicle == NO_ITEM)
				lara_collision_routines[item->ActiveState](item, coll);
		}
	}
	dbU = KeyMap[DIK_U] ? true : false;

	//if (info->gunType == WEAPON_CROSSBOW && !LaserSight)
	//	TrInput &= ~IN_ACTION;

	// Handle weapons.
	LaraGun(item);

	// Handle breath.
	LaraBreath(item);

	// Test for flags and triggers.
	ProcessSectorFlags(item);
	TestTriggers(item, false);
	TestVolumes(item);
}

void LaraUnderWater(ITEM_INFO* item, COLL_INFO* coll)
{
	auto info = GetLaraInfo(item);

	coll->Setup.LowerFloorBound = NO_LOWER_BOUND;
	coll->Setup.UpperFloorBound = -(LARA_RAD_UNDERWATER + (LARA_RAD_UNDERWATER / 3));
	coll->Setup.LowerCeilingBound = LARA_RAD_UNDERWATER + (LARA_RAD_UNDERWATER / 3);
	coll->Setup.UpperCeilingBound = NO_UPPER_BOUND;

	coll->Setup.OldPosition.x = item->Position.xPos;
	coll->Setup.OldPosition.y = item->Position.yPos;
	coll->Setup.OldPosition.z = item->Position.zPos;

	coll->Setup.FloorSlopeIsWall = false;
	coll->Setup.FloorSlopeIsPit = false;
	coll->Setup.CeilingSlopeIsWall = false;
	coll->Setup.DeathFlagIsPit = false;
	coll->Setup.NoMonkeyFlagIsWall = false;
	coll->Setup.EnableObjectPush = true;
	coll->Setup.EnableSpasm = false;
	coll->Setup.Mode = COLL_PROBE_MODE::QUADRANTS;

	coll->Setup.Radius = LARA_RAD_UNDERWATER;
	coll->Setup.Height = LARA_HEIGHT;

	if (TrInput & IN_LOOK && info->Control.CanLook)
		LookLeftRight();
	else
		ResetLook(item);

	info->Control.CanLook = true;
	info->Control.Count.Pose = 0;

	lara_control_routines[item->ActiveState](item, coll);

	auto level = g_GameFlow->GetLevel(CurrentLevel);

	if (level->LaraType == LaraType::Divesuit)
	{
		if (info->Control.TurnRate < -ANGLE(0.5f))
			info->Control.TurnRate += ANGLE(0.5f);
		else if (info->Control.TurnRate > ANGLE(0.5f))
			info->Control.TurnRate -= ANGLE(0.5f);
		else
			info->Control.TurnRate = 0;
	}
	else if (info->Control.TurnRate < -ANGLE(2.0f))
		info->Control.TurnRate += ANGLE(2.0f);
	else if (info->Control.TurnRate > ANGLE(2.0f))
		info->Control.TurnRate -= ANGLE(2.0f);
	else
		info->Control.TurnRate = 0;

	item->Position.yRot += info->Control.TurnRate;

	if (level->LaraType == LaraType::Divesuit)
		UpdateSubsuitAngles();

	if (!info->Control.IsMoving && !(TrInput & (IN_LEFT | IN_RIGHT)))
	{
		if (abs(item->Position.zRot) > 0)
			item->Position.zRot += item->Position.zRot / -8;
	}

	if (item->Position.xRot < -ANGLE(85.0f))
		item->Position.xRot = -ANGLE(85.0f);
	else if (item->Position.xRot > ANGLE(85.0f))
		item->Position.xRot = ANGLE(85.0f);

	if (level->LaraType == LaraType::Divesuit)
	{
		if (item->Position.zRot > ANGLE(44.0f))
			item->Position.zRot = ANGLE(44.0f);
		else if (item->Position.zRot < -ANGLE(44.0f))
			item->Position.zRot = -ANGLE(44.0f);
	}
	else
	{
		if (item->Position.zRot > ANGLE(22.0f))
			item->Position.zRot = ANGLE(22.0f);
		else if (item->Position.zRot < -ANGLE(22.0f))
			item->Position.zRot = -ANGLE(22.0f);
	}

	if (info->Control.WaterCurrentActive && info->Control.WaterStatus != WaterStatus::FlyCheat)
		LaraWaterCurrent(coll);

	AnimateLara(item);

	item->Position.xPos += phd_cos(item->Position.xRot) * item->VerticalVelocity * phd_sin(item->Position.yRot) / 4;
	item->Position.yPos -= item->VerticalVelocity * phd_sin(item->Position.xRot) / 4;
	item->Position.zPos += phd_cos(item->Position.xRot) * item->VerticalVelocity * phd_cos(item->Position.yRot) / 4;

	DoObjectCollision(item, coll);

	if (/*info->ExtraAnim == -1 &&*/ info->Vehicle == NO_ITEM)
		lara_collision_routines[item->ActiveState](item, coll);

	UpdateItemRoom(item, 0);

	LaraGun(item);

	ProcessSectorFlags(item);
	TestTriggers(item, false);
	TestVolumes(item);
}

void LaraSurface(ITEM_INFO* item, COLL_INFO* coll)
{
	auto info = GetLaraInfo(item);

	Camera.targetElevation = -ANGLE(22.0f);

	coll->Setup.LowerFloorBound = NO_LOWER_BOUND;
	coll->Setup.UpperFloorBound = -CLICK(0.5f);
	coll->Setup.LowerCeilingBound = LARA_RAD;
	coll->Setup.UpperCeilingBound = NO_UPPER_BOUND;

	coll->Setup.OldPosition.x = item->Position.xPos;
	coll->Setup.OldPosition.y = item->Position.yPos;
	coll->Setup.OldPosition.z = item->Position.zPos;

	coll->Setup.FloorSlopeIsWall = false;
	coll->Setup.FloorSlopeIsPit = false;
	coll->Setup.CeilingSlopeIsWall = false;
	coll->Setup.DeathFlagIsPit = false;
	coll->Setup.NoMonkeyFlagIsWall = false;
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpasm = false;
	coll->Setup.Mode = COLL_PROBE_MODE::FREE_FORWARD;

	coll->Setup.Radius = LARA_RAD;
	coll->Setup.Height = LARA_HEIGHT_SURFACE;

	if (TrInput & IN_LOOK && info->Control.CanLook)
		LookLeftRight();
	else
		ResetLook(item);

	info->Control.CanLook = true;
	info->Control.Count.Pose = 0;

	lara_control_routines[item->ActiveState](item, coll);

	if (!info->Control.IsMoving && !(TrInput & (IN_LEFT | IN_RIGHT)))
	{
		if (abs(item->Position.zRot) > 0)
			item->Position.zRot += item->Position.zRot / -8;
	}

	if (info->Control.WaterCurrentActive && info->Control.WaterStatus != WaterStatus::FlyCheat)
		LaraWaterCurrent(coll);

	AnimateLara(item);

	item->Position.xPos += item->VerticalVelocity * phd_sin(info->Control.MoveAngle) / 4;
	item->Position.zPos += item->VerticalVelocity * phd_cos(info->Control.MoveAngle) / 4;

	DoObjectCollision(item, coll);

	if (info->Vehicle == NO_ITEM)
		lara_collision_routines[item->ActiveState](item, coll);

	UpdateItemRoom(item, LARA_RAD);

	LaraGun(item);

	ProcessSectorFlags(item);
	TestTriggers(item, false);
	TestVolumes(item);
}

void LaraCheat(ITEM_INFO* item, COLL_INFO* coll)
{
	auto info = GetLaraInfo(item);

	item->HitPoints = LARA_HEALTH_MAX;
	LaraUnderWater(item, coll);

	if (TrInput & IN_WALK && !(TrInput & IN_LOOK))
	{
		if (TestEnvironment(ENV_FLAG_WATER, item) || (info->WaterSurfaceDist > 0 && info->WaterSurfaceDist != NO_HEIGHT))
		{
			info->Control.WaterStatus = WaterStatus::Underwater;
			SetAnimation(item, LA_UNDERWATER_IDLE);
			ResetLaraFlex(item);
		}
		else
		{
			info->Control.WaterStatus = WaterStatus::Dry;
			SetAnimation(item, LA_STAND_SOLID);
			item->Position.zRot = 0;
			item->Position.xRot = 0;
			ResetLaraFlex(item);
		}

		info->Control.HandStatus = HandStatus::Free;
		LaraInitialiseMeshes();
		item->HitPoints = LARA_HEALTH_MAX;
	}
}
