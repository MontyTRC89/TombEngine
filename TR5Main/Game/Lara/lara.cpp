#include "framework.h"
#include "Lara.h"
#include "lara_tests.h"
#include "lara_collide.h"
#include "lara_monkey.h"
#include "lara_crawl.h"
#include "lara_objects.h"
#include "lara_hang.h"
#include "lara_slide.h"
#include "control.h"
#include "items.h"
#include "inventory.h"
#include "lara_fire.h"
#include "misc.h"
#include "draw.h"
#include "sphere.h"
#include "camera.h"
#include "lara_surface.h"
#include "lara_swim.h"
#include "lara_one_gun.h"
#include "lara_two_guns.h"
#include "lara_flare.h"
#include "laramisc.h"
#include "lara_climb.h"
#include "rope.h"
#include "health.h"
#include "level.h"
#include "input.h"
#include "sound.h"
#include "setup.h"

#include "motorbike.h"
#include "cannon.h"
#include "quad.h"
#include "snowmobile.h"
#include "jeep.h"
#include "boat.h"
#include "upv.h"
#include "kayak.h"
#include "minecart.h"
using std::function;

extern Inventory g_Inventory;

short elevation = 57346;
bool doJump = false;
LaraInfo Lara;
ITEM_INFO* LaraItem;
byte LaraNodeUnderwater[NUM_LARA_MESHES];

function<LaraRoutineFunction> lara_control_routines[NUM_LARA_STATES + 1] = {
	lara_as_walk,
	lara_as_run,
	lara_as_stop,
	lara_as_forwardjump,
	lara_void_func,
	lara_as_fastback,
	lara_as_turn_r,
	lara_as_turn_l,
	lara_as_death,
	lara_as_fastfall,
	lara_as_hang,
	lara_as_reach,
	lara_as_splat,
	lara_as_tread,
	lara_void_func,
	lara_as_compress,
	lara_as_back,
	lara_as_swim,
	lara_as_glide,
	lara_as_null,
	lara_as_fastturn,
	lara_as_stepright,
	lara_as_stepleft,
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
	lara_as_uwdeath,
	lara_void_func,
	lara_as_special,
	lara_as_surfback,
	lara_as_surfleft,
	lara_as_surfright,
	lara_void_func,
	lara_void_func,
	lara_as_swandive,
	lara_as_fastdive,
	lara_as_gymnast,
	lara_as_waterout,
	lara_as_climbstnc,
	lara_as_climbing,
	lara_as_climbleft,
	lara_as_climbend,
	lara_as_climbright,
	lara_as_climbdown,
	lara_void_func,
	lara_void_func,
	lara_void_func,
	lara_as_wade,
	lara_as_waterroll,
	lara_as_pickupflare,
	lara_void_func,
	lara_void_func,
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
	lara_as_null,//125
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
	lara_as_hang_feet,//139
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
	coll->trigger = NULL;

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

			//case ID_SPEEDBOAT:
			//	if (BoatControl())
			//		return;
			//	break;

			//case ID_RUBBERBOAT:
			//	if (RubberBoatControl())
			//		return;
			//	break;

			//case ID_UPV:
			//	if (SubControl())
			//		return;
			//	break;

			//case ID_MINECART:
			//	if (MineCartControl())
			//		return;
			//	break;

		default:
			break;
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

	if (Lara.ExtraAnim == NO_ITEM)
	{
		// Check for collision with items
		LaraBaddieCollision(item, coll);

		// Handle Lara collision
		if (Lara.Vehicle == NO_ITEM)
			lara_collision_routines[item->currentAnimState](item, coll);
	}

	UpdateLaraRoom(item, -LARA_HITE / 2);

	//if (Lara.gunType == WEAPON_CROSSBOW && !LaserSight)
	//	TrInput &= ~IN_ACTION;

	// Handle weapons
	LaraGun();

	// Test if there's a trigger
	TestTriggers(coll->trigger, FALSE, 0);
}

int UseSpecialItem(ITEM_INFO* item) // to pickup.cpp?
{
	
	short selectedObject = g_Inventory.GetSelectedObject();

	if (item->animNumber != LA_STAND_IDLE || Lara.gunStatus || selectedObject == NO_ITEM)
		return false;

	if (selectedObject >= ID_WATERSKIN1_EMPTY && selectedObject <= ID_WATERSKIN2_5)
	{
		item->itemFlags[2] = 25;

		if (selectedObject != ID_WATERSKIN1_3 && selectedObject != ID_WATERSKIN2_5)
		{
			if (selectedObject >= ID_WATERSKIN2_EMPTY)
				Lara.Waterskin2.Quantity = 5;
			else
				Lara.Waterskin1.Quantity = 3;

			item->animNumber = LA_WATERSKIN_FILL;
		}
		else
		{
			if (selectedObject >= ID_WATERSKIN2_EMPTY)
			{
				item->itemFlags[3] = Lara.Waterskin2.Quantity;
				Lara.Waterskin2.Quantity = 1;
			}
			else
			{
				item->itemFlags[3] = Lara.Waterskin1.Quantity;
				Lara.Waterskin1.Quantity = 1;
			}

			item->animNumber = LA_WATERSKIN_POUR_LOW;
		}
	}
	else if (selectedObject == ID_CLOCKWORK_BEETLE)
	{
		item->animNumber = LA_MECHANICAL_BEETLE_USE;
		//UseClockworkBeetle(1);
	}
	else
	{
		return false;
	}

	item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
	item->goalAnimState = LS_MISC_CONTROL;
	item->currentAnimState = LS_MISC_CONTROL;

	Lara.gunStatus = LG_HANDS_BUSY;
	g_Inventory.SetSelectedObject(NO_ITEM);

	return true;
}

void lara_as_stop(ITEM_INFO* item, COLL_INFO* coll)
{
	short fheight = NO_HEIGHT;
	short rheight = NO_HEIGHT;

	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_DEATH;
		return;
	}

	if (item->animNumber != LA_SPRINT_TO_STAND_RIGHT && item->animNumber != LA_SPRINT_TO_STAND_LEFT)
		StopSoundEffect(SFX_LARA_SLIPPING);

	// Handles waterskin and clockwork beetle
	if (UseSpecialItem(item))
		return;

	if (TrInput & IN_ROLL && Lara.waterStatus != LW_WADE)
	{
		item->animNumber = LA_ROLL_180_START;
		item->frameNumber = g_Level.Anims[item->animNumber].frameBase + 2;
		item->currentAnimState = LS_ROLL_FORWARD;
		item->goalAnimState = LS_STOP;
		return;
	}

	if (TrInput & IN_DUCK
	&&  Lara.waterStatus != LW_WADE
	&&  item->currentAnimState == LS_STOP
	&& (Lara.gunStatus == LG_NO_ARMS
	||  Lara.gunType == WEAPON_NONE
	||	Lara.gunType == WEAPON_PISTOLS
	||	Lara.gunType == WEAPON_REVOLVER
	||	Lara.gunType == WEAPON_UZI
	||	Lara.gunType == WEAPON_FLARE))
	{
		item->goalAnimState = LS_CROUCH_IDLE;
		return;
	}

	item->goalAnimState = LS_STOP;

	if (TrInput & IN_LOOK)
		LookUpDown();

	if (TrInput & IN_FORWARD)
		fheight = LaraFloorFront(item, item->pos.yRot, LARA_RAD + 4);
	else if (TrInput & IN_BACK)
		rheight = LaraFloorFront(item, item->pos.yRot - ANGLE(180.0f), LARA_RAD + 4); // TR3: item->pos.yRot + ANGLE(180) ?

	if (g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_SWAMP)
	{
		if (TrInput & IN_LEFT)
			item->goalAnimState = LS_TURN_LEFT_SLOW;
		else if (TrInput & IN_RIGHT)
			item->goalAnimState = LS_TURN_RIGHT_SLOW;
	}
	else
	{
		if (TrInput & IN_LSTEP)
		{
			short height, ceiling;

			height = LaraFloorFront(item, item->pos.yRot - ANGLE(90.0f), LARA_RAD + 48);
			ceiling = LaraCeilingFront(item, item->pos.yRot - ANGLE(90.0f), LARA_RAD + 48, LARA_HITE);

			if ((height < 128 && height > -128) && HeightType != BIG_SLOPE && ceiling <= 0)
				item->goalAnimState = LS_STEP_LEFT;
		}
		else if (TrInput & IN_RSTEP)
		{
			short height, ceiling;

			height = LaraFloorFront(item, item->pos.yRot + ANGLE(90.0f), LARA_RAD + 48);
			ceiling = LaraCeilingFront(item, item->pos.yRot + ANGLE(90.0f), LARA_RAD + 48, LARA_HITE);

			if ((height < 128 && height > -128) && HeightType != BIG_SLOPE && ceiling <= 0)
				item->goalAnimState = LS_STEP_RIGHT;
		}
		else if (TrInput & IN_LEFT)
		{
			item->goalAnimState = LS_TURN_LEFT_SLOW;
		}
		else if (TrInput & IN_RIGHT)
		{
			item->goalAnimState = LS_TURN_RIGHT_SLOW;
		}
	}

	if (Lara.waterStatus == LW_WADE)
	{
		if (TrInput & IN_JUMP && !(g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_SWAMP))
			item->goalAnimState = LS_JUMP_PREPARE;

		if (TrInput & IN_FORWARD)
		{
			bool wade = false;

			if (g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_SWAMP)
			{
				if (fheight > -(STEPUP_HEIGHT - 1))
				{
					lara_as_wade(item, coll);
					wade = true;
				}
			}
			else
			{
				if ((fheight < (STEPUP_HEIGHT - 1)) && (fheight > -(STEPUP_HEIGHT - 1)))
				{
					lara_as_wade(item, coll);
					wade = true;
				}
			}

			if (!wade)
			{
				Lara.moveAngle = 0;
				coll->badPos = NO_BAD_POS;
				coll->badNeg = -STEPUP_HEIGHT;
				coll->badCeiling = 0;
				coll->slopesAreWalls = true;
				coll->radius = LARA_RAD + 2;
				coll->facing = Lara.moveAngle;

				GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, LARA_HITE);
				if (TestLaraVault(item, coll))
					return;

				coll->radius = LARA_RAD;
			}
		}
		else if (TrInput & IN_BACK)
		{
			if ((rheight < (STEPUP_HEIGHT - 1)) && (rheight > -(STEPUP_HEIGHT - 1)))
				lara_as_back(item, coll);
		}
	}
	else
	{
		if (TrInput & IN_JUMP)
		{
			item->goalAnimState = LS_JUMP_PREPARE;
		}
		else if (TrInput & IN_FORWARD)
		{
			short height, ceiling;

			height = LaraFloorFront(item, item->pos.yRot, LARA_RAD + 4);
			ceiling = LaraCeilingFront(item, item->pos.yRot, LARA_RAD + 4, LARA_HITE);

			if ((HeightType == BIG_SLOPE || HeightType == DIAGONAL) && (height < 0 || ceiling > 0))
			{
				item->goalAnimState = LS_STOP;
				return;
			}

//			if (height >= -STEP_SIZE || fheight >= -STEP_SIZE)
//			{
				if (TrInput & IN_WALK)
					lara_as_walk(item, coll);
				else
					lara_as_run(item, coll);
/*			}
			else
			{
				Lara.moveAngle = 0;

				coll->badPos = NO_BAD_POS;
				coll->badNeg = -STEPUP_HEIGHT;
				coll->badCeiling = 0;
				coll->radius = LARA_RAD + 2;
				coll->slopesAreWalls = true;
				coll->facing = Lara.moveAngle;

				GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, LARA_HITE);

				if (!TestLaraVault(item, coll))
				{
					coll->radius = LARA_RAD;
					item->goalAnimState = LS_STOP;
				}
			}*/
		}
		else if (TrInput & IN_BACK)
		{
			if (TrInput & IN_WALK)
			{
				if ((rheight < (STEPUP_HEIGHT - 1)) && (rheight > -(STEPUP_HEIGHT - 1)) && HeightType != BIG_SLOPE)
					lara_as_back(item, coll);
			}
			else if (rheight > -(STEPUP_HEIGHT - 1))
			{
				item->goalAnimState = LS_HOP_BACK;
			}
		}
	}
}

void lara_col_jumper(ITEM_INFO* item, COLL_INFO* coll)
{
	coll->badPos = NO_BAD_POS;
	coll->badNeg = -STEPUP_HEIGHT;
	coll->badCeiling = BAD_JUMP_CEILING;
	coll->facing = Lara.moveAngle;

	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, LARA_HITE);
	LaraDeflectEdgeJump(item, coll);

	if (item->fallspeed > 0 && coll->midFloor <= 0)
	{
		if (LaraLandedBad(item, coll))
			item->goalAnimState = LS_DEATH;
		else
			item->goalAnimState = LS_STOP;

		item->fallspeed = 0;
		item->gravityStatus = 0;

		if (coll->midFloor != NO_HEIGHT)
			item->pos.yPos += coll->midFloor;
	}
}

void lara_default_col(ITEM_INFO* item, COLL_INFO* coll)//1C80C(<), 1C940(<) (F)
{
	Lara.moveAngle = 0;
	coll->badPos = 384;
	coll->badNeg = -STEPUP_HEIGHT;
	coll->badCeiling = 0;
	coll->slopesArePits = true;
	coll->slopesAreWalls = true;
	coll->facing = Lara.moveAngle;
	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, LARA_HITE);
}

void lara_col_wade(ITEM_INFO* item, COLL_INFO* coll)
{
	Lara.moveAngle = 0;

	coll->badPos = NO_BAD_POS;
	coll->badNeg = -STEPUP_HEIGHT;
	coll->badCeiling = 0;

	coll->slopesAreWalls = true;

	coll->facing = Lara.moveAngle;
	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, LARA_HITE);

	if (LaraHitCeiling(item, coll))
		return;

	if (TestLaraVault(item, coll))
		return;

	if (LaraDeflectEdge(item, coll))
	{
		item->pos.zRot = 0;


		if ((coll->frontType == WALL || coll->frontType == SPLIT_TRI) && coll->frontFloor < -((STEP_SIZE*5)/2) && !(g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_SWAMP))
		{
			item->goalAnimState = LS_SPLAT;
			if (GetChange(item, &g_Level.Anims[item->animNumber]))
				return;
		}

		LaraCollideStop(item, coll);
	}


	if (coll->midFloor >= -STEPUP_HEIGHT && coll->midFloor < -STEP_SIZE/2 && !(g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_SWAMP))
	{
		item->goalAnimState = LS_STEP_UP;
		GetChange(item, &g_Level.Anims[item->animNumber]);
	}

	if (coll->midFloor >= 50 && !(g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_SWAMP))
	{
		item->pos.yPos += 50;
		return;
	}

	if (!(g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_SWAMP) || coll->midFloor < 0)
		item->pos.yPos += coll->midFloor;   // Enforce to floor height.. if not a swamp room.
	else if (g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_SWAMP && coll->midFloor)
		item->pos.yPos += SWAMP_GRAVITY;
}

void lara_col_fastdive(ITEM_INFO* item, COLL_INFO* coll)//1C558(<), 1C68C(<) (F)
{
	Lara.moveAngle = 0;

	coll->badPos = NO_BAD_POS;
	coll->badNeg = -STEPUP_HEIGHT;
	coll->badCeiling = BAD_JUMP_CEILING;

	coll->facing = Lara.moveAngle;
	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, LARA_HITE);
	LaraDeflectEdgeJump(item, coll);

	if (coll->midFloor <= 0 && item->fallspeed > 0)
	{
		if (item->fallspeed <= 133)
			item->goalAnimState = LS_STOP;
		else
			item->goalAnimState = LS_DEATH;

		item->fallspeed = 0;
		item->gravityStatus = 0;

		if (coll->midFloor != NO_HEIGHT)
			item->pos.yPos += coll->midFloor;
	}
}

void lara_col_swandive(ITEM_INFO* item, COLL_INFO* coll)//1C4A0(<), 1C5D4(<) (F)
{
	Lara.moveAngle = 0;

	coll->badPos = NO_BAD_POS;
	coll->badNeg = -STEPUP_HEIGHT;
	coll->badCeiling = BAD_JUMP_CEILING;

	coll->facing = Lara.moveAngle;
	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, LARA_HITE);
	LaraDeflectEdgeJump(item, coll);

	if (coll->midFloor <= 0 && item->fallspeed > 0)
	{
		item->goalAnimState = LS_STOP;
		item->fallspeed = 0;
		item->gravityStatus = 0;

		if (coll->midFloor != NO_HEIGHT)
			item->pos.yPos += coll->midFloor;
	}
}

void lara_col_roll2(ITEM_INFO* item, COLL_INFO* coll)//1C384, 1C4B8 (F)
{
	Camera.laraNode = 0;
	Lara.moveAngle = ANGLE(180);

	coll->badPos = NO_BAD_POS;
	coll->badNeg = -STEPUP_HEIGHT;
	coll->badCeiling = 0;
	coll->slopesAreWalls = true;

	item->gravityStatus = false;
	item->fallspeed = 0;

	coll->facing = Lara.moveAngle;
	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, LARA_HITE);

	if (LaraHitCeiling(item, coll))
		return;
	if (TestLaraSlide(item, coll))
		return;

	if (coll->midFloor > 200)
	{
		item->animNumber = LA_FALL_BACK;
		item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
		item->currentAnimState = LS_FALL_BACK;
		item->goalAnimState = LS_FALL_BACK;
		item->fallspeed = 0;
		item->gravityStatus = true;
		return;
	}

	ShiftItem(item, coll);

	if (coll->midFloor != NO_HEIGHT)
		item->pos.yPos += coll->midFloor;
}

void lara_col_roll(ITEM_INFO* item, COLL_INFO* coll)//1C2B0, 1C3E4 (F)
{
	Lara.moveAngle = 0;

	coll->badPos = NO_BAD_POS;
	coll->badNeg = -STEPUP_HEIGHT;
	coll->badCeiling = 0;
	coll->slopesArePits = false;
	coll->slopesAreWalls = true;

	item->gravityStatus = false;
	item->fallspeed = 0;

	coll->facing = Lara.moveAngle;
	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, LARA_HITE);

	if (LaraHitCeiling(item, coll))
		return;
	if (TestLaraSlide(item, coll))
		return;
	if (LaraFallen(item, coll))
		return;

	if (TrInput & IN_FORWARD && item->animNumber == LA_SWANDIVE_ROLL)
	{
		item->goalAnimState = LS_RUN_FORWARD;
	}

	ShiftItem(item, coll);

	if (coll->midFloor != NO_HEIGHT)
		item->pos.yPos += coll->midFloor;
}

void lara_col_fallback(ITEM_INFO* item, COLL_INFO* coll)//1C1B4(<), 1C2E8(<) (F)
{
	Lara.moveAngle = ANGLE(180);

	coll->badPos = NO_BAD_POS;
	coll->badNeg = -STEPUP_HEIGHT;
	coll->badCeiling = BAD_JUMP_CEILING;

	coll->facing = Lara.moveAngle;
	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, LARA_HITE);
	LaraDeflectEdgeJump(item, coll);

	if (item->fallspeed > 0 && coll->midFloor <= 0)
	{
		if (LaraLandedBad(item, coll))
			item->goalAnimState = LS_DEATH;
		else
			item->goalAnimState = LS_STOP;

		item->fallspeed = 0;
		item->gravityStatus = 0;

		if (coll->midFloor != NO_HEIGHT)
			item->pos.yPos += coll->midFloor;
	}
}

void lara_col_leftjump(ITEM_INFO* item, COLL_INFO* coll)//1C188(<), 1C2BC(<) (F)
{
	Lara.moveAngle = -ANGLE(90);
	lara_col_jumper(item, coll);
}

void lara_col_rightjump(ITEM_INFO* item, COLL_INFO* coll)//1C15C(<), 1C290(<) (F)
{
	Lara.moveAngle = ANGLE(90);
	lara_col_jumper(item, coll);
}

void lara_col_backjump(ITEM_INFO* item, COLL_INFO* coll)//1C130(<), 1C264(<) (F)
{
	Lara.moveAngle = ANGLE(180);
	lara_col_jumper(item, coll);
}

void lara_col_stepleft(ITEM_INFO* item, COLL_INFO* coll)//1C0E8(<), 1C21C(<) (F)
{
	lara_col_stepright(item, coll);
}

void lara_col_stepright(ITEM_INFO* item, COLL_INFO* coll)//1BFB0, 1C0E4 (F)
{
	if (item->currentAnimState == LS_STEP_RIGHT)
		Lara.moveAngle = ANGLE(90);
	else
		Lara.moveAngle = -ANGLE(90);

	item->gravityStatus = false;
	item->fallspeed = 0;

	if (Lara.waterStatus == LW_WADE)
		coll->badPos = NO_BAD_POS;
	else
		coll->badPos = 128;

	coll->slopesAreWalls = true;
	coll->slopesArePits = true;

	coll->badNeg = -128;
	coll->badCeiling = 0;

	coll->facing = Lara.moveAngle;
	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, LARA_HITE);

	if (!LaraHitCeiling(item, coll))
	{
		if (LaraDeflectEdge(item, coll))
			LaraCollideStop(item, coll);

		if (!LaraFallen(item, coll) && !TestLaraSlide(item, coll) && coll->midFloor != NO_HEIGHT)
			item->pos.yPos += coll->midFloor;
	}
}

void lara_col_back(ITEM_INFO* item, COLL_INFO* coll) // (F) (D)
{
	item->gravityStatus = false;
	item->fallspeed = 0;
	Lara.moveAngle = ANGLE(180);

	if (Lara.waterStatus == LW_WADE)
		coll->badPos = NO_BAD_POS;
	else
		coll->badPos = STEPUP_HEIGHT;
	coll->badNeg = -STEPUP_HEIGHT;
	coll->badCeiling = 0;
	coll->slopesArePits = true;
	coll->slopesAreWalls = 1;
	coll->facing = Lara.moveAngle;
	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, LARA_HITE);

	if (LaraHitCeiling(item, coll))
		return;

	if (LaraDeflectEdge(item, coll))
		LaraCollideStop(item, coll);

	if (LaraFallen(item, coll))
		return;

	if (coll->midFloor > STEP_SIZE / 2 && coll->midFloor < STEPUP_HEIGHT)
	{
		item->goalAnimState = LS_STEP_BACK_DOWN;
		GetChange(item, &g_Level.Anims[item->animNumber]);
	}

	if (TestLaraSlide(item, coll))
		return;

#if 0
	if (!(g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_SWAMP) || coll->midFloor < 0)
		item->pos.yPos += coll->midFloor;
	else if (g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_SWAMP && coll->midFloor)
		item->pos.yPos += SWAMP_GRAVITY;
#else
	if (coll->midFloor != NO_HEIGHT)
		item->pos.yPos += coll->midFloor;
#endif
}

void lara_col_compress(ITEM_INFO* item, COLL_INFO* coll)//1BD30, 1BE64 (F)
{
	item->fallspeed = 0;
	item->gravityStatus = false;

	coll->badPos = NO_BAD_POS;
	coll->badNeg = NO_HEIGHT;
	coll->badCeiling = 0;

	coll->facing = Lara.moveAngle;
	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, LARA_HITE);

	if (!LaraFallen(item, coll))
	{
		if (coll->midCeiling > -100)
		{
			item->animNumber = LA_STAND_SOLID;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			item->goalAnimState = LS_STOP;
			item->currentAnimState = LS_STOP;

			item->speed = 0;
			item->fallspeed = 0;
			item->gravityStatus = false;

			item->pos.xPos = coll->old.x;
			item->pos.yPos = coll->old.y;
			item->pos.zPos = coll->old.z;
		}

		if (coll->midFloor > -256 && coll->midFloor < 256)
			item->pos.yPos += coll->midFloor;
	}
}

void lara_col_land(ITEM_INFO* item, COLL_INFO* coll)//1BD10(<), 1BE44(<) (F)
{
	lara_col_stop(item, coll);
}

void lara_col_splat(ITEM_INFO* item, COLL_INFO* coll)//1BC74(<), 1BDA8(<) (F)
{
	Lara.moveAngle = 0;

	coll->slopesAreWalls = true;
	coll->slopesArePits = true;

	coll->badPos = 384;
	coll->badNeg = -STEPUP_HEIGHT;
	coll->badCeiling = 0;

	coll->facing = Lara.moveAngle;
	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, LARA_HITE);
	ShiftItem(item, coll);

	if (coll->midFloor >= -256 && coll->midFloor <= 256)
		item->pos.yPos += coll->midFloor;
}

void lara_col_fastfall(ITEM_INFO* item, COLL_INFO* coll)//1BB88, 1BCBC (F)
{
	item->gravityStatus = true;

	coll->badPos = NO_BAD_POS;
	coll->badNeg = -STEPUP_HEIGHT;
	coll->badCeiling = BAD_JUMP_CEILING;

	coll->facing = Lara.moveAngle;
	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, LARA_HITE);
	LaraSlideEdgeJump(item, coll);

	if (coll->midFloor <= 0)
	{
		if (LaraLandedBad(item, coll))
		{
			item->goalAnimState = LS_DEATH;
		}
		else
		{
			item->goalAnimState = LS_STOP;
			item->currentAnimState = LS_STOP;
			item->animNumber = LA_FREEFALL_LAND;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
		}

		StopSoundEffect(SFX_LARA_FALL);

		item->fallspeed = 0;
		item->gravityStatus = false;

		if (coll->midFloor != NO_HEIGHT)
			item->pos.yPos += coll->midFloor;
	}
}

void lara_col_death(ITEM_INFO* item, COLL_INFO* coll)//1BADC(<), 1BC10(<) (F)
{
	StopSoundEffect(SFX_LARA_FALL);

	Lara.moveAngle = 0;
	coll->badPos = 384;
	coll->badNeg = -STEPUP_HEIGHT;
	coll->badCeiling = 0;
	coll->radius = 400;

	coll->facing = Lara.moveAngle;
	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, LARA_HITE);
	ShiftItem(item, coll);

	item->hitPoints = -1;
	Lara.air = -1;

	if (coll->midFloor != NO_HEIGHT)
		item->pos.yPos += coll->midFloor;
}

void lara_col_turn_l(ITEM_INFO* item, COLL_INFO* coll) // (F) (D)
{
	lara_col_turn_r(item, coll);
}

void lara_col_turn_r(ITEM_INFO* item, COLL_INFO* coll) // (F) (D)
{
	item->fallspeed = 0;
	item->gravityStatus = false;
	Lara.moveAngle = 0;
	coll->badPos = STEPUP_HEIGHT;
	coll->badNeg = -STEPUP_HEIGHT;
	coll->badCeiling = 0;
	coll->slopesAreWalls = 1;
	coll->slopesArePits = true;
	coll->facing = Lara.moveAngle;
	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, LARA_HITE);

#if 1
	if (coll->midFloor > 100)
#else
	if (coll->midFloor > 100 && !(g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_SWAMP))
#endif
	{
		item->fallspeed = 0;
		item->animNumber = LA_FALL_START;
		item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
		item->currentAnimState = LS_JUMP_FORWARD;
		item->goalAnimState = LS_JUMP_FORWARD;
		item->gravityStatus = true;
		return;
	}

	if (TestLaraSlide(item, coll))
		return;

#if 1
	if (coll->midFloor != NO_HEIGHT)
		item->pos.yPos += coll->midFloor;
#else
	if (!(g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_SWAMP) || coll->midFloor < 0)
		item->pos.yPos += coll->midFloor;
	else if (g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_SWAMP && coll->midFloor)
		item->pos.yPos += SWAMP_GRAVITY;
#endif
}

void lara_col_fastback(ITEM_INFO* item, COLL_INFO* coll)//1B89C, 1B9D0 (F)
{
	item->fallspeed = 0;
	item->gravityStatus = false;

	Lara.moveAngle = ANGLE(180);

	coll->slopesAreWalls = 0;
	coll->slopesArePits = true;

	coll->badPos = NO_BAD_POS;
	coll->badNeg = -STEPUP_HEIGHT;
	coll->badCeiling = 0;

	coll->facing = Lara.moveAngle;
	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, LARA_HITE);

	if (!LaraHitCeiling(item, coll))
	{
		if (coll->midFloor <= 200)
		{
			if (LaraDeflectEdge(item, coll))
				LaraCollideStop(item, coll);

			if (!TestLaraSlide(item, coll) && coll->midFloor != NO_HEIGHT)
				item->pos.yPos += coll->midFloor;
		}
		else
		{
			item->fallspeed = 0;

			item->animNumber = LA_FALL_BACK;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			item->currentAnimState = LS_FALL_BACK;
			item->goalAnimState = LS_FALL_BACK;

			item->gravityStatus = true;
		}
	}
}

void lara_col_pose(ITEM_INFO* item, COLL_INFO* coll)//1B87C(<), 1B9B0(<) (F)
{
	lara_col_stop(item, coll);
}

void lara_col_run(ITEM_INFO* item, COLL_INFO* coll)//1B64C, 1B780 (F)
{
	Lara.moveAngle = 0;

	coll->badPos = NO_BAD_POS;
	coll->badNeg = -STEPUP_HEIGHT;
	coll->badCeiling = 0;

	coll->slopesAreWalls = true;

	coll->facing = Lara.moveAngle;
	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, LARA_HITE);

	if (!LaraHitCeiling(item, coll) && !TestLaraVault(item, coll))
	{
		if (LaraDeflectEdge(item, coll))
		{
			item->pos.zRot = 0;

			if (TestWall(item, 256, 0, -640))
			{
				item->goalAnimState = LS_SPLAT;
				if (GetChange(item, &g_Level.Anims[item->animNumber]))
				{
					item->currentAnimState = LS_SPLAT;
					return;
				}
			}

			LaraCollideStop(item, coll);
		}

		if (!LaraFallen(item, coll))
		{
			if (coll->midFloor >= -STEPUP_HEIGHT && coll->midFloor < -STEP_SIZE / 2)
			{
				if (coll->frontFloor == NO_HEIGHT || coll->frontFloor < -STEPUP_HEIGHT || coll->frontFloor >= -STEP_SIZE / 2)
				{
					coll->midFloor = 0;
				}
				else
				{
					item->goalAnimState = LS_STEP_UP;
					GetChange(item, &g_Level.Anims[item->animNumber]);
				}
			}

			if (!TestLaraSlide(item, coll))
			{
				if (coll->midFloor < 50)
				{
					if (coll->midFloor != NO_HEIGHT)
						item->pos.yPos += coll->midFloor;
				}
				else
				{
					item->goalAnimState = LS_STEP_DOWN; // for theoretical running stepdown anims, not in default anims
					if (GetChange(item, &g_Level.Anims[item->animNumber]))
						item->pos.yPos += coll->midFloor; // move Lara to midFloor
					else
						item->pos.yPos += 50; // do the default aligment
				}
			}
		}
	}
}

void lara_col_walk(ITEM_INFO* item, COLL_INFO* coll)//1B3E8, 1B51C (F)
{
	item->gravityStatus = false;
	item->fallspeed = 0;

	Lara.moveAngle = 0;

	coll->badPos = 384;
	coll->badNeg = -STEPUP_HEIGHT;
	coll->badCeiling = 0;

	coll->slopesAreWalls = true;
	coll->slopesArePits = true;
	coll->lavaIsPit = 1;

	coll->facing = Lara.moveAngle;
	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, LARA_HITE);

	if (!LaraHitCeiling(item, coll) && !TestLaraVault(item, coll))
	{
		if (LaraDeflectEdge(item, coll))
		{
			item->goalAnimState = LS_SPLAT;
			if (GetChange(item, &g_Level.Anims[item->animNumber]))
				return;

			LaraCollideStop(item, coll);
		}

		if (!LaraFallen(item, coll))
		{
			if (coll->midFloor > STEP_SIZE / 2)
			{
				if (coll->frontFloor == NO_HEIGHT || coll->frontFloor <= STEP_SIZE / 2)
				{
					coll->midFloor = 0;
				}
				else
				{
					item->goalAnimState = LS_STEP_DOWN;
					GetChange(item, &g_Level.Anims[item->animNumber]);
				}
			}
			if (coll->midFloor >= -STEPUP_HEIGHT && coll->midFloor < -STEP_SIZE / 2)
			{
				if (coll->frontFloor == NO_HEIGHT ||
					coll->frontFloor < -STEPUP_HEIGHT ||
					coll->frontFloor >= -STEP_SIZE / 2)
				{
					coll->midFloor = 0;
				}
				else
				{
					item->goalAnimState = LS_STEP_UP;
					GetChange(item, &g_Level.Anims[item->animNumber]);
				}
			}

			if (!TestLaraSlide(item, coll) && coll->midFloor != NO_HEIGHT)
				item->pos.yPos += coll->midFloor;
		}
	}
}

void lara_as_controlledl(ITEM_INFO* item, COLL_INFO* coll)//1B180(<), 1B2B4(<) (F)
{
	Lara.look = false;
	coll->enableBaddiePush = false;
	coll->enableSpaz = false;
}

void lara_as_controlled(ITEM_INFO* item, COLL_INFO* coll)//1B0FC(<), 1B230(<) (F)
{
	Lara.look = false;
	coll->enableBaddiePush = false;
	coll->enableSpaz = false;
	if (item->frameNumber == g_Level.Anims[item->animNumber].frameEnd - 1)
	{
		Lara.gunStatus = LG_NO_ARMS;
		if (UseForcedFixedCamera)
			UseForcedFixedCamera = 0;
	}
}

void lara_as_wade(ITEM_INFO* item, COLL_INFO* coll)//1AF10, 1B044 (F)
{
	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_STOP;
		return;
	}

	Camera.targetElevation = -ANGLE(22.0f);

	if (g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_SWAMP)
	{
		if (TrInput & IN_LEFT)
		{
			Lara.turnRate -= LARA_TURN_RATE;
			if (Lara.turnRate < -(LARA_FAST_TURN >> 1))
				Lara.turnRate = -(LARA_FAST_TURN >> 1);

			item->pos.zRot -= LARA_LEAN_RATE;
			if (item->pos.zRot < -(LARA_LEAN_MAX >> 1))
				item->pos.zRot = -(LARA_LEAN_MAX >> 1);
		}
		else if (TrInput & IN_RIGHT)
		{
			Lara.turnRate += LARA_TURN_RATE;
			if (Lara.turnRate > (LARA_FAST_TURN >> 1))
				Lara.turnRate = (LARA_FAST_TURN >> 1);

			item->pos.zRot += LARA_LEAN_RATE;
			if (item->pos.zRot > (LARA_LEAN_MAX >> 1))
				item->pos.zRot = (LARA_LEAN_MAX >> 1);
		}

		if (TrInput & IN_FORWARD)
			item->goalAnimState = LS_WADE_FORWARD;
		else
			item->goalAnimState = LS_STOP;
	}
	else
	{
		if (TrInput & IN_LEFT)
		{
			Lara.turnRate -= LARA_TURN_RATE;
			if (Lara.turnRate < -LARA_FAST_TURN)
				Lara.turnRate = -LARA_FAST_TURN;

			item->pos.zRot -= LARA_LEAN_RATE;
			if (item->pos.zRot < -LARA_LEAN_MAX)
				item->pos.zRot = -LARA_LEAN_MAX;
		}
		else if (TrInput & IN_RIGHT)
		{
			Lara.turnRate += LARA_TURN_RATE;
			if (Lara.turnRate > LARA_FAST_TURN)
				Lara.turnRate = LARA_FAST_TURN;

			item->pos.zRot += LARA_LEAN_RATE;
			if (item->pos.zRot > LARA_LEAN_MAX)
				item->pos.zRot = LARA_LEAN_MAX;
		}

		if (TrInput & IN_FORWARD)
		{
			if (Lara.waterStatus == LW_ABOVE_WATER)
				item->goalAnimState = LS_RUN_FORWARD;
			else
				item->goalAnimState = LS_WADE_FORWARD;
		}
		else
		{
			item->goalAnimState = LS_STOP;
		}
	}
}

void lara_as_waterout(ITEM_INFO* item, COLL_INFO* coll)//1AEE4(<), 1B018(<) (F)
{
	coll->enableBaddiePush = false;
	coll->enableSpaz = false;
	Camera.flags = CF_FOLLOW_CENTER;
}

void lara_as_gymnast(ITEM_INFO* item, COLL_INFO* coll)//1AEC8(<), 1AFFC(<) (F)
{
	coll->enableBaddiePush = false;
	coll->enableSpaz = false;
}

void lara_as_fastdive(ITEM_INFO* item, COLL_INFO* coll)//1AE4C(<), 1AF80(<) (F)
{
	if (TrInput & IN_ROLL && item->goalAnimState == LS_SWANDIVE_END)
		item->goalAnimState = LS_JUMP_ROLL_180;
	coll->enableBaddiePush = true;
	coll->enableSpaz = false;
	item->speed = (item->speed * 95) / 100;
}

void lara_as_swandive(ITEM_INFO* item, COLL_INFO* coll)//1AE08(<), 1AF3C(<) (F)
{
	coll->enableBaddiePush = true;
	coll->enableSpaz = false;
	if (item->fallspeed > LARA_FREEFALL_SPEED && item->goalAnimState != LS_DIVE)
		item->goalAnimState = LS_SWANDIVE_END;
}

void lara_as_special(ITEM_INFO* item, COLL_INFO* coll)//1ADDC(<), 1AF10(<) (F)
{
	Camera.flags = CF_FOLLOW_CENTER;
	Camera.targetAngle = ANGLE(170.0f);
	Camera.targetElevation = -ANGLE(25.0f);
}

void lara_as_fallback(ITEM_INFO* item, COLL_INFO* coll)//1959C(<), 196D0(<) (F)
{
	if (item->fallspeed > LARA_FREEFALL_SPEED)
		item->goalAnimState = LS_FREEFALL;

	if (TrInput & IN_ACTION)
		if (Lara.gunStatus == LG_NO_ARMS)
			item->goalAnimState = LS_REACH;
}

void lara_as_leftjump(ITEM_INFO* item, COLL_INFO* coll)//1A92C(<), 1AA60(<) (F)
{
	Lara.look = false;
	if (item->fallspeed > LARA_FREEFALL_SPEED)
		item->goalAnimState = LS_FREEFALL;
	else if (TrInput & IN_RIGHT && item->goalAnimState != LS_STOP)
		item->goalAnimState = LS_JUMP_ROLL_180;
}

void lara_as_rightjump(ITEM_INFO* item, COLL_INFO* coll)//1A8C4(<), 1A9F8(<) (F)
{
	Lara.look = false;
	if (item->fallspeed > LARA_FREEFALL_SPEED)
		item->goalAnimState = LS_FREEFALL;
	else if (TrInput & IN_LEFT && item->goalAnimState != LS_STOP)
		item->goalAnimState = LS_JUMP_ROLL_180;
}

void lara_as_backjump(ITEM_INFO* item, COLL_INFO* coll)//1A854(<), 1A988(<) (F)
{
	Camera.targetAngle = ANGLE(135.0f);
	if (item->fallspeed <= LARA_FREEFALL_SPEED)
	{
		if (item->goalAnimState == LS_RUN_FORWARD)
		{
			item->goalAnimState = LS_STOP;
		}
		else if ((TrInput & IN_FORWARD || TrInput & IN_ROLL) && item->goalAnimState != LS_STOP)
		{
			item->goalAnimState = LS_JUMP_ROLL_180;
		}
	}
	else
	{
		item->goalAnimState = LS_FREEFALL;
	}
}

void lara_as_stepleft(ITEM_INFO* item, COLL_INFO* coll)//1A750(<), 1A884(<) (F)
{
	Lara.look = false;

	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_STOP;
		return;
	}

	if (!Lara.isMoving)
	{
		if (!(TrInput & IN_LSTEP))
		{
			item->goalAnimState = LS_STOP;
		}

		if (TrInput & IN_LEFT)
		{
			Lara.turnRate -= LARA_TURN_RATE;
			if (Lara.turnRate < -LARA_SLOW_TURN)
				Lara.turnRate = -LARA_SLOW_TURN;
		}
		else if (TrInput & IN_RIGHT)
		{
			Lara.turnRate += LARA_TURN_RATE;
			if (Lara.turnRate > LARA_SLOW_TURN)
				Lara.turnRate = LARA_SLOW_TURN;
		}
	}
}

void lara_as_stepright(ITEM_INFO* item, COLL_INFO* coll)//1A67C(<), 1A7B0(<) (F)
{
	Lara.look = false;

	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_STOP;
		return;
	}

	if (!Lara.isMoving)
	{
		if (!(TrInput & IN_RSTEP))
		{
			item->goalAnimState = LS_STOP;
		}

		if (TrInput & IN_LEFT)
		{
			Lara.turnRate -= LARA_TURN_RATE;
			if (Lara.turnRate < -ANGLE(4.0f))
				Lara.turnRate = -ANGLE(4.0f);
		}
		else if (TrInput & IN_RIGHT)
		{
			Lara.turnRate += LARA_TURN_RATE;
			if (Lara.turnRate > ANGLE(4.0f))
				Lara.turnRate = ANGLE(4.0f);
		}
	}
}

void lara_col_fastturn(ITEM_INFO* item, COLL_INFO* coll)//1A65C(<), 1A790(<) (F)
{
	lara_col_stop(item, coll);
}

void lara_as_fastturn(ITEM_INFO* item, COLL_INFO* coll)//1A5F8(<), 1A72C(<) (F)
{
	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_STOP;
		return;
	}

	if (Lara.turnRate < 0)
	{
		Lara.turnRate = -LARA_FAST_TURN;

		if (!(TrInput & IN_LEFT))
			item->goalAnimState = LS_STOP;
	}
	else
	{
		Lara.turnRate = LARA_FAST_TURN;

		if (!(TrInput & IN_RIGHT))
			item->goalAnimState = LS_STOP;
	}
}

void lara_as_null(ITEM_INFO* item, COLL_INFO* coll)//1A5DC(<), 1A710(<) (F)
{
	coll->enableBaddiePush = false;
	coll->enableSpaz = false;
}

void lara_as_back(ITEM_INFO* item, COLL_INFO* coll)//1A4F0(<), 1A624(<) (F)
{
	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_STOP;
		return;
	}

	if (!Lara.isMoving)
	{
		if ((TrInput & IN_BACK) && ((TrInput & IN_WALK) || Lara.waterStatus == LW_WADE))
			item->goalAnimState = LS_WALK_BACK;
		else
			item->goalAnimState = LS_STOP;

		if (TrInput & IN_LEFT)
		{
			Lara.turnRate -= LARA_TURN_RATE;
			if (Lara.turnRate < -ANGLE(4.0f))
				Lara.turnRate = -ANGLE(4.0f);
		}
		else if (TrInput & IN_RIGHT)
		{
			Lara.turnRate += LARA_TURN_RATE;
			if (Lara.turnRate > ANGLE(4.0f))
				Lara.turnRate = ANGLE(4.0f);
		}
	}
}

void lara_as_compress(ITEM_INFO* item, COLL_INFO* coll)
{
	if (Lara.waterStatus != LW_WADE)
	{
		if (TrInput & IN_FORWARD && LaraFloorFront(item, item->pos.yRot, 256) >= -384)
		{
			item->goalAnimState = LS_JUMP_FORWARD;
			Lara.moveAngle = 0;
		}
		else if (TrInput & IN_LEFT && LaraFloorFront(item, item->pos.yRot - ANGLE(90.0f), 256) >= -384)
		{
			item->goalAnimState = LS_JUMP_RIGHT;
			Lara.moveAngle = -ANGLE(90);
		}
		else if (TrInput & IN_RIGHT && LaraFloorFront(item, item->pos.yRot + ANGLE(90.0f), 256) >= -384)
		{
			item->goalAnimState = LS_JUMP_LEFT;
			Lara.moveAngle = ANGLE(90);
		}
		else if (TrInput & IN_BACK && LaraFloorFront(item, item->pos.yRot - ANGLE(180.0f), 256) >= -384)
		{
			item->goalAnimState = LS_JUMP_BACK;
			Lara.moveAngle = ANGLE(180);
		}
	}

	if (item->fallspeed > LARA_FREEFALL_SPEED)
		item->goalAnimState = LS_FREEFALL;
}

void lara_as_splat(ITEM_INFO* item, COLL_INFO* coll)//1A340(<), 1A474(<) (F)
{
	Lara.look = false;
}

void lara_void_func(ITEM_INFO* item, COLL_INFO* coll)//19928(<), 19A5C(<) (F)
{
	return;
}

void lara_as_fastfall(ITEM_INFO* item, COLL_INFO* coll)//198BC(<), 199F0(<) (F)
{
	item->speed = (item->speed * 95) / 100;
	if (item->fallspeed == 154)
		SoundEffect(SFX_LARA_FALL, &item->pos, 0);
}

void lara_as_death(ITEM_INFO* item, COLL_INFO* coll)//19830(<), 19964(<) (F)
{
	Lara.look = false;
	coll->enableBaddiePush = false;
	coll->enableSpaz = false;
	if (BinocularRange)
	{
		BinocularRange = 0;
		LaserSight = 0;
		AlterFOV(ANGLE(80.0f));
		LaraItem->meshBits = -1;
		Lara.busy = false;
	}
}

void lara_as_turn_l(ITEM_INFO* item, COLL_INFO* coll)//1972C(<), 19860(<) (F)
{
	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_STOP;
		return;
	}

	Lara.turnRate -= LARA_TURN_RATE;

	if (Lara.gunStatus != LG_READY || Lara.waterStatus == LW_WADE)
	{
		if (Lara.turnRate < -ANGLE(4.0f))
		{
			if (TrInput & IN_WALK)
				Lara.turnRate = -ANGLE(4.0f);
			else
				item->goalAnimState = LS_TURN_FAST;
		}
	}
	else
	{
		item->goalAnimState = LS_TURN_FAST;
	}

	if (!(TrInput & IN_FORWARD))
	{
		if (!(TrInput & IN_LEFT))
			item->goalAnimState = LS_STOP;

		return;
	}

	if (Lara.waterStatus == LW_WADE)
	{
		item->goalAnimState = LS_WADE_FORWARD;
	}
	else if (TrInput & IN_WALK)
	{
		item->goalAnimState = LS_WALK_FORWARD;
	}
	else
	{
		item->goalAnimState = LS_RUN_FORWARD;
	}
}

void lara_as_turn_r(ITEM_INFO* item, COLL_INFO* coll)//19628(<), 1975C(<) (F)
{
	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_STOP;

		return;
	}

	Lara.turnRate += LARA_TURN_RATE;

	if (Lara.gunStatus != LG_READY || Lara.waterStatus == LW_WADE)
	{
		if (Lara.turnRate > ANGLE(4.0f))
		{
			if (TrInput & IN_WALK)
				Lara.turnRate = ANGLE(4.0f);
			else
				item->goalAnimState = LS_TURN_FAST;
		}
	}
	else
	{
		item->goalAnimState = LS_TURN_FAST;
	}

	if (!(TrInput & IN_FORWARD))
	{
		if (!(TrInput & IN_RIGHT))
			item->goalAnimState = LS_STOP;

		return;
	}

	if (Lara.waterStatus == LW_WADE)
	{
		item->goalAnimState = LS_WADE_FORWARD;
	}
	else if (TrInput & IN_WALK)
	{
		item->goalAnimState = LS_WALK_FORWARD;
	}
	else
	{
		item->goalAnimState = LS_RUN_FORWARD;
	}
}

void lara_as_fastback(ITEM_INFO* item, COLL_INFO* coll)//1959C(<), 196D0(<) (F)
{
	item->goalAnimState = LS_STOP;
	if (TrInput & IN_LEFT)
	{
		Lara.turnRate -= LARA_TURN_RATE;
		if (Lara.turnRate < -ANGLE(6.0f))
			Lara.turnRate = -ANGLE(6.0f);
	}
	else if (TrInput & IN_RIGHT)
	{
		Lara.turnRate += LARA_TURN_RATE;
		if (Lara.turnRate > ANGLE(6.0f))
			Lara.turnRate = ANGLE(6.0f);
	}
}

void lara_as_run(ITEM_INFO* item, COLL_INFO* coll)//192EC, 19420 (F)
{
	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_DEATH;
		return;
	}

	if (TrInput & IN_ROLL)
	{
		item->animNumber = LA_ROLL_180_START;
		item->frameNumber = g_Level.Anims[item->animNumber].frameBase + 2;
		item->currentAnimState = LS_ROLL_FORWARD;
		item->goalAnimState = LS_STOP;
		return;
	}

	if (TrInput & IN_SPRINT && DashTimer)
	{
		item->goalAnimState = LS_SPRINT;
		return;
	}

	if (TrInput & IN_DUCK &&
		Lara.waterStatus != LW_WADE &&
		(Lara.gunStatus == LG_NO_ARMS ||
			Lara.gunType == WEAPON_NONE ||
			Lara.gunType == WEAPON_PISTOLS ||
			Lara.gunType == WEAPON_REVOLVER ||
			Lara.gunType == WEAPON_UZI ||
			Lara.gunType == WEAPON_FLARE))
	{
		item->goalAnimState = LS_CROUCH_IDLE;
		return;
	}

	if (TrInput & IN_LEFT)
	{
		Lara.turnRate -= LARA_TURN_RATE;
		if (Lara.turnRate < -LARA_FAST_TURN)
			Lara.turnRate = -LARA_FAST_TURN;

		item->pos.zRot -= LARA_LEAN_RATE;
		if (item->pos.zRot < -LARA_LEAN_MAX)
			item->pos.zRot = -LARA_LEAN_MAX;
	}
	else if (TrInput & IN_RIGHT)
	{
		Lara.turnRate += LARA_TURN_RATE;
		if (Lara.turnRate > LARA_FAST_TURN)
			Lara.turnRate = LARA_FAST_TURN;

		item->pos.zRot += LARA_LEAN_RATE;
		if (item->pos.zRot > LARA_LEAN_MAX)
			item->pos.zRot = LARA_LEAN_MAX;
	}

	if (item->animNumber == LA_STAND_TO_RUN)
	{
		doJump = false;
	}
	else if (item->animNumber == LA_RUN)
	{
		if (item->frameNumber == 4)
			doJump = true;
	}
	else
	{
		doJump = true;
	}

	if (TrInput & IN_JUMP && doJump && !item->gravityStatus)
	{
		item->goalAnimState = LS_JUMP_FORWARD;
	}
	else if (TrInput & IN_FORWARD)
	{
		if (Lara.waterStatus == LW_WADE)
		{
			item->goalAnimState = LS_WADE_FORWARD;
		}
		else
		{
			if (TrInput & IN_WALK)
				item->goalAnimState = LS_WALK_FORWARD;
			else
				item->goalAnimState = LS_RUN_FORWARD;
		}
	}
	else
	{
		item->goalAnimState = LS_STOP;
	}
}

void lara_as_walk(ITEM_INFO* item, COLL_INFO* coll)//191B8(<), 192EC(<) (F)
{
	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_STOP;
		return;
	}

	if (!Lara.isMoving)
	{
		if (TrInput & IN_LEFT)
		{
			Lara.turnRate -= LARA_TURN_RATE;
			if (Lara.turnRate < -ANGLE(4.0f))
				Lara.turnRate = -ANGLE(4.0f);
		}
		else if (TrInput & IN_RIGHT)
		{
			Lara.turnRate += LARA_TURN_RATE;
			if (Lara.turnRate > ANGLE(4.0f))
				Lara.turnRate = ANGLE(4.0f);
		}

		if (TrInput & IN_FORWARD)
		{
			if (Lara.waterStatus == LW_WADE)
			{
				item->goalAnimState = LS_WADE_FORWARD;
			}
			else if (TrInput & IN_WALK)
			{
				item->goalAnimState = LS_WALK_FORWARD;
			}
			else
			{
				item->goalAnimState = LS_RUN_FORWARD;
			}
		}
		else
		{
			item->goalAnimState = LS_STOP;
		}
	}
}

// FIXED
void lara_col_reach(ITEM_INFO* item, COLL_INFO* coll)//18D0C, 18E40 (F)
{
	if (Lara.ropePtr == -1)
		item->gravityStatus = true;

	Lara.moveAngle = 0;

	coll->badPos = NO_BAD_POS;
	coll->badNeg = 0;
	coll->badCeiling = BAD_JUMP_CEILING;

	coll->facing = Lara.moveAngle;
	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, LARA_HITE);

	short angle;
	bool result = false;
	int edge = 0;
	int edgeCatch = 0;

	if (TrInput & IN_ACTION && Lara.gunStatus == LG_NO_ARMS && !coll->hitStatic)
	{
		if (Lara.canMonkeySwing && coll->collType == CT_TOP)
		{
			Lara.headYrot = 0;
			Lara.headXrot = 0;
			Lara.torsoYrot = 0;
			Lara.torsoXrot = 0;
			Lara.gunStatus = LG_HANDS_BUSY;

			item->animNumber = LA_REACH_TO_MONKEYSWING;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			item->goalAnimState = LS_MONKEYSWING_IDLE;
			item->currentAnimState = LS_MONKEYSWING_IDLE;
			item->gravityStatus = false;
			item->speed = 0;
			item->fallspeed = 0;

			return;
		}

		if (coll->midCeiling <= -384 &&
			coll->midFloor >= 200 &&
			coll->collType == CT_FRONT)
		{
			edgeCatch = LaraTestEdgeCatch(item, coll, &edge);

			if (!(!edgeCatch || edgeCatch < 0 && !LaraTestHangOnClimbWall(item, coll)))
			{
				angle = item->pos.yRot;
				if (coll->midSplitFloor && coll->frontSplitFloor == coll->midSplitFloor)
				{
					result = SnapToDiagonal(angle, 35);
				}
				else
				{
					result = SnapToQuadrant(angle, 35);
				}
			}
		}
	}

	if (!result)
	{
		LaraSlideEdgeJump(item, coll);
		coll->facing = Lara.moveAngle;
		GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, LARA_HITE);
		ShiftItem(item, coll);

		if (item->fallspeed > 0 && coll->midFloor <= 0)
		{
			if (LaraLandedBad(item, coll))
			{
				item->goalAnimState = LS_DEATH;
			}
			else
			{
				item->goalAnimState = LS_STOP;
				item->fallspeed = 0;
				item->gravityStatus = false;
				if (coll->midFloor != NO_HEIGHT)
					item->pos.yPos += coll->midFloor;
			}
		}
	}
	else
	{
		if (TestHangSwingIn(item, angle))
		{
/*			if (TR12_OSCILLATE_HANG == true)
			{
				Lara.headYrot = 0;
				Lara.headXrot = 0;
				Lara.torsoYrot = 0;
				Lara.torsoXrot = 0;
				item->animNumber = LA_REACH_TO_HANG_OSCILLATE;
				item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
				item->currentAnimState = LS_HANG;
				item->goalAnimState = LS_HANG;
			}
			else
			{	*/
				Lara.headYrot = 0;
				Lara.headXrot = 0;
				Lara.torsoYrot = 0;
				Lara.torsoXrot = 0;
				item->animNumber = LA_REACH_TO_MONKEYSWING;
				item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
				item->currentAnimState = LS_MONKEYSWING_IDLE;
				item->goalAnimState = LS_MONKEYSWING_IDLE;
//			}
		}
		else
		{
			if (TestHangFeet(item, angle))
			{
				item->animNumber = LA_REACH_TO_HANG;
				item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
				item->currentAnimState = LS_HANG;
				item->goalAnimState = LS_HANG_FEET;
			}
			else
			{
				item->animNumber = LA_REACH_TO_HANG;
				item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
				item->currentAnimState = LS_HANG;
				item->goalAnimState = LS_HANG;
			}
		}

		BOUNDING_BOX* bounds = GetBoundsAccurate(item);

		if (edgeCatch <= 0)
		{
			item->pos.yPos = edge - bounds->Y1 - 22;
		}
		else
		{
			item->pos.yPos += coll->frontFloor - bounds->Y1;

			if (coll->midSplitFloor)
			{
				Vector2 v = GetDiagonalIntersect(item->pos.xPos, item->pos.zPos, coll->midSplitFloor, LARA_RAD, angle);
				item->pos.xPos = v.x;
				item->pos.zPos = v.y;
			}
			else
			{
				Vector2 v = GetOrthogonalIntersect(item->pos.xPos, item->pos.zPos, LARA_RAD, angle);
				item->pos.xPos = v.x;
				item->pos.zPos = v.y;
			}
		}

		item->pos.yRot = angle;

		item->gravityStatus = true;
		item->speed = 2;
		item->fallspeed = 1;

		Lara.gunStatus = LG_HANDS_BUSY;
	}
}

void lara_as_reach(ITEM_INFO* item, COLL_INFO* coll)//18CE0(<), 18E14(<) (F)
{
	Camera.targetAngle = ANGLE(85.0f);
	if (item->fallspeed > LARA_FREEFALL_SPEED)
		item->goalAnimState = LS_FREEFALL;
}

void lara_col_forwardjump(ITEM_INFO* item, COLL_INFO* coll)//18B88, 18CBC (F)
{
	if (item->speed < 0)
		Lara.moveAngle = ANGLE(180);
	else
		Lara.moveAngle = 0;

	coll->badPos = NO_BAD_POS;
	coll->badNeg = -STEPUP_HEIGHT;
	coll->badCeiling = BAD_JUMP_CEILING;

	coll->facing = Lara.moveAngle;
	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, LARA_HITE);
	LaraDeflectEdgeJump(item, coll);

	if (item->speed < 0)
		Lara.moveAngle = 0;

	if (coll->midFloor <= 0 && item->fallspeed > 0)
	{
		if (LaraLandedBad(item, coll))
		{
			item->goalAnimState = LS_DEATH;
		}
		else
		{
			if (Lara.waterStatus == LW_WADE)
			{
				item->goalAnimState = LS_STOP;
			}
			else
			{
				if (TrInput & IN_FORWARD && !(TrInput & IN_STEPSHIFT))
					item->goalAnimState = LS_RUN_FORWARD;
				else
					item->goalAnimState = LS_STOP;
			}
		}

		item->gravityStatus = false;
		item->fallspeed = 0;
		item->speed = 0;

		if (coll->midFloor != NO_HEIGHT)
			item->pos.yPos += coll->midFloor;

		AnimateLara(item);
	}
}

void lara_as_forwardjump(ITEM_INFO* item, COLL_INFO* coll)//18A34, 18B68 (F)
{
	if (item->goalAnimState == LS_SWANDIVE_START ||
		item->goalAnimState == LS_REACH)
		item->goalAnimState = LS_JUMP_FORWARD;

	if (item->goalAnimState != LS_DEATH &&
		item->goalAnimState != LS_STOP &&
		item->goalAnimState != LS_RUN_FORWARD)
	{
		if (Lara.gunStatus == LG_NO_ARMS && TrInput & IN_ACTION)
			item->goalAnimState = LS_REACH;

		if (TrInput & IN_BACK || TrInput & IN_ROLL)
			item->goalAnimState = LS_JUMP_ROLL_180;

		if (Lara.gunStatus == LG_NO_ARMS && TrInput & IN_WALK)
			item->goalAnimState = LS_SWANDIVE_START;

		if (item->fallspeed > LARA_FREEFALL_SPEED)
			item->goalAnimState = LS_FREEFALL;
	}

	if (TrInput & IN_LEFT)
	{
		Lara.turnRate -= LARA_TURN_RATE;

		if (Lara.turnRate < -ANGLE(3.0f))
			Lara.turnRate = -ANGLE(3.0f);
	}
	else if (TrInput & IN_RIGHT)
	{
		Lara.turnRate += LARA_TURN_RATE;

		if (Lara.turnRate > ANGLE(3.0f))
			Lara.turnRate = ANGLE(3.0f);
	}
}

void lara_col_upjump(ITEM_INFO* item, COLL_INFO* coll) // (F) (D)
{
	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_STOP;
		return;
	}

	Lara.moveAngle = 0;

	coll->badPos = NO_BAD_POS;
	coll->badNeg = -STEPUP_HEIGHT;
	coll->badCeiling = BAD_JUMP_CEILING;
	coll->hitCeiling = false;
	coll->facing = item->speed < 0 ? Lara.moveAngle + ANGLE(180.0f) : Lara.moveAngle;

	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, 870);

	if (TrInput & IN_ACTION && Lara.gunStatus == LG_NO_ARMS && !coll->hitStatic)
	{
		if (Lara.canMonkeySwing && coll->collType == CT_TOP)
		{
			item->goalAnimState = LS_MONKEYSWING_IDLE;
			item->currentAnimState = LS_MONKEYSWING_IDLE;
			item->animNumber = LA_JUMP_UP_TO_MONKEYSWING;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			item->gravityStatus = false;
			item->speed = 0;
			item->fallspeed = 0;

			Lara.gunStatus = LG_HANDS_BUSY;

			MonkeySwingSnap(item, coll);

			return;
		}

		if (coll->collType == CT_FRONT && coll->midCeiling <= -STEPUP_HEIGHT)
		{
			int edge;
			int edgeCatch = LaraTestEdgeCatch(item, coll, &edge);

			if (edgeCatch)
			{
				if (edgeCatch >= 0 || LaraTestHangOnClimbWall(item, coll))
				{
					short angle = item->pos.yRot;
					bool result;

					if (coll->midSplitFloor && coll->frontSplitFloor == coll->midSplitFloor)
					{
						result = SnapToDiagonal(angle, 35);
					}
					else
					{
						result = SnapToQuadrant(angle, 35);
					}

					if (result)
					{
						BOUNDING_BOX* bounds;

						if (TestHangSwingIn(item, angle))
						{
								item->animNumber = LA_JUMP_UP_TO_MONKEYSWING;
								item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
								item->goalAnimState = LS_MONKEYSWING_IDLE;
								item->currentAnimState = LS_MONKEYSWING_IDLE;
						}
						else
						{
							if (TestHangFeet(item, angle))
							{
								item->animNumber = LA_REACH_TO_HANG;
								item->frameNumber = g_Level.Anims[item->animNumber].frameBase + 12;
								item->currentAnimState = LS_HANG;
								item->goalAnimState = LS_HANG_FEET;
							}
							else
							{
								item->animNumber = LA_REACH_TO_HANG;
								item->frameNumber = g_Level.Anims[item->animNumber].frameBase + 12;
								item->currentAnimState = LS_HANG;
								item->goalAnimState = LS_HANG;
							}
						}

						bounds = GetBoundsAccurate(item);

						if (edgeCatch <= 0)
							item->pos.yPos = edge - bounds->Y1 + 4;
						else
							item->pos.yPos += coll->frontFloor - bounds->Y1;

						if (coll->midSplitFloor)
						{
							Vector2 v = GetDiagonalIntersect(item->pos.xPos, item->pos.zPos, coll->midSplitFloor, LARA_RAD, item->pos.yRot);
							item->pos.xPos = v.x;
							item->pos.zPos = v.y;
						}
						else
						{
							Vector2 v = GetOrthogonalIntersect(item->pos.xPos, item->pos.zPos, LARA_RAD, item->pos.yRot);
							item->pos.xPos = v.x;
							item->pos.zPos = v.y;
						}
						item->pos.yRot = angle;

						item->gravityStatus = false;
						item->speed = 0;
						item->fallspeed = 0;

						Lara.gunStatus = LG_HANDS_BUSY;
						Lara.torsoYrot = 0;
						Lara.torsoXrot = 0;

						return;
					}
				}
			}
		}
	}

	ShiftItem(item, coll);

	if (coll->collType == CT_CLAMP ||
		coll->collType == CT_TOP ||
		coll->collType == CT_TOP_FRONT ||
		coll->hitCeiling)
		item->fallspeed = 1;

	if (coll->collType == CT_NONE)
	{
		if (item->fallspeed < -70)
		{
			if (TrInput & IN_FORWARD && item->speed < 5)
			{
				item->speed++;
			}
			else if (TrInput & IN_BACK && item->speed > -5)
			{
				item->speed -= 2;
			}
		}
	}
	else
	{
		item->speed = item->speed <= 0 ? -2 : 2;
	}

	if (item->fallspeed > 0 && coll->midFloor <= 0)
	{
		item->goalAnimState = LaraLandedBad(item, coll) ? LS_DEATH : LS_STOP;

		item->gravityStatus = false;
		item->fallspeed = 0;

		if (coll->midFloor != NO_HEIGHT)
			item->pos.yPos += coll->midFloor;
	}
}

void lara_as_upjump(ITEM_INFO* item, COLL_INFO* coll) // (F) (D)
{
	if (item->fallspeed > LARA_FREEFALL_SPEED)
	{
		item->goalAnimState = LS_FREEFALL;
	}
}

void lara_col_stop(ITEM_INFO* item, COLL_INFO* coll) // (F) (D)
{
	Lara.moveAngle = 0;
	coll->badPos = STEPUP_HEIGHT;
	coll->badNeg = -STEPUP_HEIGHT;
	coll->badCeiling = 0;
	item->gravityStatus = false;
	item->fallspeed = 0;
	coll->slopesArePits = true;
	coll->slopesAreWalls = 1;
	coll->facing = Lara.moveAngle;
	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, LARA_HITE);

	if (LaraHitCeiling(item, coll))
		return;

	if (LaraFallen(item, coll))
		return;

	if (TestLaraSlide(item, coll))
		return;

	ShiftItem(item, coll);

#if 1
	if (coll->midFloor != NO_HEIGHT)
		item->pos.yPos += coll->midFloor;
#else
	if (!(g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_SWAMP) || coll->midFloor < 0)
		item->pos.yPos += coll->midFloor;
	else if (g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_SWAMP && coll->midFloor)
		item->pos.yPos += SWAMP_GRAVITY;
#endif
}

void lara_col_dashdive(ITEM_INFO* item, COLL_INFO* coll)//15E5C, 15F90 (F)
{
	if (item->speed < 0)
		Lara.moveAngle = ANGLE(180);
	else
		Lara.moveAngle = 0;

	coll->badPos = NO_BAD_POS;
	coll->badNeg = -256;
	coll->badCeiling = BAD_JUMP_CEILING;

	coll->slopesAreWalls = true;

	coll->facing = Lara.moveAngle;
	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, LARA_HITE);
	LaraDeflectEdgeJump(item, coll);

	if (!LaraFallen(item, coll))
	{
		if (item->speed < 0)
			Lara.moveAngle = 0;

		if (coll->midFloor <= 0 && item->fallspeed > 0)
		{
			if (LaraLandedBad(item, coll))
			{
				item->goalAnimState = LS_DEATH;
			}
			else if (Lara.waterStatus == LW_WADE || !(TrInput & IN_FORWARD) || TrInput & IN_WALK)
			{
				item->goalAnimState = LS_STOP;
			}
			else
			{
				item->goalAnimState = LS_RUN_FORWARD;
			}

			item->gravityStatus = false;
			item->fallspeed = 0;
			item->pos.yPos += coll->midFloor;
			item->speed = 0;

			AnimateLara(item);
		}

		ShiftItem(item, coll);

		if (coll->midFloor != NO_HEIGHT)
			item->pos.yPos += coll->midFloor;
	}
}

void lara_as_dashdive(ITEM_INFO* item, COLL_INFO* coll)//15E1C(<), 15F50(<) (F)
{
	if (item->goalAnimState != LS_DEATH &&
		item->goalAnimState != LS_STOP &&
		item->goalAnimState != LS_RUN_FORWARD &&
		item->fallspeed > LARA_FREEFALL_SPEED)
		item->goalAnimState = LS_FREEFALL;
}

void lara_col_dash(ITEM_INFO* item, COLL_INFO* coll)//15C50, 15D84 (F)
{
	Lara.moveAngle = 0;

	coll->badPos = NO_BAD_POS;
	coll->badNeg = -STEPUP_HEIGHT;
	coll->badCeiling = 0;

	coll->slopesAreWalls = true;

	coll->facing = Lara.moveAngle;
	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, LARA_HITE);

	if (!LaraHitCeiling(item, coll) && !TestLaraVault(item, coll))
	{
		if (LaraDeflectEdge(item, coll))
		{
			item->pos.zRot = 0;

			if (TestWall(item, 256, 0, -640))
			{
				item->goalAnimState = LS_SPLAT;
				if (GetChange(item, &g_Level.Anims[item->animNumber]))
				{
					item->currentAnimState = LS_SPLAT;
					return;
				}
			}

			LaraCollideStop(item, coll);
		}

		if (!LaraFallen(item, coll))
		{
			if (coll->midFloor >= -STEPUP_HEIGHT && coll->midFloor < -STEP_SIZE/2)
			{
				item->goalAnimState = LS_STEP_UP;
				GetChange(item, &g_Level.Anims[item->animNumber]);
			}

			if (!TestLaraSlide(item, coll))
			{
				if (coll->midFloor < 50)
				{
					if (coll->midFloor != NO_HEIGHT)
						item->pos.yPos += coll->midFloor;
				}
				else
				{
					item->goalAnimState = LS_STEP_DOWN; // for theoretical sprint stepdown anims, not in default anims
					if (GetChange(item, &g_Level.Anims[item->animNumber]))
						item->pos.yPos += coll->midFloor; // move Lara to midFloor
					else
						item->pos.yPos += 50; // do the default aligment
				}
			}
		}
	}
}

void lara_as_dash(ITEM_INFO* item, COLL_INFO* coll)//15A28, 15B5C (F)
{
	if (item->hitPoints <= 0 || !DashTimer || !(TrInput & IN_SPRINT) || Lara.waterStatus == LW_WADE)
	{
		item->goalAnimState = LS_RUN_FORWARD;
		return;
	}

	DashTimer--;

	if (TrInput & IN_DUCK
		&& (Lara.gunStatus == LG_NO_ARMS
			|| Lara.gunType == WEAPON_NONE
			|| Lara.gunType == WEAPON_PISTOLS
			|| Lara.gunType == WEAPON_REVOLVER
			|| Lara.gunType == WEAPON_UZI
			|| Lara.gunType == WEAPON_FLARE))
	{
		item->goalAnimState = LS_CROUCH_IDLE;
		return;
	}

	if (TrInput & IN_LEFT)
	{
		Lara.turnRate -= LARA_TURN_RATE;
		if (Lara.turnRate < -ANGLE(4.0f))
			Lara.turnRate = -ANGLE(4.0f);

		item->pos.zRot -= ANGLE(1.5f);
		if (item->pos.zRot < -ANGLE(16.0f))
			item->pos.zRot = -ANGLE(16.0f);
	}
	else if (TrInput & IN_RIGHT)
	{
		Lara.turnRate += LARA_TURN_RATE;
		if (Lara.turnRate > ANGLE(4.0f))
			Lara.turnRate = ANGLE(4.0f);

		item->pos.zRot += ANGLE(1.5f);
		if (item->pos.zRot > ANGLE(16.0f))
			item->pos.zRot = ANGLE(16.0f);
	}

	if (!(TrInput & IN_JUMP) || item->gravityStatus)
	{
		if (TrInput & IN_FORWARD)
		{
			if (TrInput & IN_WALK)
				item->goalAnimState = LS_WALK_FORWARD;
			else
				item->goalAnimState = LS_SPRINT;
		}
		else if (!(TrInput & IN_LEFT) && !(TrInput & IN_RIGHT))
		{
			item->goalAnimState = LS_STOP;
		}
	}
	else
	{
		item->goalAnimState = LS_SPRINT_ROLL;
	}
}
