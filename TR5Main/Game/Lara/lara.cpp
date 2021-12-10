#include "framework.h"

#include "Lara.h"
#include "lara_basic.h"
#include "lara_tests.h"
#include "lara_monkey.h"
#include "lara_crawl.h"
#include "lara_objects.h"
#include "lara_hang.h"
#include "lara_slide.h"
#include "lara_fire.h"
#include "lara_surface.h"
#include "lara_swim.h"
#include "lara_one_gun.h"
#include "lara_cheat.h"
#include "lara_climb.h"
#include "lara_initialise.h"

#include "motorbike.h"
#include "biggun.h"
#include "quad.h"
#include "snowmobile.h"
#include "jeep.h"
#include "upv.h"
#include "kayak.h"
#include "minecart.h"

#include "animation.h"
#include "GameFlowScript.h"
#include "flipeffect.h"
#include "Sound/sound.h"
#include "savegame.h"
#include "rope.h"
#include "misc.h"
#include "control/volume.h"
#include "Renderer11.h"
#include "camera.h"
#include "items.h"
#include "gui.h"

#include "Game/effects/lara_fx.h"
#include "Game/effects/tomb4fx.h"

using namespace TEN::Effects::Lara;
using namespace TEN::Entities::Generic;
using namespace TEN::Control::Volumes;
using std::function;
using TEN::Renderer::g_Renderer;

LaraInfo Lara;
ITEM_INFO* LaraItem;
COLL_INFO LaraCollision = {};
byte LaraNodeUnderwater[NUM_LARA_MESHES];

function<LaraRoutineFunction> lara_control_routines[NUM_LARA_STATES + 1] = 
{
	lara_as_walk,
	lara_as_run,
	lara_as_stop,
	lara_as_forwardjump,
	lara_void_func,//4
	lara_as_fastback,//5
	lara_as_turn_r,//6
	lara_as_turn_l,//7
	lara_as_death,//8
	lara_as_fastfall,
	lara_as_hang,
	lara_as_reach,
	lara_as_splat,
	lara_as_tread,
	lara_void_func,
	lara_as_compress,//15
	lara_as_back,//16
	lara_as_swim,//17
	lara_as_glide,//18
	lara_as_null,//19
	lara_as_fastturn,//20
	lara_as_stepright,//21
	lara_as_stepleft,//22
	lara_void_func,
	lara_as_slide,//24
	lara_as_backjump,//25
	lara_as_rightjump,//26
	lara_as_leftjump,//27
	lara_as_upjump,//28
	lara_as_fallback,//29
	lara_as_hangleft,//30
	lara_as_hangright,//31
	lara_as_slideback,//32
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
	lara_void_func,//45
	lara_as_special,//46
	lara_as_surfback,//47
	lara_as_surfleft,//48
	lara_as_surfright,//49
	lara_void_func,//50
	lara_void_func,//51
	lara_as_swandive,//52
	lara_as_fastdive,//53
	lara_as_gymnast,//54
	lara_as_waterout,
	lara_as_climbstnc,
	lara_as_climbing,
	lara_as_climbleft,
	lara_as_climbend,
	lara_as_climbright,
	lara_as_climbdown,//
	lara_void_func,
	lara_void_func,
	lara_void_func,
	lara_as_wade,//65
	lara_as_waterroll,//66
	lara_as_pickupflare,//67
	lara_void_func,//68
	lara_void_func,//69
	lara_as_deathslide,//70
	lara_as_duck,//71
	lara_as_crouch_roll,//72
	lara_as_dash,
	lara_as_dashdive,
	lara_as_monkey_idle,
	lara_as_monkeyswing,
	lara_as_monkeyl,
	lara_as_monkeyr,
	lara_as_monkey180,
	lara_as_all4s,//80
	lara_as_crawl,//81
	lara_as_hangturnl,
	lara_as_hangturnr,
	lara_as_all4turnl,//84
	lara_as_all4turnr,//85
	lara_as_crawlb,//86
	lara_as_null,
	lara_as_null,
	lara_as_controlled,
	lara_as_ropel,
	lara_as_roper,
	lara_as_controlled,
	lara_as_controlled,
	lara_as_controlled,
	lara_as_controlledl,
	lara_as_controlledl,
	lara_as_controlled,
	lara_as_pickup,//98
	lara_as_pole_idle,//99
	lara_as_pole_up,//100
	lara_as_pole_down,//101
	lara_as_pole_turn_clockwise,//102
	lara_as_pole_turn_counter_clockwise,//103
	lara_as_pulley,//104
	lara_as_duckl,//105
	lara_as_duckr,//106
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
	lara_as_null,//120
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
	lara_as_null,//131
	lara_as_null,//132
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
	lara_as_null,// 143 - Unused
	lara_as_null,// 144 - Unused
	lara_as_null,// 145 - Unused
	lara_as_controlledl,
	lara_as_null,
	lara_as_null,
	lara_as_null,
	lara_as_stepoff_left,
	lara_as_stepoff_right
};

function<LaraRoutineFunction> lara_collision_routines[NUM_LARA_STATES + 1] = {
	lara_col_walk,
	lara_col_run,
	lara_col_stop,
	lara_col_forwardjump,
	lara_col_pose,
	lara_col_fastback,
	lara_col_turn_r,
	lara_col_turn_l,
	lara_col_death,
	lara_col_fastfall,
	lara_col_hang,
	lara_col_reach,
	lara_col_splat,
	lara_col_tread,
	lara_col_land,
	lara_col_compress,
	lara_col_back,
	lara_col_swim,
	lara_col_glide,
	lara_default_col,
	lara_col_fastturn,
	lara_col_stepright,
	lara_col_stepleft,
	lara_col_roll2,
	lara_col_slide,
	lara_col_backjump,
	lara_col_rightjump,
	lara_col_leftjump,
	lara_col_upjump,
	lara_col_fallback,
	lara_col_hangleft,
	lara_col_hangright,
	lara_col_slideback,
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
	lara_col_roll,
	lara_void_func,
	lara_col_surfback,
	lara_col_surfleft,
	lara_col_surfright,
	lara_void_func,
	lara_void_func,
	lara_col_swandive,
	lara_col_fastdive,
	lara_default_col,
	lara_default_col,
	lara_col_climbstnc,
	lara_col_climbing,
	lara_col_climbleft,
	lara_col_climbend,
	lara_col_climbright,
	lara_col_climbdown,
	lara_void_func,
	lara_void_func,
	lara_void_func,
	lara_col_wade,
	lara_col_waterroll,
	lara_default_col,
	lara_void_func,
	lara_void_func,
	lara_void_func,
	lara_col_duck,
	lara_col_crouch_roll,
	lara_col_dash,
	lara_col_dashdive,
	lara_col_monkey_idle,
	lara_col_monkeyswing,
	lara_col_monkeyl,
	lara_col_monkeyr,
	lara_col_monkey180,
	lara_col_all4s,
	lara_col_crawl,
	lara_col_hangturnlr,
	lara_col_hangturnlr,
	lara_col_all4turnlr,
	lara_col_all4turnlr,
	lara_col_crawlb,
	lara_void_func,
	lara_col_crawl2hang,
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
	lara_col_ducklr,
	lara_col_ducklr,
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
	lara_void_func, // 143 - Unused
	lara_void_func, // 144 - Unused
	lara_void_func, // 145 - Unused
	lara_void_func,
	lara_void_func,
	lara_void_func,
	lara_void_func,
	lara_default_col,
	lara_default_col
};

void LaraControl(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraCheatyBits();

	if (Lara.hasFired)
	{
		AlertNearbyGuards(item);
		Lara.hasFired = false;
	}

	if (Lara.poisoned)
	{
		if (Lara.poisoned > 4096)
			Lara.poisoned = 4096;

		if (Lara.poisoned >= 256 && !(Wibble & 0xFF))
			item->hitPoints -= Lara.poisoned >> 8;
	}

	if (Lara.isMoving)
	{
		if (Lara.moveCount > 90)
		{
			Lara.isMoving = false;
			Lara.gunStatus = LG_NO_ARMS;
		}

		++Lara.moveCount;
	}

	if (!Lara.uncontrollable)
		Lara.locationPad = 128;

	int oldX = item->pos.xPos;
	int oldY = item->pos.yPos;
	int oldZ = item->pos.zPos;

	if (Lara.gunStatus == LG_HANDS_BUSY &&
		item->currentAnimState == LS_STOP &&
		item->goalAnimState == LS_STOP &&
		item->animNumber == LA_STAND_IDLE &&
		!item->gravityStatus)
	{
		Lara.gunStatus = LG_NO_ARMS;
	}

	if (item->currentAnimState != LS_SPRINT && Lara.sprintTimer < LARA_SPRINT_MAX)
		Lara.sprintTimer++;

	Lara.isDucked = false;

	bool isWater = TestLaraWater(item);
	bool isSwamp = TestLaraSwamp(item);

	int waterDepth = GetWaterDepth(item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber);
	int waterHeight = GetWaterHeight(item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber);

	int heightFromWater;
	if (waterHeight != NO_HEIGHT)
		heightFromWater = item->pos.yPos - waterHeight;
	else
		heightFromWater = NO_HEIGHT;
	Lara.waterSurfaceDist = -heightFromWater;

	if (Lara.Vehicle == NO_ITEM)
		WadeSplash(item, waterHeight, waterDepth);

	TriggerLaraDrips(item);

	short roomNumber;

	if (Lara.Vehicle == NO_ITEM && Lara.ExtraAnim == -1)
	{
		switch (Lara.waterStatus)
		{
		case LW_ABOVE_WATER:
			if (heightFromWater == NO_HEIGHT || heightFromWater < WADE_DEPTH)
				break;

			Camera.targetElevation = -ANGLE(22.0f);


			if (waterDepth > (SWIM_DEPTH - STEP_SIZE) &&
				!isSwamp &&
				item->currentAnimState != LS_RUN_FORWARD &&		// Prevent ridiculous entrances when stepping down into wade-height water. @Sezz 2021.11.17
				item->currentAnimState != LS_WALK_FORWARD &&
				item->currentAnimState != LS_WALK_BACK)
			{
				if (isWater)
				{
					Lara.air = 1800;
					Lara.waterStatus = LW_UNDERWATER;
					item->gravityStatus = false;
					item->pos.yPos += 100;

					UpdateItemRoom(item, 0);
					StopSoundEffect(SFX_TR4_LARA_FALL);

					if (item->currentAnimState == LS_SWANDIVE_START)
					{
						item->pos.xRot = -ANGLE(45.0f);
						item->goalAnimState = LS_DIVE;
						AnimateLara(item);
						item->fallspeed *= 2;
					}
					else if (item->currentAnimState == LS_SWANDIVE_END)
					{
						item->pos.xRot = -ANGLE(85.0f);
						item->goalAnimState = LS_DIVE;
						AnimateLara(item);
						item->fallspeed *= 2;
					}
					else
					{
						item->pos.xRot = -ANGLE(45.0f);
						SetAnimation(item, LA_FREEFALL_DIVE);
						item->fallspeed = 3 * item->fallspeed / 2;
					}

					Lara.torsoYrot = 0;
					Lara.torsoXrot = 0;
					Lara.headYrot = 0;
					Lara.headXrot = 0;

					Splash(item);
				}
			}
			else if (heightFromWater > WADE_DEPTH)
			{
				Lara.waterStatus = LW_WADE;

				if (!item->gravityStatus)
					item->goalAnimState = LS_STOP;
				else if (isSwamp)
				{
					if (item->currentAnimState == LS_SWANDIVE_START || item->currentAnimState == LS_SWANDIVE_END)
						item->pos.yPos = waterHeight + (WALL_SIZE - 24);

					SetAnimation(item, LA_WADE);
				}
			}
			break;

		case LW_UNDERWATER:
			roomNumber = item->roomNumber;
			GetFloor(item->pos.xPos, item->pos.yPos - STEP_SIZE, item->pos.zPos, &roomNumber);

			if (isWater ||
				waterDepth == DEEP_WATER ||
				abs(heightFromWater) >= STEP_SIZE ||
				item->animNumber == LA_UNDERWATER_RESURFACE ||
				item->animNumber == LA_ONWATER_DIVE)
			{
				if (!isWater)
				{
					if (waterDepth == DEEP_WATER || abs(heightFromWater) >= STEP_SIZE)
					{
						SetAnimation(item, LA_FALL_START);
						item->speed = item->fallspeed / 4;
						item->gravityStatus = true;
						item->fallspeed = 0;
						item->pos.zRot = 0;
						item->pos.xRot = 0;
						Lara.waterStatus = LW_ABOVE_WATER;
						Lara.torsoYrot = 0;
						Lara.torsoXrot = 0;
						Lara.headYrot = 0;
						Lara.headXrot = 0;
					}
					else
					{
						SetAnimation(item, LA_UNDERWATER_RESURFACE);
						item->pos.yPos = waterHeight;
						item->fallspeed = 0;
						item->pos.zRot = 0;
						item->pos.xRot = 0;
						Lara.waterStatus = LW_SURFACE;
						Lara.diveCount = 11;
						Lara.torsoYrot = 0;
						Lara.torsoXrot = 0;
						Lara.headYrot = 0;
						Lara.headXrot = 0;

						UpdateItemRoom(item, -(STEPUP_HEIGHT - 3));
						SoundEffect(SFX_TR4_LARA_BREATH, &item->pos, 2);
					}
				}
			}
			else
			{
				SetAnimation(item, LA_UNDERWATER_RESURFACE);
				item->pos.yPos = waterHeight + 1;
				item->fallspeed = 0;
				item->pos.zRot = 0;
				item->pos.xRot = 0;
				Lara.waterStatus = LW_SURFACE;
				Lara.diveCount = 11;
				Lara.torsoYrot = 0;
				Lara.torsoXrot = 0;
				Lara.headYrot = 0;
				Lara.headXrot = 0;

				UpdateItemRoom(item, 0);
				SoundEffect(SFX_TR4_LARA_BREATH, &item->pos, 2);
			}
			break;

		case LW_SURFACE:
			if (!isWater)
			{
				if (heightFromWater <= WADE_DEPTH)
				{
					SetAnimation(item, LA_FALL_START);
					item->speed = item->fallspeed / 4;
					item->gravityStatus = true;
					Lara.waterStatus = LW_ABOVE_WATER;
				}
				else
				{
					SetAnimation(item, LA_STAND_IDLE);
					item->goalAnimState = LS_WADE_FORWARD; // TODO: Check if really needed? -- Lwmte, 10.11.21
					Lara.waterStatus = LW_WADE;

					AnimateItem(item);
				}

				item->fallspeed = 0;
				item->pos.zRot = 0;
				item->pos.xRot = 0;
				Lara.torsoYrot = 0;
				Lara.torsoXrot = 0;
				Lara.headYrot = 0;
				Lara.headXrot = 0;
			}

			break;

		case LW_WADE:
			Camera.targetElevation = -ANGLE(22.0f);

			if (heightFromWater >= WADE_DEPTH)
			{
				if (heightFromWater > SWIM_DEPTH && !isSwamp)
				{
					SetAnimation(item, LA_ONWATER_IDLE);

					Lara.waterStatus = LW_SURFACE;
					item->pos.yPos += 1 - heightFromWater;
					item->gravityStatus = false;
					item->fallspeed = 0;
					item->pos.zRot = 0;
					item->pos.xRot = 0;
					Lara.diveCount = 0;
					Lara.torsoYrot = 0;
					Lara.torsoXrot = 0;
					Lara.headYrot = 0;
					Lara.headXrot = 0;

					UpdateItemRoom(item, 0);
				}
			}
			else
			{
				Lara.waterStatus = LW_ABOVE_WATER;

				if (item->currentAnimState == LS_WADE_FORWARD)
					item->goalAnimState = LS_RUN_FORWARD;
			}

			break;
		}
	}

	if (item->hitPoints <= 0)
	{
		item->hitPoints = -1;

		if (Lara.deathCount == 0)
			StopSoundTracks();

		Lara.deathCount++;
		if ((item->flags & 0x100))
		{
			Lara.deathCount++;

			return;
		}
	}

	switch (Lara.waterStatus)
	{
	case LW_ABOVE_WATER:
	case LW_WADE:
		if (isSwamp	&& Lara.waterSurfaceDist < -(LARA_HEIGHT + 8)) // TODO: Find best height. @Sezz 2021.11.10
		{
			if (item->hitPoints >= 0)
			{
				Lara.air -= 6;
				if (Lara.air < 0)
				{
					Lara.air = -1;
					item->hitPoints -= 10;
				}
			}
		}
		else if (Lara.air < LARA_AIR_MAX && item->hitPoints >= 0)
		{
			if (Lara.Vehicle == NO_ITEM) // only for the upv !!
			{
				Lara.air += 10;
				if (Lara.air > LARA_AIR_MAX)
					Lara.air = LARA_AIR_MAX;
			}
		}

		LaraAboveWater(item, coll);

		break;

	case LW_UNDERWATER:
		if (item->hitPoints >= 0)
		{
			auto level = g_GameFlow->GetLevel(CurrentLevel);
			if (level->LaraType != LaraType::Divesuit)
				Lara.air--;

			if (Lara.air < 0)
			{
			//	if (LaraDrawType == LARA_TYPE::DIVESUIT && Lara.anxiety < 251)
			//		Lara.anxiety += 4;
				Lara.air = -1;
				item->hitPoints -= 5;
			}
		}

		LaraUnderWater(item, coll);

		break;

	case LW_SURFACE:
		if (item->hitPoints >= 0)
		{
			Lara.air += 10;
			if (Lara.air > LARA_AIR_MAX)
				Lara.air = LARA_AIR_MAX;
		}

		LaraSurface(item, coll);

		break;

	case LW_FLYCHEAT:
		LaraCheat(item, coll);

		break;
	}

	Statistics.Game.Distance += sqrt(
		SQUARE(item->pos.xPos - oldX) +
		SQUARE(item->pos.yPos - oldY) +
		SQUARE(item->pos.zPos - oldZ));
}

void LaraAboveWater(ITEM_INFO* item, COLL_INFO* coll)
{
	coll->Setup.OldPosition.x = item->pos.xPos;
	coll->Setup.OldPosition.y = item->pos.yPos;
	coll->Setup.OldPosition.z = item->pos.zPos;
	coll->Setup.OldAnimState = item->currentAnimState;
	coll->Setup.OldAnimNumber = item->animNumber;
	coll->Setup.OldFrameNumber = item->frameNumber;

	coll->Setup.EnableObjectPush = true;
	coll->Setup.EnableSpaz = true;
	coll->Setup.SlopesAreWalls = false;
	coll->Setup.SlopesArePits = false;
	coll->Setup.DeathFlagIsPit = false;
	coll->Setup.Mode = COLL_PROBE_MODE::QUADRANTS;

	coll->Setup.Radius = LARA_RAD;
	coll->Setup.Height = LARA_HEIGHT;

	if (TrInput & IN_LOOK && Lara.look &&
		Lara.ExtraAnim == NO_ITEM)
	{
		LookLeftRight();
	}
	else
		ResetLook();

	Lara.look = true;

	UpdateItemRoom(item, -LARA_HEIGHT / 2);

	// Process Vehicles
	if (Lara.Vehicle != NO_ITEM)
	{
		switch (g_Level.Items[Lara.Vehicle].objectNumber)
		{
		case ID_QUAD:
			if (QuadBikeControl(item, coll))
				return;
			break;

		case ID_JEEP:
			if (JeepControl())
				return;
			break;

		case ID_MOTORBIKE:
			if (MotorbikeControl())
				return;
			break;

		case ID_KAYAK:
			if (KayakControl())
				return;
			break;

		case ID_SNOWMOBILE:
			if (SkidooControl(item, coll))
				return;
			break;

		case ID_UPV:
			if (SubControl(item, coll))
				return;
			break;

		case ID_MINECART:
			if (MineCartControl())
				return;
			break;

		case ID_BIGGUN:
			if (BigGunControl(item, coll))
				return;
			break;

		default:
			// Boats are processed like normal items in loop
			LaraGun(item);
			return;
		}
	}

	// Handle current Lara status
	lara_control_routines[item->currentAnimState](item, coll);

	if (item->pos.zRot >= -ANGLE(1.0f) && item->pos.zRot <= ANGLE(1.0f))
		item->pos.zRot = 0;
	else if (item->pos.zRot < -ANGLE(1.0f))
		item->pos.zRot += ANGLE(1.0f);
	else
		item->pos.zRot -= ANGLE(1.0f);

	if (Lara.turnRate >= -ANGLE(2.0f) && Lara.turnRate <= ANGLE(2.0f))
		Lara.turnRate = 0;
	else if (Lara.turnRate < -ANGLE(2.0f))
		Lara.turnRate += ANGLE(2.0f);
	else
		Lara.turnRate -= ANGLE(2.0f);
	item->pos.yRot += Lara.turnRate;

	// Animate Lara
	AnimateLara(item);

	if (Lara.ExtraAnim == -1)
	{
		// Check for collision with items
		DoObjectCollision(item, coll);

		// Handle Lara collision
		if (Lara.Vehicle == NO_ITEM)
			lara_collision_routines[item->currentAnimState](item, coll);
	}

	//if (Lara.gunType == WEAPON_CROSSBOW && !LaserSight)
	//	TrInput &= ~IN_ACTION;

	// Handle weapons
	LaraGun(item);

	// Handle breath
	LaraBreath(item);

	// Test for flags & triggers
	ProcessSectorFlags(item);
	TestTriggers(item, false);
	TestVolumes(item);
}

void LaraUnderWater(ITEM_INFO* item, COLL_INFO* coll)
{
	coll->Setup.BadHeightDown = 32512;
	coll->Setup.BadHeightUp = -400;
	coll->Setup.BadCeilingHeight = 400;

	coll->Setup.OldPosition.x = item->pos.xPos;
	coll->Setup.OldPosition.y = item->pos.yPos;
	coll->Setup.OldPosition.z = item->pos.zPos;

	coll->Setup.SlopesAreWalls = false;
	coll->Setup.SlopesArePits = false;
	coll->Setup.DeathFlagIsPit = false;
	coll->Setup.EnableObjectPush = true;
	coll->Setup.EnableSpaz = false;
	coll->Setup.Mode = COLL_PROBE_MODE::QUADRANTS;

	coll->Setup.Radius = LARA_RAD_UNDERWATER;
	coll->Setup.Height = LARA_HEIGHT;

	if (TrInput & IN_LOOK && Lara.look)
		LookLeftRight();
	else
		ResetLook();

	Lara.look = true;

	lara_control_routines[item->currentAnimState](item, coll);

	auto level = g_GameFlow->GetLevel(CurrentLevel);

	if (level->LaraType == LaraType::Divesuit)
	{
		if (Lara.turnRate < -ANGLE(0.5f))
			Lara.turnRate += ANGLE(0.5f);
		else if (Lara.turnRate > ANGLE(0.5f))
			Lara.turnRate -= ANGLE(0.5f);
		else
			Lara.turnRate = 0;
	}
	else if (Lara.turnRate < -ANGLE(2.0f))
		Lara.turnRate += ANGLE(2.0f);
	else if (Lara.turnRate > ANGLE(2.0f))
		Lara.turnRate -= ANGLE(2.0f);
	else
		Lara.turnRate = 0;

	item->pos.yRot += Lara.turnRate;

	if (level->LaraType == LaraType::Divesuit)
		UpdateSubsuitAngles();

	if (item->pos.zRot < -ANGLE(2.0f))
		item->pos.zRot += ANGLE(2.0f);
	else if (item->pos.zRot > ANGLE(2.0f))
		item->pos.zRot -= ANGLE(2.0f);
	else
		item->pos.zRot = 0;

	if (item->pos.xRot < -ANGLE(85.0f))
		item->pos.xRot = -ANGLE(85.0f);
	else if (item->pos.xRot > ANGLE(85.0f))
		item->pos.xRot = ANGLE(85.0f);

	if (level->LaraType == LaraType::Divesuit)
	{
		if (item->pos.zRot > ANGLE(44.0f))
			item->pos.zRot = ANGLE(44.0f);
		else if (item->pos.zRot < -ANGLE(44.0f))
			item->pos.zRot = -ANGLE(44.0f);
	}
	else
	{
		if (item->pos.zRot > ANGLE(22.0f))
			item->pos.zRot = ANGLE(22.0f);
		else if (item->pos.zRot < -ANGLE(22.0f))
			item->pos.zRot = -ANGLE(22.0f);
	}

	if (Lara.currentActive && Lara.waterStatus != LW_FLYCHEAT)
		LaraWaterCurrent(coll);

	AnimateLara(item);

	item->pos.xPos += phd_cos(item->pos.xRot) * item->fallspeed * phd_sin(item->pos.yRot) / 4;
	item->pos.yPos -= item->fallspeed * phd_sin(item->pos.xRot) / 4;
	item->pos.zPos += phd_cos(item->pos.xRot) * item->fallspeed * phd_cos(item->pos.yRot) / 4;

	DoObjectCollision(item, coll);

	if (/*Lara.ExtraAnim == -1 &&*/ Lara.Vehicle == NO_ITEM)
		lara_collision_routines[item->currentAnimState](item, coll);

	UpdateItemRoom(item, 0);

	LaraGun(item);

	ProcessSectorFlags(item);
	TestTriggers(item, false);
	TestVolumes(item);
}

void LaraSurface(ITEM_INFO* item, COLL_INFO* coll)
{
	Camera.targetElevation = -ANGLE(22.0f);

	coll->Setup.BadHeightDown = 32512;
	coll->Setup.BadHeightUp = -128;
	coll->Setup.BadCeilingHeight = 100;

	coll->Setup.OldPosition.x = item->pos.xPos;
	coll->Setup.OldPosition.y = item->pos.yPos;
	coll->Setup.OldPosition.z = item->pos.zPos;

	coll->Setup.SlopesAreWalls = false;
	coll->Setup.SlopesArePits = false;
	coll->Setup.DeathFlagIsPit = false;
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpaz = false;
	coll->Setup.Mode = COLL_PROBE_MODE::FREE_FORWARD;

	coll->Setup.Radius = LARA_RAD;
	coll->Setup.Height = LARA_HEIGHT_SURFACE;

	if (TrInput & IN_LOOK && Lara.look)
		LookLeftRight();
	else
		ResetLook();

	Lara.look = true;

	lara_control_routines[item->currentAnimState](item, coll);

	if (item->pos.zRot >= -ANGLE(2) && item->pos.zRot <= ANGLE(2.0f))
		item->pos.zRot = 0;
	else if (item->pos.zRot < 0)
		item->pos.zRot += ANGLE(2.0f);
	else
		item->pos.zRot -= ANGLE(2.0f);

	if (Lara.currentActive && Lara.waterStatus != LW_FLYCHEAT)
		LaraWaterCurrent(coll);

	AnimateLara(item);

	item->pos.xPos += item->fallspeed * phd_sin(Lara.moveAngle) / 4;
	item->pos.zPos += item->fallspeed * phd_cos(Lara.moveAngle) / 4;

	DoObjectCollision(item, coll);

	if (Lara.Vehicle == NO_ITEM)
		lara_collision_routines[item->currentAnimState](item, coll);

	UpdateItemRoom(item, 100);

	LaraGun(item);

	ProcessSectorFlags(item);
	TestTriggers(item, false);
	TestVolumes(item);
}

void LaraCheat(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraItem->hitPoints = 1000;
	LaraUnderWater(item, coll);
	if (TrInput & IN_WALK && !(TrInput & IN_LOOK))
	{
		if (TestLaraWater(item) || (Lara.waterSurfaceDist > 0 && Lara.waterSurfaceDist != NO_HEIGHT))
		{
			Lara.waterStatus = LW_UNDERWATER;
			SetAnimation(item, LA_UNDERWATER_IDLE);
			Lara.torsoYrot = 0;
			Lara.torsoXrot = 0;
			Lara.headYrot = 0;
			Lara.headXrot = 0;
		}
		else
		{
			Lara.waterStatus = LW_ABOVE_WATER;
			SetAnimation(item, LA_STAND_SOLID);
			item->pos.zRot = 0;
			item->pos.xRot = 0;
			Lara.torsoYrot = 0;
			Lara.torsoXrot = 0;
			Lara.headYrot = 0;
			Lara.headXrot = 0;
		}
		Lara.gunStatus = LG_NO_ARMS;
		LaraInitialiseMeshes();
		LaraItem->hitPoints = 1000;
	}
}

void AnimateLara(ITEM_INFO* item)
{
	item->frameNumber++;

	ANIM_STRUCT* anim = &g_Level.Anims[item->animNumber];
	if (anim->numberChanges > 0 && GetChange(item, anim))
	{
		anim = &g_Level.Anims[item->animNumber];
		item->currentAnimState = anim->currentAnimState;
	}

	if (item->frameNumber > anim->frameEnd)
	{
		if (anim->numberCommands > 0)
		{
			short* cmd = &g_Level.Commands[anim->commandIndex];
			for (int i = anim->numberCommands; i > 0; i--)
			{
				switch (*(cmd++))
				{
				case COMMAND_MOVE_ORIGIN:
					TranslateItem(item, cmd[0], cmd[1], cmd[2]);
					UpdateItemRoom(item, -LARA_HEIGHT / 2);
					cmd += 3;
					break;

				case COMMAND_JUMP_VELOCITY:
					item->fallspeed = *(cmd++);
					item->speed = *(cmd++);
					item->gravityStatus = true;
					if (Lara.calcFallSpeed)
					{
						item->fallspeed = Lara.calcFallSpeed;
						Lara.calcFallSpeed = 0;
					}
					break;

				case COMMAND_ATTACK_READY:
					if (Lara.gunStatus != LG_SPECIAL)
						Lara.gunStatus = LG_NO_ARMS;
					break;

				case COMMAND_SOUND_FX:
				case COMMAND_EFFECT:
					cmd += 2;
					break;

				default:
					break;
				}
			}
		}

		item->animNumber = anim->jumpAnimNum;
		item->frameNumber = anim->jumpFrameNum;

		anim = &g_Level.Anims[item->animNumber];
		item->currentAnimState = anim->currentAnimState;
	}

	if (anim->numberCommands > 0)
	{
		short* cmd = &g_Level.Commands[anim->commandIndex];
		int flags;
		int effectID = 0;

		for (int i = anim->numberCommands; i > 0; i--)
		{
			switch (*(cmd++))
			{
			case COMMAND_MOVE_ORIGIN:
				cmd += 3;
				break;

			case COMMAND_JUMP_VELOCITY:
				cmd += 2;
				break;

			case COMMAND_SOUND_FX:
				if (item->frameNumber != *cmd)
				{
					cmd += 2;
					break;
				}

				flags = cmd[1] & 0xC000;
				if ( flags == (int)SOUND_PLAYCONDITION::LandAndWater ||
					(flags == (int)SOUND_PLAYCONDITION::Land && (Lara.waterSurfaceDist >= 0 || Lara.waterSurfaceDist == NO_HEIGHT)) ||
					(flags == (int)SOUND_PLAYCONDITION::Water && Lara.waterSurfaceDist < 0 && Lara.waterSurfaceDist != NO_HEIGHT && !TestLaraSwamp(item)))
				{
					SoundEffect(cmd[1] & 0x3FFF, &item->pos, 2);
				}

				cmd += 2;
				break;

			case COMMAND_EFFECT:
				if (item->frameNumber != *cmd)
				{
					cmd += 2;
					break;
				}

				effectID = cmd[1] & 0x3FFF;
				DoFlipEffect(effectID, item);

				cmd += 2;
				break;

			default:
				break;

			}
		}
	}

	int lateral = anim->Xvelocity;
	if (anim->Xacceleration)
		lateral += anim->Xacceleration * (item->frameNumber - anim->frameBase);
	lateral >>= 16;

	if (item->gravityStatus)
	{
		if (TestLaraSwamp(item))
		{
			item->speed -= item->speed >> 3;
			if (abs(item->speed) < 8)
			{
				item->speed = 0;
				item->gravityStatus = false;
			}
			if (item->fallspeed > 128)
				item->fallspeed /= 2;
			item->fallspeed -= item->fallspeed / 4;
			if (item->fallspeed < 4)
				item->fallspeed = 4;
			item->pos.yPos += item->fallspeed;
		}
		else
		{
			int velocity = (anim->velocity + anim->acceleration * (item->frameNumber - anim->frameBase - 1));
			item->speed -= velocity >> 16;
			item->speed += (velocity + anim->acceleration) >> 16;
			item->fallspeed += (item->fallspeed >= 128 ? 1 : GRAVITY);
			item->pos.yPos += item->fallspeed;
		}
	}
	else
	{
		int velocity;

		if (Lara.waterStatus == LW_WADE && TestLaraSwamp(item))
		{
			velocity = (anim->velocity >> 1);
			if (anim->acceleration)
				velocity += (anim->acceleration * (item->frameNumber - anim->frameBase)) >> 2;
		}
		else
		{
			velocity = anim->velocity;
			if (anim->acceleration)
				velocity += anim->acceleration * (item->frameNumber - anim->frameBase);
		}

		item->speed = velocity >> 16;
	}

	if (Lara.ropePtr != -1)
		DelAlignLaraToRope(item);

	if (!Lara.isMoving)
		MoveItem(item, Lara.moveAngle, item->speed, lateral);

	// Update matrices
	g_Renderer.updateLaraAnimations(true);
}
