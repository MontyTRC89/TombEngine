#include "framework.h"
#include "Lara.h"
#include "lara_basic.h"
#include "lara_tests.h"
#include "lara_monkey.h"
#include "lara_crawl.h"
#include "lara_objects.h"
#include "lara_hang.h"
#include "lara_slide.h"
#ifdef NEW_INV
#include "newinv2.h"
#else
#include "inventory.h"
#endif
#include "lara_fire.h"
#include "lara_surface.h"
#include "lara_swim.h"
#include "lara_one_gun.h"
#include "lara_two_guns.h"
#include "lara_cheat.h"
#include "lara_climb.h"
#include "lara_initialise.h"

#include "motorbike.h"
#include "biggun.h"
#include "quad.h"
#include "snowmobile.h"
#include "jeep.h"
#include "boat.h"
#include "upv.h"
#include "kayak.h"
#include "minecart.h"
//#include "rubberboat.h"

#include "GameFlowScript.h"
#include "health.h"
#include "flipeffect.h"
#include "Sound\sound.h"
#include "savegame.h"
#include "rope.h"
#include "rubberboat.h"
#include <Game\misc.h>
#include <control\volume.h>
#include "Renderer11.h"
#include "camera.h"
using std::function;
using TEN::Renderer::g_Renderer;
using namespace TEN::Control::Volumes;

#ifndef NEW_INV
extern Inventory g_Inventory;
#endif

short Elevation = 57346;
extern short FXType;
LaraInfo Lara;
ITEM_INFO* LaraItem;
COLL_INFO lara_coll = {};
byte LaraNodeUnderwater[NUM_LARA_MESHES];

function<LaraRoutineFunction> lara_control_routines[NUM_LARA_STATES + 1] = {
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
	lara_as_hang2,
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
	lara_as_null,//99
	lara_as_null,//100
	lara_as_null,//101
	lara_as_poleleft,//102
	lara_as_poleright,//103
	lara_as_pulley,//104
	lara_as_duckl,//105
	lara_as_duckr,//106
	lara_as_extcornerl,//107
	lara_as_extcornerr,//108
	lara_as_intcornerl,//109
	lara_as_intcornerr,//110
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
	lara_as_null,//ara_as_hang_feet,//139
	lara_as_hang_feet_shimmyr,//140
	lara_as_hang_feet_shimmyl,//141
	lara_as_hang_feet_inRcorner,//142
	lara_as_hang_feet_inLcorner,//143
	lara_as_hang_feet_outRcorner,//144
	lara_as_hang_feet_outLcorner,//145
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
	lara_col_hang2,
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
	lara_col_polestat,
	lara_col_poleup,
	lara_col_poledown,
	lara_void_func,
	lara_void_func,
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
	lara_col_hang_feet,
	lara_col_hang_feet_shimmyr,
	lara_col_hang_feet_shimmyl,
	lara_default_col,
	lara_default_col,
	lara_default_col,
	lara_default_col,
	lara_void_func,
	lara_void_func,
	lara_void_func,
	lara_void_func,
	lara_default_col,
	lara_default_col
};

void LaraControl(short itemNumber)
{
	ITEM_INFO* item = LaraItem;

	LaraCheatyBits();

	if (Lara.isMoving)
	{
		if (Lara.moveCount > 90)
		{
			Lara.isMoving = false;
			Lara.gunStatus = LG_NO_ARMS;
		}
		++Lara.moveCount;
	}

	if (!DisableLaraControl)
		Lara.locationPad = 128;

	int oldX = LaraItem->pos.xPos;
	int oldY = LaraItem->pos.yPos;
	int oldZ = LaraItem->pos.zPos;

	if (Lara.gunStatus == LG_HANDS_BUSY &&
		LaraItem->currentAnimState == LS_STOP &&
		LaraItem->goalAnimState == LS_STOP &&
		LaraItem->animNumber == LA_STAND_IDLE &&
		!LaraItem->gravityStatus)
	{
		Lara.gunStatus = LG_NO_ARMS;
	}

	if (item->currentAnimState != LS_SPRINT && DashTimer < 120)
		DashTimer++;

	Lara.isDucked = false;

	bool isWater = g_Level.Rooms[item->roomNumber].flags & (ENV_FLAG_WATER | ENV_FLAG_SWAMP);

	int wd = GetWaterDepth(item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber);
	int wh = GetWaterHeight(item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber);

	int hfw;
	if (wh != NO_HEIGHT)
		hfw = item->pos.yPos - wh;
	else
		hfw = NO_HEIGHT;
	Lara.waterSurfaceDist = -hfw;

	if (Lara.Vehicle == NO_ITEM)
		WadeSplash(item, wh, wd);

	short roomNumber;

	if (Lara.Vehicle == NO_ITEM && Lara.ExtraAnim == -1)
	{
		switch (Lara.waterStatus)
		{
		case LW_ABOVE_WATER:
			if (hfw != NO_HEIGHT && hfw >= STEP_SIZE && Lara.Vehicle == NO_ITEM)
			{
				if (wd <= 474)
				{
					if (hfw > 256)
					{
						Lara.waterStatus = LW_WADE;
						if (!(item->gravityStatus))
						{
							item->goalAnimState = LS_STOP;
						}
						else if (isWater & ENV_FLAG_SWAMP)
						{
							if (item->currentAnimState == LS_SWANDIVE_START 
								|| item->currentAnimState == LS_SWANDIVE_END)			// Is Lara swan-diving?
								item->pos.yPos = wh + 1000;

							item->goalAnimState = LS_WADE_FORWARD;
							item->currentAnimState = LS_WADE_FORWARD;
							item->animNumber = LA_WADE;
							item->frameNumber = GF(LA_WADE, 0);
						}
					}
				}
				else if (!(isWater & ENV_FLAG_SWAMP))
				{
					Lara.air = 1800;
					Lara.waterStatus = LW_UNDERWATER;
					item->gravityStatus = false;
					item->pos.yPos += 100;

					UpdateLaraRoom(LaraItem, 0);
					StopSoundEffect(SFX_TR4_LARA_FALL);

					if (item->currentAnimState == LS_SWANDIVE_START)
					{
						item->pos.xRot = -ANGLE(45);
						item->goalAnimState = LS_DIVE;
						AnimateLara(item);
						item->fallspeed *= 2;
					}
					else if (item->currentAnimState == LS_SWANDIVE_END)
					{
						item->pos.xRot = -ANGLE(85);
						item->goalAnimState = LS_DIVE;
						AnimateLara(item);
						item->fallspeed *= 2;
					}
					else
					{
						item->pos.xRot = -ANGLE(45);
						item->animNumber = LA_FREEFALL_DIVE;
						item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
						item->currentAnimState = LS_DIVE;
						item->goalAnimState = LS_UNDERWATER_FORWARD;
						item->fallspeed = 3 * item->fallspeed / 2;
					}

					Lara.torsoYrot = 0;
					Lara.torsoXrot = 0;
					Lara.headYrot = 0;
					Lara.headXrot = 0;

					Splash(LaraItem);
				}

				Camera.targetElevation = -ANGLE(22);
				if (hfw > 256)
				{
					if (hfw > 730)
					{
						Lara.waterStatus = LW_SURFACE;
						item->pos.yPos += 1 - hfw;

						switch (item->currentAnimState)
						{
						case LS_WALK_BACK:
							item->animNumber = LA_ONWATER_IDLE_TO_SWIM_BACK;
							item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
							item->goalAnimState = LS_ONWATER_BACK;
							item->currentAnimState = LS_ONWATER_BACK;
							break;

						case LS_STEP_RIGHT:
							item->animNumber = LA_ONWATER_SWIM_RIGHT;
							item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
							item->goalAnimState = LS_ONWATER_RIGHT;
							item->currentAnimState = LS_ONWATER_RIGHT;
							break;

						case LS_STEP_LEFT:
							item->animNumber = LA_ONWATER_SWIM_LEFT;
							item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
							item->goalAnimState = LS_ONWATER_LEFT;
							item->currentAnimState = LS_ONWATER_LEFT;
							break;

						default:
							item->animNumber = LA_ONWATER_SWIM;
							item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
							item->goalAnimState = LS_ONWATER_FORWARD;
							item->currentAnimState = LS_ONWATER_FORWARD;
							break;
						}

						item->gravityStatus = false;
						item->fallspeed = 0;
						Lara.diveCount = 0;
						LaraItem->pos.zRot = 0;
						LaraItem->pos.xRot = 0;
						Lara.torsoYrot = 0;
						Lara.torsoXrot = 0;
						Lara.headYrot = 0;
						Lara.headXrot = 0;

						UpdateLaraRoom(item, 0);
					}
				}
				else
				{
					LaraItem->roomNumber;
					Lara.waterStatus = LW_ABOVE_WATER;
					if (item->currentAnimState == LS_WADE_FORWARD)
						item->goalAnimState = LS_RUN_FORWARD;
				}
			}

			break;

		case LW_UNDERWATER:
			roomNumber = item->roomNumber;
			GetFloor(item->pos.xPos, item->pos.yPos - 256, item->pos.zPos, &roomNumber);

			if (wd == NO_HEIGHT
				|| abs(hfw) >= 256
				|| g_Level.Rooms[roomNumber].flags & ENV_FLAG_WATER
				|| item->animNumber == LA_UNDERWATER_RESURFACE
				|| item->animNumber == LA_ONWATER_DIVE)
			{
				if (!isWater)
				{
					if (wd == NO_HEIGHT || abs(hfw) >= 256)
					{
						Lara.waterStatus = LW_ABOVE_WATER;
						item->animNumber = LA_FALL_START;
						item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
						item->goalAnimState = LS_JUMP_FORWARD;
						item->currentAnimState = LS_JUMP_FORWARD;
						item->speed = item->fallspeed / 4;
						item->gravityStatus = true;

						item->fallspeed = 0;
						LaraItem->pos.zRot = 0;
						LaraItem->pos.xRot = 0;
						Lara.torsoYrot = 0;
						Lara.torsoXrot = 0;
						Lara.headYrot = 0;
						Lara.headXrot = 0;
					}
					else
					{
						Lara.waterStatus = LW_SURFACE;
						item->pos.yPos = wh;
						item->animNumber = LA_UNDERWATER_RESURFACE;
						item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
						item->goalAnimState = LS_ONWATER_STOP;
						item->currentAnimState = LS_ONWATER_STOP;
						item->fallspeed = 0;
						Lara.diveCount = 11;
						LaraItem->pos.zRot = 0;
						LaraItem->pos.xRot = 0;
						Lara.torsoYrot = 0;
						Lara.torsoXrot = 0;
						Lara.headYrot = 0;
						Lara.headXrot = 0;

						UpdateLaraRoom(item, -381);
						SoundEffect(SFX_TR4_LARA_BREATH, &LaraItem->pos, 2);
					}
				}
			}
			else
			{
				Lara.waterStatus = LW_SURFACE;
				item->pos.yPos = wh + 1;
				item->animNumber = LA_UNDERWATER_RESURFACE;
				item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
				item->goalAnimState = LS_ONWATER_STOP;
				item->currentAnimState = LS_ONWATER_STOP;
				item->fallspeed = 0;
				Lara.diveCount = 11;
				LaraItem->pos.zRot = 0;
				LaraItem->pos.xRot = 0;
				Lara.torsoYrot = 0;
				Lara.torsoXrot = 0;
				Lara.headYrot = 0;
				Lara.headXrot = 0;

				UpdateLaraRoom(item, 0);
				SoundEffect(SFX_TR4_LARA_BREATH, &LaraItem->pos, 2);
			}
			break;

		case LW_SURFACE:
			if (!isWater)
			{
				if (hfw <= 256)
				{
					Lara.waterStatus = LW_ABOVE_WATER;
					item->animNumber = LA_FALL_START;
					item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
					item->goalAnimState = LS_JUMP_FORWARD;
					item->currentAnimState = LS_JUMP_FORWARD;
					item->speed = item->fallspeed / 4;
					item->gravityStatus = true;
				}
				else
				{
					Lara.waterStatus = LW_WADE;
					item->animNumber = LA_STAND_IDLE;
					item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
					item->goalAnimState = LS_WADE_FORWARD;
					item->currentAnimState = LS_STOP;

					AnimateItem(item);
				}

				item->fallspeed = 0;
				LaraItem->pos.zRot = 0;
				LaraItem->pos.xRot = 0;
				Lara.torsoYrot = 0;
				Lara.torsoXrot = 0;
				Lara.headYrot = 0;
				Lara.headXrot = 0;
			}
			break;

		case LW_WADE:
			Camera.targetElevation = -ANGLE(22);
			if (hfw > 256)
			{
				if (hfw > 730 && !(isWater & ENV_FLAG_SWAMP))
				{
					Lara.waterStatus = LW_SURFACE;
					item->pos.yPos += 1 - hfw;

					switch (item->currentAnimState)
					{
					case LS_WALK_BACK:
						item->animNumber = LA_ONWATER_IDLE_TO_SWIM_BACK;
						item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
						item->goalAnimState = LS_ONWATER_BACK;
						item->currentAnimState = LS_ONWATER_BACK;
						break;

					case LS_STEP_RIGHT:
						item->animNumber = LA_ONWATER_SWIM_RIGHT;
						item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
						item->goalAnimState = LS_ONWATER_RIGHT;
						item->currentAnimState = LS_ONWATER_RIGHT;
						break;

					case LS_STEP_LEFT:
						item->animNumber = LA_ONWATER_SWIM_LEFT;
						item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
						item->goalAnimState = LS_ONWATER_LEFT;
						item->currentAnimState = LS_ONWATER_LEFT;
						break;

					default:
						item->animNumber = LA_ONWATER_SWIM;
						item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
						item->goalAnimState = LS_ONWATER_FORWARD;
						item->currentAnimState = LS_ONWATER_FORWARD;
						break;
					}

					item->gravityStatus = false;
					item->fallspeed = 0;
					Lara.diveCount = 0;
					LaraItem->pos.zRot = 0;
					LaraItem->pos.xRot = 0;
					Lara.torsoYrot = 0;
					Lara.torsoXrot = 0;
					Lara.headYrot = 0;
					Lara.headXrot = 0;

					UpdateLaraRoom(item, 0);
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
			S_CDStop();

		Lara.deathCount++;
		if ((LaraItem->flags & 0x100))
		{
			Lara.deathCount++;
			return;
		}
	}

	switch (Lara.waterStatus)
	{
	case LW_ABOVE_WATER:
	case LW_WADE:
		if ((g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_SWAMP) 
			&& Lara.waterSurfaceDist < -775)
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
		else if (Lara.air < 1800 && item->hitPoints >= 0)
		{
			if (Lara.Vehicle == NO_ITEM) // only for the upv !!
			{
				Lara.air += 10;
				if (Lara.air > 1800)
					Lara.air = 1800;
			}
		}
		LaraAboveWater(item, &lara_coll);
		break;

	case LW_UNDERWATER:
		if (item->hitPoints >= 0)
		{
			if (LaraDrawType == LARA_TYPE::DIVESUIT)
			{
				/* Hardcoded code */
			}
			else
			{
				Lara.air--;
			}
			if (Lara.air < 0)
			{
			//	if (LaraDrawType == LARA_TYPE::DIVESUIT && Lara.anxiety < 251)
			//		Lara.anxiety += 4;
				Lara.air = -1;
				item->hitPoints -= 5;
			}
		}
		LaraUnderWater(item, &lara_coll);
		break;

	case LW_SURFACE:
		if (item->hitPoints >= 0)
		{
			Lara.air += 10;
			if (Lara.air > 1800)
				Lara.air = 1800;
		}
		LaraSurface(item, &lara_coll);
		break;

	case LW_FLYCHEAT:
		LaraCheat(item, &lara_coll);
		break;
	}

	Savegame.Game.Distance += sqrt(
		SQUARE(item->pos.xPos - oldX) +
		SQUARE(item->pos.yPos - oldY) +
		SQUARE(item->pos.zPos - oldZ));
}

void LaraAboveWater(ITEM_INFO* item, COLL_INFO* coll) //hmmmm
{
	coll->old.x = item->pos.xPos;
	coll->old.y = item->pos.yPos;
	coll->old.z = item->pos.zPos;
	coll->oldAnimState = item->currentAnimState;
	coll->enableBaddiePush = true;
	coll->enableSpaz = true;
	coll->slopesAreWalls = false;
	coll->slopesArePits = false;
	coll->lavaIsPit = false;
	coll->oldAnimNumber = item->animNumber;
	coll->oldFrameNumber = item->frameNumber;
	coll->radius = LARA_RAD;

	if ((TrInput & IN_LOOK) && Lara.ExtraAnim == NO_ITEM && Lara.look)
		LookLeftRight();
	else
		ResetLook();

	Lara.look = true;

	// Process Vehicles
	if (Lara.Vehicle != NO_ITEM)
	{
		switch (g_Level.Items[Lara.Vehicle].objectNumber)
		{
		case ID_QUAD:
			if (QuadBikeControl())
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
			if (SkidooControl())
				return;
			break;

		case ID_UPV:
			if (SubControl())
				return;
			break;

		case ID_MINECART:
			if (MineCartControl())
				return;
			break;

		case ID_BIGGUN:
			if (BigGunControl(coll))
				return;
			break;

		default:
			// Boats are processed like normal items in loop
			LaraGun();
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

	UpdateLaraRoom(item, -LARA_HEIGHT / 2);

	//if (Lara.gunType == WEAPON_CROSSBOW && !LaserSight)
	//	TrInput &= ~IN_ACTION;

	// Handle weapons
	LaraGun();

	// Test for flags & triggers
	ProcessSectorFlags(item);
	TestTriggers(item, false, NULL);
	TestVolumes(item);
}

void LaraUnderWater(ITEM_INFO* item, COLL_INFO* coll)
{
	coll->badPos = 32512;
	coll->badNeg = -400;
	coll->badCeiling = 400;

	coll->old.x = item->pos.xPos;
	coll->old.y = item->pos.yPos;
	coll->old.z = item->pos.zPos;

	coll->slopesAreWalls = false;
	coll->slopesArePits = false;
	coll->lavaIsPit = false;

	coll->enableBaddiePush = true;
	coll->enableSpaz = false;

	coll->radius = 300;

	if (TrInput & IN_LOOK && Lara.look)
		LookLeftRight();
	else
		ResetLook();

	Lara.look = true;

	lara_control_routines[item->currentAnimState](item, coll);

	if (LaraDrawType == LARA_TYPE::DIVESUIT)
	{
		if (Lara.turnRate < -ANGLE(0.5))
		{
			Lara.turnRate += ANGLE(0.5);
		}
		else if (Lara.turnRate > ANGLE(0.5))
		{
			Lara.turnRate -= ANGLE(0.5);
		}
		else
		{
			Lara.turnRate = 0;
		}
	}
	else if (Lara.turnRate < -ANGLE(2))
	{
		Lara.turnRate += ANGLE(2);
	}
	else if (Lara.turnRate > ANGLE(2))
	{
		Lara.turnRate -= ANGLE(2);
	}
	else
	{
		Lara.turnRate = 0;
	}

	item->pos.yRot += Lara.turnRate;

	if (LaraDrawType == LARA_TYPE::DIVESUIT)
		UpdateSubsuitAngles();

	if (item->pos.zRot < -ANGLE(2))
		item->pos.zRot += ANGLE(2);
	else if (item->pos.zRot > ANGLE(2))
		item->pos.zRot -= ANGLE(2);
	else
		item->pos.zRot = 0;

	if (item->pos.xRot < -ANGLE(85))
		item->pos.xRot = -ANGLE(85);
	else if (item->pos.xRot > ANGLE(85))
		item->pos.xRot = ANGLE(85);

	if (LaraDrawType == LARA_TYPE::DIVESUIT)
	{
		if (item->pos.zRot > ANGLE(44))
			item->pos.zRot = ANGLE(44);
		else if (item->pos.zRot < -ANGLE(44))
			item->pos.zRot = -ANGLE(44);
	}
	else
	{
		if (item->pos.zRot > ANGLE(22))
			item->pos.zRot = ANGLE(22);
		else if (item->pos.zRot < -ANGLE(22))
			item->pos.zRot = -ANGLE(22);
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

	UpdateLaraRoom(item, 0);

	LaraGun();

	ProcessSectorFlags(item);
	TestTriggers(item, false, NULL);
	TestVolumes(item);
}

void LaraSurface(ITEM_INFO* item, COLL_INFO* coll)
{
	Camera.targetElevation = -ANGLE(22);

	coll->badPos = 32512;
	coll->badNeg = -128;
	coll->badCeiling = 100;

	coll->old.x = item->pos.xPos;
	coll->old.y = item->pos.yPos;
	coll->old.z = item->pos.zPos;

	coll->slopesAreWalls = false;
	coll->slopesArePits = false;
	coll->lavaIsPit = false;
	coll->enableBaddiePush = false;
	coll->enableSpaz = false;

	coll->radius = 100;

	if (TrInput & IN_LOOK && Lara.look)
		LookLeftRight();
	else
		ResetLook();

	Lara.look = true;

	lara_control_routines[item->currentAnimState](item, coll);

	if (item->pos.zRot >= -ANGLE(2) && item->pos.zRot <= ANGLE(2))
		item->pos.zRot = 0;
	else if (item->pos.zRot < 0)
		item->pos.zRot += ANGLE(2);
	else
		item->pos.zRot -= ANGLE(2);

	if (Lara.currentActive && Lara.waterStatus != LW_FLYCHEAT)
		LaraWaterCurrent(coll);

	AnimateLara(item);

	item->pos.xPos += item->fallspeed * phd_sin(Lara.moveAngle) / 4;
	item->pos.zPos += item->fallspeed * phd_cos(Lara.moveAngle) / 4;

	DoObjectCollision(item, coll);

	if (Lara.Vehicle == NO_ITEM)
		lara_collision_routines[item->currentAnimState](item, coll);

	UpdateLaraRoom(item, 100);

	LaraGun();

	ProcessSectorFlags(item);
	TestTriggers(item, false, NULL);
	TestVolumes(item);
}

void LaraCheat(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraItem->hitPoints = 1000;
	LaraUnderWater(item, coll);
	if (TrInput & IN_WALK && !(TrInput & IN_LOOK))
	{
		if (g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_WATER || 
			(Lara.waterSurfaceDist > 0 && 
			Lara.waterSurfaceDist != NO_HEIGHT))
		{
			Lara.waterStatus = LW_UNDERWATER;
			item->animNumber = LA_UNDERWATER_IDLE;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			item->currentAnimState = LS_UNDERWATER_STOP;
			item->goalAnimState = LS_UNDERWATER_STOP;
			Lara.torsoYrot = 0;
			Lara.torsoXrot = 0;
			Lara.headYrot = 0;
			Lara.headXrot = 0;
		}
		else
		{
			Lara.waterStatus = LW_ABOVE_WATER;
			item->animNumber = LA_STAND_SOLID;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
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
					UpdateLaraRoom(item, -LARA_HEIGHT / 2);
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
				if (flags == SFX_LANDANDWATER ||
					(flags == SFX_LANDONLY && (Lara.waterSurfaceDist >= 0 || Lara.waterSurfaceDist == NO_HEIGHT)) ||
					(flags == SFX_WATERONLY && Lara.waterSurfaceDist < 0 && Lara.waterSurfaceDist != NO_HEIGHT && !(g_Level.Rooms[LaraItem->roomNumber].flags & ENV_FLAG_SWAMP)))
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

				FXType = cmd[1] & 0xC000;
				effectID = cmd[1] & 0x3FFF;
				effect_routines[effectID](item);

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

	if (item->gravityStatus)             // If gravity ON (Do Up/Down movement)
	{
		if (g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_SWAMP)
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
	else                                 // if on the Ground...
	{
		int velocity;

		if (Lara.waterStatus == LW_WADE && g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_SWAMP)
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

	if (!Lara.isMoving) // TokyoSU: i dont know why but it's wreid, in TR3 only the 2 first line there is used and worked fine !
	{
		item->pos.xPos += item->speed * phd_sin(Lara.moveAngle);
		item->pos.zPos += item->speed * phd_cos(Lara.moveAngle);

		item->pos.xPos += lateral * phd_sin(Lara.moveAngle + ANGLE(90));
		item->pos.zPos += lateral * phd_cos(Lara.moveAngle + ANGLE(90));
	}

	// Update matrices
	g_Renderer.updateLaraAnimations(true);
}
