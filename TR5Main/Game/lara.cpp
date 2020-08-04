#include "framework.h"
#include "Lara.h"
#include "control.h"
#include "items.h"
#include "inventory.h"
#include "larafire.h"
#include "misc.h"
#include "draw.h"
#include "sphere.h"
#include "camera.h"
#include "larasurf.h"
#include "laraswim.h"
#include "lara1gun.h"
#include "lara2gun.h"
#include "laraflar.h"
#include "laramisc.h"
#include "laraclmb.h"
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
static short LeftClimbTab[4] = // offset 0xA0638
{
	0x0200, 0x0400, 0x0800, 0x0100
};

static short RightClimbTab[4] = // offset 0xA0640
{
	0x0800, 0x0100, 0x0200, 0x0400
};

extern Inventory g_Inventory;

short elevation = 57346;
bool doJump = false;
short OldAngle = 1;
int RopeSwing = 0;
LaraInfo Lara;
ITEM_INFO* LaraItem;
byte LaraNodeUnderwater[NUM_LARA_MESHES];
bool EnableCrouchRoll, EnableFeetHang, EnableMonkeyVault, EnableMonkeyRoll, EnableCrawlFlex1click, EnableCrawlFlex2click, EnableCrawlFlex3click;
bool EnableCrawlFlex1clickE, EnableCrawlFlex2clickE, EnableCrawlFlex3clickE, EnableCrawlFlex1clickup, EnableCrawlFlex1clickdown;
bool TR12_OSCILLATE_HANG;

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
	lara_as_slide,
	lara_as_backjump,
	lara_as_rightjump,
	lara_as_leftjump,
	lara_as_upjump,
	lara_as_fallback,
	lara_as_hangleft,
	lara_as_hangright,
	lara_as_slideback,
	lara_as_surftread,
	lara_as_surfswim,
	lara_as_dive,
	lara_as_pushblock,
	lara_as_pullblock,
	lara_as_ppready,
	lara_as_pickup,
	lara_as_switchon,
	lara_as_switchoff,
	lara_as_usekey,
	lara_as_usepuzzle,
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
	lara_as_deathslide,
	lara_as_duck,
	lara_as_crouch_roll,
	lara_as_dash,
	lara_as_dashdive,
	lara_as_hang2,
	lara_as_monkeyswing,
	lara_as_monkeyl,
	lara_as_monkeyr,
	lara_as_monkey180,
	lara_as_all4s,
	lara_as_crawl,
	lara_as_hangturnl,
	lara_as_hangturnr,
	lara_as_all4turnl,
	lara_as_all4turnr,
	lara_as_crawlb,
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
	lara_as_pickup,
	lara_as_null,
	lara_as_null,
	lara_as_null,
	lara_as_poleleft,
	lara_as_poleright,
	lara_as_pulley,
	lara_as_duckl,
	lara_as_duckr,
	lara_as_extcornerl,
	lara_as_extcornerr,
	lara_as_intcornerl,
	lara_as_intcornerr,
	lara_as_rope,
	lara_as_climbrope,
	lara_as_climbroped,
	lara_as_rope,
	lara_as_rope,
	lara_void_func,
	lara_as_controlled,
	lara_as_swimcheat,
	lara_as_trpose,
	lara_as_null,
	lara_as_trwalk,
	lara_as_trfall,
	lara_as_trfall,
	lara_as_null,
	lara_as_null,
	lara_as_switchon,
	lara_as_null,
	lara_as_parallelbars,
	lara_as_pbleapoff,
	lara_as_null,
	lara_as_null,
	lara_as_null,
	lara_as_null,
	lara_as_null,
	lara_as_null,
	lara_as_null,
	lara_as_null,
	lara_as_null,
	lara_as_hang_feet,
	lara_as_hang_feet_shimmyr,
	lara_as_hang_feet_shimmyl,
	lara_as_hang_feet_inRcorner,
	lara_as_hang_feet_inLcorner,
	lara_as_hang_feet_outRcorner,
	lara_as_hang_feet_outLcorner,
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
	lara_col_duck,
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
/*function<LaraRoutineFunction> lara_camera_routines[NUM_LARA_STATES + 1] = {

};*/

void LaraAboveWater(ITEM_INFO* item, COLL_INFO* coll)
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

	UpdateLaraRoom(item, -LARA_HITE/2);

	//if (Lara.gunType == WEAPON_CROSSBOW && !LaserSight)
	//	TrInput &= ~IN_ACTION;

	// Handle weapons
	LaraGun();

	// Test if there's a trigger
	TestTriggers(coll->trigger, FALSE, 0);
}

int UseSpecialItem(ITEM_INFO* item)
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
				Lara.moveAngle = item->pos.yRot;
				coll->badPos = NO_BAD_POS;
				coll->badNeg = -STEPUP_HEIGHT;
				coll->badCeiling = 0;
				coll->slopesAreWalls = true;
				coll->radius = LARA_RAD + 2;

				GetLaraCollisionInfo(item, coll);
				if (TestLaraVault(item, coll))
					return;

				coll->radius = LARA_RAD;
			}
		}
		else if (TrInput & IN_BACK)
		{
			if ((rheight < (STEPUP_HEIGHT-1)) && (rheight > -(STEPUP_HEIGHT-1)))
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

			height = LaraFloorFront(item, item->pos.yRot, LARA_RAD+4);
			ceiling = LaraCeilingFront(item, item->pos.yRot, LARA_RAD+4, LARA_HITE);

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
				Lara.moveAngle = item->pos.yRot;

				coll->badPos = NO_BAD_POS;
				coll->badNeg = -STEPUP_HEIGHT;
				coll->badCeiling = 0;
				coll->radius = LARA_RAD + 2;
				coll->slopesAreWalls = true;

				GetLaraCollisionInfo(item, coll);

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
				if ((rheight < (STEPUP_HEIGHT-1)) && (rheight > -(STEPUP_HEIGHT-1)) && HeightType != BIG_SLOPE)
					lara_as_back(item, coll);
			}
			else if (rheight > -(STEPUP_HEIGHT-1))
			{
				item->goalAnimState = LS_HOP_BACK;
			}
		}
	}
}

void lara_as_pbleapoff(ITEM_INFO* item, COLL_INFO* coll)//1D244, 1D3D8 (F)
{
	ITEM_INFO* pitem = (ITEM_INFO*)Lara.generalPtr;

	item->gravityStatus = true;

	if (item->frameNumber == g_Level.Anims[item->animNumber].frameBase)
	{
		int dist;

		if (item->pos.yRot == pitem->pos.yRot)
		{
			dist = pitem->triggerFlags / 100 - 2;
		}
		else
		{
			dist = pitem->triggerFlags % 100 - 2;
		}

		item->fallspeed = -(20 * dist + 64);
		item->speed = 20 * dist + 58;
	}

	if (item->frameNumber == g_Level.Anims[item->animNumber].frameEnd)
	{
		item->pos.xPos += 700 * phd_sin(item->pos.yRot) >> W2V_SHIFT;
		item->pos.yPos -= 361;
		item->pos.zPos += 700 * phd_cos(item->pos.yRot) >> W2V_SHIFT;

		item->animNumber = LA_REACH;
		item->frameNumber = g_Level.Anims[item->animNumber].frameBase;

		item->goalAnimState = LS_REACH;
		item->currentAnimState = LS_REACH;
	}
}

void lara_as_parallelbars(ITEM_INFO* item, COLL_INFO* coll) 
{
	if (!(TrInput & IN_ACTION))
	{
		item->goalAnimState = LS_BARS_JUMP;
	}
}

void lara_as_trfall(ITEM_INFO* item, COLL_INFO* coll) 
{
	if (item->animNumber == LA_TIGHTROPE_FALL_LEFT || item->animNumber == LA_TIGHTROPE_FALL_RIGHT)
	{
		if (item->frameNumber == g_Level.Anims[item->animNumber].frameEnd)
		{
			PHD_VECTOR pos;
			pos.x = 0;
			pos.y = 0;
			pos.z = 0;

			GetLaraJointPosition(&pos, LM_RFOOT);

			item->pos.xPos = pos.x;
			item->pos.yPos = pos.y + 75;
			item->pos.zPos = pos.z;

			item->goalAnimState = LS_FREEFALL;
			item->currentAnimState = LS_FREEFALL;

			item->animNumber = LA_FREEFALL;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;

			item->fallspeed = 81;
			Camera.targetspeed = 16;
		}
	}
	else
	{
		int undoInp, wrongInput;
		int undoAnim, undoFrame;

		if (Lara.tightRopeOnCount > 0)
			Lara.tightRopeOnCount--;

		if (item->animNumber == LA_TIGHTROPE_UNBALANCE_LEFT)
		{
			undoInp = IN_RIGHT;
			wrongInput = IN_LEFT;
			undoAnim = LA_TIGHTROPE_RECOVER_LEFT;
		}
		else if (item->animNumber == LA_TIGHTROPE_UNBALANCE_RIGHT)
		{
			undoInp = IN_LEFT;
			wrongInput = IN_RIGHT;
			undoAnim = LA_TIGHTROPE_RECOVER_RIGHT;
		}
		else
		{
			return;
		}

		undoFrame = g_Level.Anims[item->animNumber].frameEnd + g_Level.Anims[undoAnim].frameBase - item->frameNumber;

		if (TrInput & undoInp && Lara.tightRopeOnCount == 0)
		{
			item->currentAnimState = LS_TIGHTROPE_RECOVER_BALANCE;
			item->goalAnimState = LS_TIGHTROPE_IDLE;
			item->animNumber = undoAnim;
			item->frameNumber = undoFrame;

			Lara.tightRopeFall--;
		}
		else
		{
			if (TrInput & wrongInput)
			{
				if (Lara.tightRopeOnCount < 10)
					Lara.tightRopeOnCount += (GetRandomControl() & 3) + 2;
			}
		}
	}
}

void lara_as_trwalk(ITEM_INFO* item, COLL_INFO* coll)
{
	if (Lara.tightRopeOnCount)
	{
		Lara.tightRopeOnCount--;
	}
	else if (Lara.tightRopeOff)
	{
		short roomNumber = item->roomNumber;

		if (GetFloorHeight(GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber),
			item->pos.xPos, item->pos.yPos, item->pos.zPos) == item->pos.yPos)
		{
			Lara.tightRopeOff = 0;
			item->goalAnimState = LS_TIGHTROPE_EXIT;
		}
	}
	else
	{
		GetTighRopeFallOff(127);
	}

	if (LaraItem->currentAnimState != LS_TIGHTROPE_UNBALANCE_LEFT)
	{
		if (TrInput & IN_LOOK)
		{
			LookUpDown();
		}

		if (item->goalAnimState != LS_TIGHTROPE_EXIT &&
			(Lara.tightRopeFall
				|| (TrInput & IN_BACK || TrInput & IN_ROLL || !(TrInput & IN_FORWARD)) && !Lara.tightRopeOnCount && !Lara.tightRopeOff))
		{
			item->goalAnimState = LS_TIGHTROPE_IDLE;
		}
	}
}

void lara_as_trpose(ITEM_INFO* item, COLL_INFO* coll) 
{
	if (TrInput & IN_LOOK)
		LookUpDown();

	GetTighRopeFallOff(127);
	if (LaraItem->currentAnimState != LS_TIGHTROPE_UNBALANCE_LEFT)
	{
		if (Lara.tightRopeFall)
		{
			if (GetRandomControl() & 1)
				item->goalAnimState = LS_TIGHTROPE_UNBALANCE_RIGHT;
			else
				item->goalAnimState = LS_TIGHTROPE_UNBALANCE_LEFT;
		}
		else
		{
			if (TrInput & IN_FORWARD)
			{
				item->goalAnimState = LS_TIGHTROPE_FORWARD;
			}
			else if ((TrInput & IN_ROLL) || (TrInput & IN_BACK))
			{
				if (item->animNumber == LA_TIGHTROPE_IDLE)
				{
					item->currentAnimState = LS_TIGHTROPE_TURN_180;
					item->animNumber = LA_TIGHTROPE_TURN_180;
					item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
					GetTighRopeFallOff(1);
				}
			}
		}
	}
}

void GetTighRopeFallOff(int regularity) 
{
	if (LaraItem->hitPoints <= 0 || LaraItem->hitStatus)
	{
		LaraItem->goalAnimState = LS_TIGHTROPE_UNBALANCE_LEFT;
		LaraItem->currentAnimState = LS_TIGHTROPE_UNBALANCE_LEFT;
		LaraItem->animNumber = LA_TIGHTROPE_FALL_LEFT;
		LaraItem->frameNumber = g_Level.Anims[LaraItem->animNumber].frameBase;
	}

	if (!Lara.tightRopeFall && !(GetRandomControl() & regularity))
		Lara.tightRopeFall = 2 - ((GetRandomControl() & 0xF) != 0);
}

void LookLeftRight() 
{
	Camera.type = LOOK_CAMERA;
	if (TrInput & IN_LEFT)
	{
		TrInput &= ~IN_LEFT;
		if (Lara.headYrot > -ANGLE(44.0f))
		{
			if (BinocularRange)
				Lara.headYrot += ANGLE(2.0f) * (BinocularRange - 1792) / 1536;
			else
				Lara.headYrot -= ANGLE(2.0f);
		}
	}
	else if (TrInput & IN_RIGHT)
	{
		TrInput &= ~IN_RIGHT;
		if (Lara.headYrot < ANGLE(44.0f))
		{
			if (BinocularRange)
				Lara.headYrot += ANGLE(2.0f) * (1792 - BinocularRange) / 1536;
			else
				Lara.headYrot += ANGLE(2.0f);
		}
	}
	if (Lara.gunStatus != LG_HANDS_BUSY && !Lara.leftArm.lock && !Lara.rightArm.lock)
		Lara.torsoYrot = Lara.headYrot;
}

void LookUpDown() 
{
	Camera.type = LOOK_CAMERA;
	if (TrInput & IN_FORWARD)
	{
		TrInput &= ~IN_FORWARD;
		if (Lara.headXrot > -ANGLE(35.0f))
		{
			if (BinocularRange)
				Lara.headXrot += ANGLE(2.0f) * (BinocularRange - 1792) / 3072;
			else
				Lara.headXrot -= ANGLE(2.0f);
		}
	}
	else if (TrInput & IN_BACK)
	{
		TrInput &= ~IN_BACK;
		if (Lara.headXrot < ANGLE(30.0f))
		{
			if (BinocularRange)
				Lara.headXrot += ANGLE(2.0f) * (1792 - BinocularRange) / 3072;
			else
				Lara.headXrot += ANGLE(2.0f);
		}
	}
	if (Lara.gunStatus != LG_HANDS_BUSY && !Lara.leftArm.lock && !Lara.rightArm.lock)
		Lara.torsoXrot = Lara.headXrot;
}

void ResetLook() 
{
	if (Camera.type != 2)
	{
		if (Lara.headXrot <= -ANGLE(2.0f) || Lara.headXrot >= ANGLE(2.0f))
			Lara.headXrot = Lara.headXrot / -8 + Lara.headXrot;
		else
			Lara.headXrot = 0;

		if (Lara.headYrot <= -ANGLE(2.0f) || Lara.headYrot >= ANGLE(2.0f))
			Lara.headYrot = Lara.headYrot / -8 + Lara.headYrot;
		else
			Lara.headYrot = 0;

		if (Lara.gunStatus == LG_HANDS_BUSY || Lara.leftArm.lock || Lara.rightArm.lock)
		{
			if (!Lara.headXrot)
				Lara.torsoXrot = 0;
			if (!Lara.headYrot)
				Lara.torsoYrot = 0;
		}
		else
		{
			Lara.torsoYrot = Lara.headYrot;
			Lara.torsoXrot = Lara.headXrot;
		}
	}
}

void lara_col_jumper(ITEM_INFO* item, COLL_INFO* coll) 
{
	coll->badPos = NO_BAD_POS;
	coll->badNeg = -STEPUP_HEIGHT;
	coll->badCeiling = BAD_JUMP_CEILING;

	GetLaraCollisionInfo(item, coll);
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
	Lara.moveAngle = item->pos.yRot;
	coll->badPos = 384;
	coll->badNeg = -STEPUP_HEIGHT;
	coll->badCeiling = 0;
	coll->slopesArePits = true;
	coll->slopesAreWalls = true;
	GetLaraCollisionInfo(item, coll);
}

void lara_col_wade(ITEM_INFO* item, COLL_INFO* coll) 
{
	Lara.moveAngle = item->pos.yRot;

	coll->badPos = NO_BAD_POS;
	coll->badNeg = -STEPUP_HEIGHT;
	coll->badCeiling = 0;

	coll->slopesAreWalls = true;

	GetLaraCollisionInfo(item, coll);

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
	Lara.moveAngle = item->pos.yRot;

	coll->badPos = NO_BAD_POS;
	coll->badNeg = -STEPUP_HEIGHT;
	coll->badCeiling = BAD_JUMP_CEILING;

	GetLaraCollisionInfo(item, coll);
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
	Lara.moveAngle = item->pos.yRot;

	coll->badPos = NO_BAD_POS;
	coll->badNeg = -STEPUP_HEIGHT;
	coll->badCeiling = BAD_JUMP_CEILING;

	GetLaraCollisionInfo(item, coll);
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
	Lara.moveAngle = item->pos.yRot - ANGLE(180.0f);

	coll->badPos = NO_BAD_POS;
	coll->badNeg = -STEPUP_HEIGHT;
	coll->badCeiling = 0;
	coll->slopesAreWalls = true;

	item->gravityStatus = false;
	item->fallspeed = 0;

	GetLaraCollisionInfo(item, coll);

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
	Lara.moveAngle = item->pos.yRot;

	coll->badPos = NO_BAD_POS;
	coll->badNeg = -STEPUP_HEIGHT;
	coll->badCeiling = 0;
	coll->slopesArePits = false;
	coll->slopesAreWalls = true;

	item->gravityStatus = false;
	item->fallspeed = 0;

	GetLaraCollisionInfo(item, coll);

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

void lara_col_slideback(ITEM_INFO* item, COLL_INFO* coll)//1C284(<), 1C3B8(<) (F)
{
	Lara.moveAngle = item->pos.yRot - ANGLE(180.0f);
	lara_slide_slope(item, coll);
}

void lara_col_fallback(ITEM_INFO* item, COLL_INFO* coll)//1C1B4(<), 1C2E8(<) (F)
{
	Lara.moveAngle = item->pos.yRot - ANGLE(180.0f);

	coll->badPos = NO_BAD_POS;
	coll->badNeg = -STEPUP_HEIGHT;
	coll->badCeiling = BAD_JUMP_CEILING;

	GetLaraCollisionInfo(item, coll);
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
	Lara.moveAngle = item->pos.yRot - ANGLE(90.0f);
	lara_col_jumper(item, coll);
}

void lara_col_rightjump(ITEM_INFO* item, COLL_INFO* coll)//1C15C(<), 1C290(<) (F)
{
	Lara.moveAngle = item->pos.yRot + ANGLE(90.0f);
	lara_col_jumper(item, coll);
}

void lara_col_backjump(ITEM_INFO* item, COLL_INFO* coll)//1C130(<), 1C264(<) (F)
{
	Lara.moveAngle = item->pos.yRot - ANGLE(180.0f);
	lara_col_jumper(item, coll);
}

void lara_col_slide(ITEM_INFO* item, COLL_INFO* coll)//1C108(<), 1C23C(<) (F)
{
	Lara.moveAngle = item->pos.yRot;
	lara_slide_slope(item, coll);
}

void lara_col_stepleft(ITEM_INFO* item, COLL_INFO* coll)//1C0E8(<), 1C21C(<) (F)
{
	lara_col_stepright(item, coll);
}

void lara_col_stepright(ITEM_INFO* item, COLL_INFO* coll)//1BFB0, 1C0E4 (F)
{
	if (item->currentAnimState == LS_STEP_RIGHT)
		Lara.moveAngle = item->pos.yRot + ANGLE(90.0f);
	else
		Lara.moveAngle = item->pos.yRot - ANGLE(90.0f);

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

	GetLaraCollisionInfo(item, coll);

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
	Lara.moveAngle = item->pos.yRot - ANGLE(180.0f);

	if (Lara.waterStatus == LW_WADE)
		coll->badPos = NO_BAD_POS;
	else
		coll->badPos = STEPUP_HEIGHT;
	coll->badNeg = -STEPUP_HEIGHT;
	coll->badCeiling = 0;
	coll->slopesArePits = true;
	coll->slopesAreWalls = 1;
	GetLaraCollisionInfo(item, coll);

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

	GetLaraCollisionInfo(item, coll);

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
	Lara.moveAngle = item->pos.yRot;

	coll->slopesAreWalls = true;
	coll->slopesArePits = true;

	coll->badPos = 384;
	coll->badNeg = -STEPUP_HEIGHT;
	coll->badCeiling = 0;

	GetLaraCollisionInfo(item, coll);
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

	GetLaraCollisionInfo(item, coll);
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

	Lara.moveAngle = item->pos.yRot;
	coll->badPos = 384;
	coll->badNeg = -STEPUP_HEIGHT;
	coll->badCeiling = 0;
	coll->radius = 400;

	GetLaraCollisionInfo(item, coll);
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
	Lara.moveAngle = item->pos.yRot;
	coll->badPos = STEPUP_HEIGHT;
	coll->badNeg = -STEPUP_HEIGHT;
	coll->badCeiling = 0;
	coll->slopesAreWalls = 1;
	coll->slopesArePits = true;
	GetLaraCollisionInfo(item, coll);

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

	Lara.moveAngle = item->pos.yRot - ANGLE(180.0f);

	coll->slopesAreWalls = 0;
	coll->slopesArePits = true;

	coll->badPos = NO_BAD_POS;
	coll->badNeg = -STEPUP_HEIGHT;
	coll->badCeiling = 0;

	GetLaraCollisionInfo(item, coll);

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
	Lara.moveAngle = item->pos.yRot;

	coll->badPos = NO_BAD_POS;
	coll->badNeg = -STEPUP_HEIGHT;
	coll->badCeiling = 0;

	coll->slopesAreWalls = true;

	GetLaraCollisionInfo(item, coll);

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

	Lara.moveAngle = item->pos.yRot;

	coll->badPos = 384;
	coll->badNeg = -STEPUP_HEIGHT;
	coll->badCeiling = 0;

	coll->slopesAreWalls = true;
	coll->slopesArePits = true;
	coll->lavaIsPit = 1;

	GetLaraCollisionInfo(item, coll);

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

void lara_as_pulley(ITEM_INFO* item, COLL_INFO* coll)//1B288, 1B3BC (F)
{
	ITEM_INFO* p = (ITEM_INFO*) Lara.generalPtr;

	Lara.look = false;

	coll->enableSpaz = false;
	coll->enableBaddiePush = false;

	if (TrInput & IN_ACTION && p->triggerFlags)
	{
		item->goalAnimState = LS_PULLEY;
	}
	else
	{
		item->goalAnimState = LS_STOP;
	}

	if (item->animNumber == LA_PULLEY_PULL && item->frameNumber == g_Level.Anims[LA_PULLEY_PULL].frameBase + 44)
	{
		if (p->triggerFlags)
		{
			p->triggerFlags--;

			if (p->triggerFlags)
			{
				if (p->itemFlags[2])
				{
					p->itemFlags[2] = 0;
					p->status = ITEM_DEACTIVATED;
				}
			}
			else
			{
				if (!p->itemFlags[1])
					p->status = ITEM_DEACTIVATED;

				p->itemFlags[2] = 1;

				if (p->itemFlags[3] >= 0)
					p->triggerFlags = abs(p->itemFlags[3]);
				else
					p->itemFlags[0] = 1;
			}
		}
	}

	if (item->animNumber == LA_PULLEY_RELEASE && item->frameNumber == g_Level.Anims[item->animNumber].frameEnd - 1)
		Lara.gunStatus = LG_NO_ARMS;
}

void lara_col_turnswitch(ITEM_INFO* item, COLL_INFO* coll)//1B1B4(<), 1B2E8(<) (F)
{
	if (coll->old.x != item->pos.xPos || coll->old.z != item->pos.zPos)
	{
		if (item->animNumber == LA_TURNSWITCH_PUSH_COUNTER_CLOCKWISE_CONTINUE)
		{
			item->pos.yRot -= ANGLE(90.0f);

			item->animNumber = LA_TURNSWITCH_PUSH_COUNTER_CLOCKWISE_END;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
		}

		if (item->animNumber == LA_TURNSWITCH_PUSH_CLOCKWISE_CONTINUE)
		{
			item->pos.yRot += ANGLE(90.0f);

			item->animNumber = LA_TURNSWITCH_PUSH_CLOCKWISE_END;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
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

void lara_as_deathslide(ITEM_INFO* item, COLL_INFO* coll)//1B038, 1B16C (F)
{
	short roomNumber = item->roomNumber;

	Camera.targetAngle = ANGLE(70.0f);

	GetFloorHeight(GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber),
		item->pos.xPos, item->pos.yPos, item->pos.zPos);

	coll->trigger = TriggerIndex;

	if (!(TrInput & IN_ACTION))
	{
		item->goalAnimState = LS_JUMP_FORWARD;

		AnimateLara(item);

		LaraItem->gravityStatus = true;
		LaraItem->speed = 100;
		LaraItem->fallspeed = 40;

		Lara.moveAngle = item->pos.yRot;
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

void lara_as_usepuzzle(ITEM_INFO* item, COLL_INFO* coll)//1AD18(<), 1AE4C(<) (F)
{
	Lara.look = false;

	coll->enableBaddiePush = false;
	coll->enableSpaz = false;

	Camera.targetAngle = -ANGLE(80.0f);
	Camera.targetElevation = -ANGLE(25.0f);
	Camera.targetDistance = SECTOR(1);

	if (item->frameNumber == g_Level.Anims[item->animNumber].frameEnd)
	{
		if (item->itemFlags[0])
		{
			item->animNumber = item->itemFlags[0];
			item->currentAnimState = LS_MISC_CONTROL;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
		}
	}
}

void lara_as_usekey(ITEM_INFO* item, COLL_INFO* coll)//1ACBC(<), 1ADF0(<) (F)
{
	Lara.look = false;
	coll->enableBaddiePush = false;
	coll->enableSpaz = false;
	Camera.targetAngle = -ANGLE(80.0f);
	Camera.targetElevation = -4550;
	Camera.targetDistance = SECTOR(1);
}

void lara_as_switchoff(ITEM_INFO* item, COLL_INFO* coll)//1AC54(<), 1AD88(<) (F)
{
	Lara.look = false;
	coll->enableBaddiePush = false;
	coll->enableSpaz = false;
	Camera.targetAngle = ANGLE(80.0f);
	Camera.targetElevation = -ANGLE(25.0f);
	Camera.targetDistance = SECTOR(1);
	Camera.speed = 6;
}

void lara_as_switchon(ITEM_INFO* item, COLL_INFO* coll)//1ABEC(<), 1AD20(<) (F)
{
	Lara.look = false;
	coll->enableBaddiePush = false;
	coll->enableSpaz = false;
	Camera.targetAngle = ANGLE(80.0f);
	Camera.targetElevation = -ANGLE(25.0f);
	Camera.targetDistance = SECTOR(1);
	Camera.speed = 6;
}

void lara_as_pickupflare(ITEM_INFO* item, COLL_INFO* coll)//1AB5C(<), 1AC90(<) (F)
{
	Lara.look = false;
	coll->enableBaddiePush = false;
	coll->enableSpaz = false;
	Camera.targetAngle = ANGLE(130.0f);
	Camera.targetElevation = -ANGLE(15.0f);
	Camera.targetDistance = SECTOR(1);
	if (item->frameNumber == g_Level.Anims[item->animNumber].frameEnd - 1)
		Lara.gunStatus = LG_NO_ARMS;
}

void lara_as_pickup(ITEM_INFO* item, COLL_INFO* coll)//1AB00(<), 1AC34(<) (F)
{
	Lara.look = false;
	coll->enableBaddiePush = false;
	coll->enableSpaz = false;
	Camera.targetAngle = -ANGLE(130.0f);
	Camera.targetElevation = -ANGLE(15.0f);
	Camera.targetDistance = SECTOR(1);
}

void lara_as_ppready(ITEM_INFO* item, COLL_INFO* coll)//1AABC(<), 1ABF0(<) (F)
{
	coll->enableBaddiePush = false;
	coll->enableSpaz = false;
	Camera.targetAngle = ANGLE(75.0f);
	if (!(TrInput & IN_ACTION))
		item->goalAnimState = LS_STOP;
}

void lara_as_pullblock(ITEM_INFO* item, COLL_INFO* coll)//1AA60(<), 1AB94(<) (F)
{
	Lara.look = false;
	coll->enableBaddiePush = false;
	coll->enableSpaz = false;
	Camera.flags = CF_FOLLOW_CENTER;
	Camera.targetAngle = ANGLE(35.0f);
	Camera.targetElevation = -ANGLE(25.0f);
}

void lara_as_pushblock(ITEM_INFO* item, COLL_INFO* coll)//1AA04(<), 1AB38(<) (F)
{
	Lara.look = false;
	coll->enableBaddiePush = false;
	coll->enableSpaz = false;
	Camera.flags = CF_FOLLOW_CENTER;
	Camera.targetAngle = ANGLE(90.0f);
	Camera.targetElevation = -ANGLE(25.0f);
}

void lara_as_slideback(ITEM_INFO* item, COLL_INFO* coll)//1A9E0(<), 1AB14(<) (F)
{
	if ((TrInput & IN_JUMP) && !(TrInput & IN_FORWARD))
	{
		item->goalAnimState = LS_JUMP_BACK;
	}
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

void lara_as_slide(ITEM_INFO* item, COLL_INFO* coll)//1A824(<), 1A958(<) (F)
{
	Camera.targetElevation = -ANGLE(45.0f); // FIXED
	if ((TrInput & IN_JUMP) && !(TrInput & IN_BACK))
		item->goalAnimState = LS_JUMP_FORWARD;
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
			Lara.moveAngle = item->pos.yRot;
		}
		else if (TrInput & IN_LEFT && LaraFloorFront(item, item->pos.yRot - ANGLE(90.0f), 256) >= -384)
		{
			item->goalAnimState = LS_JUMP_RIGHT;
			Lara.moveAngle = item->pos.yRot - ANGLE(90.0f);
		}
		else if (TrInput & IN_RIGHT && LaraFloorFront(item, item->pos.yRot + ANGLE(90.0f), 256) >= -384)
		{
			item->goalAnimState = LS_JUMP_LEFT;
			Lara.moveAngle = item->pos.yRot + ANGLE(90.0f);
		}
		else if (TrInput & IN_BACK && LaraFloorFront(item, item->pos.yRot - ANGLE(180.0f), 256) >= -384)
		{
			item->goalAnimState = LS_JUMP_BACK;
			Lara.moveAngle = item->pos.yRot - ANGLE(180.0f);
		}
	}

	if (item->fallspeed > LARA_FREEFALL_SPEED)
		item->goalAnimState = LS_FREEFALL;
}

void lara_as_splat(ITEM_INFO* item, COLL_INFO* coll)//1A340(<), 1A474(<) (F)
{
	Lara.look = false;
}

void lara_as_intcornerr(ITEM_INFO* item, COLL_INFO* coll)//1A2EC(<), 1A420(<) (F)
{
	Camera.laraNode = 8;
	Camera.targetElevation = ANGLE(33.0f);
	SetCornerAnim(item, coll, ANGLE(90.0f),
		item->animNumber == LA_SHIMMY_RIGHT_CORNER_INNER_END ||
		item->animNumber == LA_LADDER_RIGHT_CORNER_INNER_END);
}

void lara_as_intcornerl(ITEM_INFO* item, COLL_INFO* coll)//1A298(<), 1A3CC(<) (F)
{
	Camera.laraNode = 8;
	Camera.targetElevation = ANGLE(33.0f);
	SetCornerAnim(item, coll, -ANGLE(90.0f),
		item->animNumber == LA_SHIMMY_LEFT_CORNER_INNER_END ||
		item->animNumber == LA_LADDER_LEFT_CORNER_INNER_END);
}

void lara_as_extcornerr(ITEM_INFO* item, COLL_INFO* coll)//1A244(<), 1A378(<) (F)
{
	Camera.laraNode = 8;
	Camera.targetElevation = ANGLE(33.0f);
	SetCornerAnim(item, coll, -ANGLE(90.0f),
		item->animNumber == LA_SHIMMY_RIGHT_CORNER_OUTER_END ||
		item->animNumber == LA_LADDER_RIGHT_CORNER_OUTER_END);
}

void lara_as_extcornerl(ITEM_INFO* item, COLL_INFO* coll)//1A1F0(<), 1A324(<) (F)
{
	Camera.laraNode = 8;
	Camera.targetElevation = ANGLE(33.0f);
	SetCornerAnim(item, coll, ANGLE(90.0f),
		item->animNumber == LA_SHIMMY_LEFT_CORNER_OUTER_END ||
		item->animNumber == LA_LADDER_LEFT_CORNER_OUTER_END);
}

void SetCornerAnim(ITEM_INFO* item, COLL_INFO* coll, short rot, short flip)//1A090, 1A1C4 (F)
{
	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_JUMP_FORWARD;
		item->currentAnimState = LS_JUMP_FORWARD;
		item->animNumber = LA_FALL_START;
		item->frameNumber = g_Level.Anims[item->animNumber].frameBase;

		item->gravityStatus = true;
		item->speed = 2;
		item->pos.yPos += 256;
		item->fallspeed = 1;

		Lara.gunStatus = LG_NO_ARMS;

		item->pos.yRot += rot / 2;
	}
	else if (flip)
	{
		if (Lara.isClimbing)
		{
			item->animNumber = LA_LADDER_IDLE;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			item->goalAnimState = LS_LADDER_IDLE;
			item->currentAnimState = LS_LADDER_IDLE;
		}
		else
		{
			item->animNumber = LA_REACH_TO_HANG;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase + 21;
			item->goalAnimState = LS_HANG;
			item->currentAnimState = LS_HANG;
		}

		coll->old.x = Lara.cornerX;
		item->pos.xPos = Lara.cornerX;

		coll->old.z = Lara.cornerZ;
		item->pos.zPos = Lara.cornerZ;

		item->pos.yRot += rot;
	}
}

void lara_col_hangright(ITEM_INFO* item, COLL_INFO* coll) // (F) (D)
{
	Lara.moveAngle = item->pos.yRot + ANGLE(90.0f);
	coll->radius = 102; /* @ORIGINAL_BUG: this value (instead of LARA_RAD) can make Lara glitch if coll->frontType is DIAGONAL or SPLIT_TRI */
	LaraHangTest(item, coll);
	Lara.moveAngle = item->pos.yRot + ANGLE(90.0f);
}

void lara_as_hangright(ITEM_INFO* item, COLL_INFO* coll) // (F) (D)
{
	coll->enableBaddiePush = false;
	coll->enableSpaz = false;
	Camera.targetAngle = 0;
	Camera.targetElevation = -ANGLE(45.0f);
	if (!(TrInput & (IN_RIGHT | IN_RSTEP)))
		item->goalAnimState = LS_HANG;
}

void lara_col_hangleft(ITEM_INFO* item, COLL_INFO* coll) // (F) (D)
{
	Lara.moveAngle = item->pos.yRot - ANGLE(90.0f);
	coll->radius = 102; /* @ORIGINAL_BUG: this value (instead of LARA_RAD) can make Lara glitch if coll->frontType is DIAGONAL or SPLIT_TRI */
	LaraHangTest(item, coll);
	Lara.moveAngle = item->pos.yRot - ANGLE(90.0f);
}

void lara_as_hangleft(ITEM_INFO* item, COLL_INFO* coll) // (F) (D)
{
	coll->enableBaddiePush = false;
	coll->enableSpaz = false;
	Camera.targetAngle = 0;
	Camera.targetElevation = -ANGLE(45.0f);
	if (!(TrInput & (IN_LEFT | IN_LSTEP)))
		item->goalAnimState = LS_HANG;
}

void lara_col_hang(ITEM_INFO* item, COLL_INFO* coll)//19AC8, 19BFC (F)
{
	item->fallspeed = 0;
	item->gravityStatus = false;

	if (item->animNumber == LA_REACH_TO_HANG)
	{
		int flag;

		if (TrInput & IN_LEFT || TrInput & IN_LSTEP)
		{
			if (CanLaraHangSideways(item, coll, -ANGLE(90.0f)))
			{
				item->goalAnimState = LS_SHIMMY_LEFT;

				return;
			}

			flag = LaraHangLeftCornerTest(item, coll);
			if (flag != 0)
			{
				if (flag <= 0)
					item->goalAnimState = LS_SHIMMY_INNER_LEFT;
				else
					item->goalAnimState = LS_SHIMMY_OUTER_LEFT;

				return;
			}
		}

		if (TrInput & IN_RIGHT || TrInput & IN_RSTEP)
		{
			if (CanLaraHangSideways(item, coll, ANGLE(90.0f)))
			{
				item->goalAnimState = LS_SHIMMY_RIGHT;

				return;
			}

			flag = LaraHangRightCornerTest(item, coll);
			if (flag != 0)
			{
				if (flag <= 0)
					item->goalAnimState = LS_SHIMMY_INNER_RIGHT;
				else
					item->goalAnimState = LS_SHIMMY_OUTER_RIGHT;

				return;
			}
		}
	}

	Lara.moveAngle = item->pos.yRot;

	LaraHangTest(item, coll);

	if (item->animNumber == LA_REACH_TO_HANG)
	{
		TestForObjectOnLedge(item, coll);

		if (TrInput & IN_FORWARD)
		{
			if (coll->frontFloor > -850)
			{
				if (coll->frontFloor < -650 &&
					coll->frontFloor >= coll->frontCeiling &&
					coll->frontFloor >= coll->leftCeiling2 &&
					coll->frontFloor >= coll->rightCeiling2)
				{
					if (abs(coll->leftFloor2 - coll->rightFloor2) < 60 && !coll->hitStatic)
					{
						if (TrInput & IN_WALK)
						{
							item->goalAnimState = LS_HANDSTAND;
						}
						else if (TrInput & IN_DUCK)
						{
							item->goalAnimState = LS_HANG_TO_CRAWL;
							item->requiredAnimState = LS_CROUCH_IDLE;
						}
						else
						{
							item->goalAnimState = LS_GRABBING;
						}

						return;
					}
				}

				if (coll->frontFloor < -650 &&
					coll->frontFloor - coll->frontCeiling >= -256 &&
					coll->frontFloor - coll->leftCeiling2 >= -256 &&
					coll->frontFloor - coll->rightCeiling2 >= -256)
				{
					if (abs(coll->leftFloor2 - coll->rightFloor2) < 60 && !coll->hitStatic)
					{
						item->goalAnimState = LS_HANG_TO_CRAWL;
						item->requiredAnimState = LS_CROUCH_IDLE;

						return;
					}
				}
			}

			if (Lara.climbStatus != 0 &&
				coll->midCeiling <= -256 &&
				abs(coll->leftCeiling2 - coll->rightCeiling2) < 60)
			{
				if (LaraTestClimbStance(item, coll))
				{
					item->goalAnimState = LS_LADDER_IDLE;
				}
				else
				{
					item->animNumber = LA_LADDER_SHIMMY_UP;
					item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
					item->goalAnimState = LS_HANG;
					item->currentAnimState = LS_HANG;
				}
			}

			return;
		}

		if (TrInput & IN_BACK &&
			Lara.climbStatus &&
			coll->midFloor > 344 && 
			item->animNumber == LA_REACH_TO_HANG)
		{
			if (LaraTestClimbStance(item, coll))
			{
				item->goalAnimState = LS_LADDER_IDLE;
			}
			else
			{
				item->animNumber = LA_LADDER_SHIMMY_DOWN;
				item->goalAnimState = LS_HANG;
				item->currentAnimState = LS_HANG;
				item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			}
		}
	}
}

void lara_as_hang(ITEM_INFO* item, COLL_INFO* coll)//19A28, 19B5C (F)
{
	Lara.isClimbing = false;

	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_STOP;
		return;
	}

	if (TrInput & IN_LOOK)
		LookUpDown();

	coll->enableBaddiePush = false;
	coll->enableSpaz = false;

	Camera.targetAngle = 0;
	Camera.targetElevation = -ANGLE(45.0f);
}

int CanLaraHangSideways(ITEM_INFO* item, COLL_INFO* coll, short angle)//19930, 19A64 (F)
{
	int oldx = item->pos.xPos;
	int oldz = item->pos.zPos;
	int x = item->pos.xPos;
	int z = item->pos.zPos;
	int res;

	Lara.moveAngle = angle + item->pos.yRot;
	short ang = (unsigned short) (Lara.moveAngle + ANGLE(45.0f)) >> W2V_SHIFT;


		switch (ang)
		{
		case 0:
			z += 16;
			break;
		case 1:
			x += 16;
			break;
		case 2:
			z -= 16;
			break;
		case 3:
			x -= 16;
			break;
		}

	item->pos.xPos = x;
	item->pos.zPos = z;

	coll->old.y = item->pos.yPos;

	res = LaraHangTest(item, coll);

	item->pos.xPos = oldx;
	item->pos.zPos = oldz;

	Lara.moveAngle = angle + item->pos.yRot;

	return !res;
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




	Lara.moveAngle = item->pos.yRot;

	coll->badPos = NO_BAD_POS;
	coll->badNeg = 0;
	coll->badCeiling = BAD_JUMP_CEILING;

	GetLaraCollisionInfo(item, coll);

	short angle = 1;
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
				if (abs(angle) > ANGLE(35.0f))
				{
					if (angle < 10014 || angle > 22754)
					{
						if (angle >= 26397 || angle <= -26397)
						{
							angle = -ANGLE(180.0f);
						}
						else if (angle >= -22754 && angle <= -10014)
						{
							angle = -ANGLE(90.0f);
						}
					}
					else
					{
						angle = ANGLE(90.0f);
					}
				}
				else
				{
					angle = 0;
				}
			}
		}
	}

	if (angle & 0x3FFF)
	{
		LaraSlideEdgeJump(item, coll);
		GetLaraCollisionInfo(item, coll);
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
			if (TR12_OSCILLATE_HANG == true)
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
			{
				Lara.headYrot = 0;
				Lara.headXrot = 0;
				Lara.torsoYrot = 0;
				Lara.torsoXrot = 0;
				item->animNumber = LA_REACH_TO_MONKEYSWING;
				item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
				item->currentAnimState = LS_MONKEYSWING_IDLE;
				item->goalAnimState = LS_MONKEYSWING_IDLE;
			}
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

			short dir = (unsigned short) (item->pos.yRot + ANGLE(45.0f)) / ANGLE(90.0f);
			switch (dir)
			{
			case NORTH:
				item->pos.zPos = (item->pos.zPos | 0x3FF) - 100;
				item->pos.xPos += coll->shift.x;
				break;
			case SOUTH:
				item->pos.zPos = (item->pos.zPos & 0xFFFFFC00) + 100;
				item->pos.xPos += coll->shift.x;
				break;
			case EAST:
				item->pos.xPos = (item->pos.xPos | 0x3FF) - 100;
				item->pos.zPos += coll->shift.z;
				break;
			case WEST:
				item->pos.xPos = (item->pos.xPos & 0xFFFFFC00) + 100;
				item->pos.zPos += coll->shift.z;
				break;
			default:
				break;
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
		Lara.moveAngle = item->pos.yRot - ANGLE(180.0f);
	else
		Lara.moveAngle = item->pos.yRot;

	coll->badPos = NO_BAD_POS;
	coll->badNeg = -STEPUP_HEIGHT;
	coll->badCeiling = BAD_JUMP_CEILING;

	GetLaraCollisionInfo(item, coll);
	LaraDeflectEdgeJump(item, coll);

	if (item->speed < 0)
		Lara.moveAngle = item->pos.yRot;

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

	Lara.moveAngle = item->pos.yRot;

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

					if (abs(angle) <= ANGLE(35.0f))
					{
						angle = 0;
					}
					else if (angle >= ANGLE(55.0f) && angle <= ANGLE(125.0f))
					{
						angle = ANGLE(90.0f);
					}
					else if (angle >= ANGLE(145.0f) || angle <= -ANGLE(145.0f))
					{
						angle = ANGLE(180);
					}
					else if (angle >= -ANGLE(125.0f) && angle <= -ANGLE(55.0f))
					{
						angle = -ANGLE(90.0f);
					}

					if ((angle & 0x3FFF) == 0)
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

						item->pos.xPos += coll->shift.x;
						item->pos.zPos += coll->shift.z;
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
	Lara.moveAngle = item->pos.yRot;
	coll->badPos = STEPUP_HEIGHT;
	coll->badNeg = -STEPUP_HEIGHT;
	coll->badCeiling = 0;
	item->gravityStatus = false;
	item->fallspeed = 0;
	coll->slopesArePits = true;
	coll->slopesAreWalls = 1;
	GetLaraCollisionInfo(item, coll);

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

void lara_as_climbroped(ITEM_INFO* item, COLL_INFO* coll) // (F) (D)
{
	LaraClimbRope(item, coll);
}

void lara_as_climbrope(ITEM_INFO* item, COLL_INFO* coll) // (F) (D)
{
	if (TrInput & IN_ROLL)
	{
		FallFromRope(item);
	}
	else
	{
		Camera.targetAngle = ANGLE(30.0f);

		if (g_Level.Anims[item->animNumber].frameEnd == item->frameNumber)
		{
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			Lara.ropeSegment -= 2;
		}

		if (!(TrInput & IN_FORWARD) || Lara.ropeSegment <= 4)
			item->goalAnimState = LS_ROPE_IDLE;
	}
}

void lara_col_ropefwd(ITEM_INFO* item, COLL_INFO* coll) // (F) (D)
{
	Camera.targetDistance = SECTOR(2);

	UpdateRopeSwing(item);

	if (item->animNumber == LA_ROPE_SWING)
	{
		if (TrInput & IN_SPRINT)
		{
			int vel;

			if (abs(Lara.ropeLastX) < 9000)
				vel = 192 * (9000 - abs(Lara.ropeLastX)) / 9000;
			else
				vel = 0;

			ApplyVelocityToRope(Lara.ropeSegment - 2,
				item->pos.yRot + (Lara.ropeDirection ? ANGLE(0.0f) : ANGLE(180.0f)),
				vel >> 5);
		}

		if (Lara.ropeFrame > Lara.ropeDFrame)
		{
			Lara.ropeFrame -= (unsigned short) Lara.ropeFrameRate;
			if (Lara.ropeFrame < Lara.ropeDFrame)
				Lara.ropeFrame = Lara.ropeDFrame;
		}
		else if (Lara.ropeFrame < Lara.ropeDFrame)
		{
			Lara.ropeFrame += (unsigned short) Lara.ropeFrameRate;
			if (Lara.ropeFrame > Lara.ropeDFrame)
				Lara.ropeFrame = Lara.ropeDFrame;
		}

		item->frameNumber = Lara.ropeFrame >> 8;

		if (!(TrInput & IN_SPRINT) &&
			item->frameNumber == g_Level.Anims[LA_ROPE_SWING].frameBase + 32 &&
			Lara.ropeMaxXBackward < 6750 &&
			Lara.ropeMaxXForward < 6750)
		{
			item->animNumber = LA_JUMP_UP_TO_ROPE_END;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;

			item->currentAnimState = LS_ROPE_IDLE;
			item->goalAnimState = LS_ROPE_IDLE;
		}

		if (TrInput & IN_JUMP)
			JumpOffRope(item);
	}
	else if (item->frameNumber == g_Level.Anims[LA_ROPE_IDLE_TO_SWING].frameBase + 15)
	{
		ApplyVelocityToRope(Lara.ropeSegment, item->pos.yRot, 128);
	}
}

void lara_as_roper(ITEM_INFO* item, COLL_INFO* coll) // (F) (D)
{
	if (TrInput & IN_ACTION)
	{
		if (TrInput & IN_RIGHT)
		{
			Lara.ropeY -= 256;
		}
		else
		{
			item->goalAnimState = LS_ROPE_IDLE;
		}
	}
	else
	{
		FallFromRope(item);
	}
}

void lara_as_ropel(ITEM_INFO* item, COLL_INFO* coll) // (F) (D)
{
	if (TrInput & IN_ACTION)
	{
		if (TrInput & IN_LEFT)
		{
			Lara.ropeY += 256;
		}
		else
		{
			item->goalAnimState = LS_ROPE_IDLE;
		}
	}
	else
	{
		FallFromRope(item);
	}
}

void lara_col_rope(ITEM_INFO* item, COLL_INFO* coll) // (F) (D)
{
	if (TrInput & IN_ACTION)
	{
		UpdateRopeSwing(item);

		if (TrInput & IN_SPRINT)
		{
			Lara.ropeDFrame = (g_Level.Anims[LA_ROPE_SWING].frameBase + 32) << 8;
			Lara.ropeFrame = Lara.ropeDFrame;

			item->goalAnimState = LS_ROPE_SWING;
		}
		else if (TrInput & IN_FORWARD && Lara.ropeSegment > 4)
		{
			item->goalAnimState = LS_ROPE_UP;
		}
		else if (TrInput & IN_BACK && Lara.ropeSegment < 21)
		{
			item->goalAnimState = LS_ROPE_DOWN;

			Lara.ropeFlag = 0;
			Lara.ropeCount = 0;
		}
		else if (TrInput & IN_LEFT)
		{
			item->goalAnimState = LS_ROPE_TURN_CLOCKWISE;
		}
		else if (TrInput & IN_RIGHT)
		{
			item->goalAnimState = LS_ROPE_TURN_COUNTER_CLOCKWISE;
		}
	}
	else
	{
		FallFromRope(item);
	}
}

void lara_as_rope(ITEM_INFO* item, COLL_INFO* coll) // (F) (D)
{
	if (!(TrInput & IN_ACTION))
		FallFromRope(item);

	if (TrInput & IN_LOOK)
		LookUpDown();
}

void UpdateRopeSwing(ITEM_INFO* item) // (F) (D)
{
	if (Lara.ropeMaxXForward > 9000)
	{
		Lara.ropeMaxXForward = 9000;
	}
	
	if (Lara.ropeMaxXBackward > 9000)
	{
		Lara.ropeMaxXBackward = 9000;
	}

	if (Lara.ropeDirection)
	{
		if (item->pos.xRot > 0 && item->pos.xRot - Lara.ropeLastX < -100)
		{
			Lara.ropeArcFront = Lara.ropeLastX;
			Lara.ropeDirection = 0;
			Lara.ropeMaxXBackward = 0;
			int frame = 15 * Lara.ropeMaxXForward / 18000 + g_Level.Anims[LA_ROPE_SWING].frameBase + 47 << 8;
			if (frame > Lara.ropeDFrame)
			{
				Lara.ropeDFrame = frame;
				RopeSwing = 1;				
			}
			else
			{
				RopeSwing = 0;
			}
		
			SoundEffect(SFX_LARA_ROPE_CREAK, &item->pos, 0);
		}
		else if (Lara.ropeLastX < 0 && Lara.ropeFrame == Lara.ropeDFrame)
		{
			RopeSwing = 0;
			Lara.ropeDFrame = 15 * Lara.ropeMaxXBackward / 18000 + g_Level.Anims[LA_ROPE_SWING].frameBase + 47 << 8;
			Lara.ropeFrameRate = 15 * Lara.ropeMaxXBackward / 9000 + 1;
		}
		else if (Lara.ropeFrameRate < 512)
		{
			int num = RopeSwing ? 31 : 7;

			Lara.ropeFrameRate += num * Lara.ropeMaxXBackward / 9000 + 1;
		}
	}
	else
	{
		if (item->pos.xRot < 0 && item->pos.xRot - Lara.ropeLastX > 100)
		{
			Lara.ropeArcBack = Lara.ropeLastX;
			Lara.ropeDirection = 1;
			Lara.ropeMaxXForward = 0;
			int frame = g_Level.Anims[LA_ROPE_SWING].frameBase - 15 * Lara.ropeMaxXBackward / 18000 + 17 << 8;
			if (frame < Lara.ropeDFrame)
			{
				Lara.ropeDFrame = frame;
				RopeSwing = 1;
			}
			else
			{
				RopeSwing = 0;
			}

			SoundEffect(SFX_LARA_ROPE_CREAK, &item->pos, 0);
		}
		else if (Lara.ropeLastX > 0 && Lara.ropeFrame == Lara.ropeDFrame)
		{
			RopeSwing = 0;
			
			Lara.ropeDFrame = g_Level.Anims[LA_ROPE_SWING].frameBase - 15 * Lara.ropeMaxXForward / 18000 + 17 << 8;
			Lara.ropeFrameRate = 15 * Lara.ropeMaxXForward / 9000 + 1;
		}
		else if (Lara.ropeFrameRate < 512)
		{
			int num = RopeSwing ? 31 : 7;

			Lara.ropeFrameRate += num * Lara.ropeMaxXForward / 9000 + 1;
		}
	}

	Lara.ropeLastX = item->pos.xRot;
	if (Lara.ropeDirection)
	{
		if (item->pos.xRot > Lara.ropeMaxXForward)
			Lara.ropeMaxXForward = item->pos.xRot;
	}
	else
	{
		if (item->pos.xRot < -Lara.ropeMaxXBackward)
			Lara.ropeMaxXBackward = abs(item->pos.xRot);
	}
}

void JumpOffRope(ITEM_INFO* item) // (F) (D)
{
	if (Lara.ropePtr != -1)
	{
		if (item->pos.xRot >= 0)
		{
			item->fallspeed = -112;
			item->speed = item->pos.xRot / 128;
		}
		else
		{
			item->speed = 0;
			item->fallspeed = -20;
		}

		item->pos.xRot = 0;
		item->gravityStatus = true;

		Lara.gunStatus = LG_NO_ARMS;

		if (item->frameNumber - g_Level.Anims[LA_ROPE_SWING].frameBase > 42)
		{
			item->animNumber = LA_ROPE_SWING_TO_REACH_1;
		}
		else if (item->frameNumber - g_Level.Anims[LA_ROPE_SWING].frameBase > 21)
		{
			item->animNumber = LA_ROPE_SWING_TO_REACH_2;
		}
		else
		{
			item->animNumber = LA_ROPE_SWING_TO_REACH_3;
		}

		item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
		item->currentAnimState = LS_REACH;
		item->goalAnimState = LS_REACH;

		Lara.ropePtr = -1;
	}
}

void FallFromRope(ITEM_INFO* item) // (F) (D)
{
	item->speed = abs(CurrentPendulum.Velocity.x >> 16) + abs(CurrentPendulum.Velocity.z >> 16) >> 1;
	item->pos.xRot = 0;
	item->pos.yPos += 320;

	item->animNumber = LA_FALL_START;
	item->frameNumber = g_Level.Anims[LA_FALL_START].frameBase;
	item->currentAnimState = LS_JUMP_FORWARD;
	item->goalAnimState = LS_JUMP_FORWARD;

	item->fallspeed = 0;
	item->gravityStatus = true;

	Lara.gunStatus = LG_NO_ARMS;
	Lara.ropePtr = -1;
}

void lara_col_poledown(ITEM_INFO* item, COLL_INFO* coll)//171A0, 172D4 (F)
{
	coll->enableSpaz = false;
	coll->enableBaddiePush = false;

	if (TrInput & IN_LOOK)
		LookUpDown();

	if ((TrInput & (IN_BACK | IN_ACTION)) != (IN_BACK | IN_ACTION) || item->hitPoints <= 0)
		item->goalAnimState = LS_POLE_IDLE;

	coll->badPos = NO_BAD_POS;
	coll->badNeg = -STEPUP_HEIGHT;
	coll->badCeiling = 0;

	Lara.moveAngle = item->pos.yRot;

	coll->slopesAreWalls = true;

	coll->facing = Lara.moveAngle;
	coll->radius = 100;

	GetLaraCollisionInfo(item, coll);

	if (coll->midFloor < 0)
	{
		short roomNumber = item->roomNumber;
		item->floor = GetFloorHeight(GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber),
			item->pos.xPos, item->pos.yPos - 762, item->pos.zPos);

		item->goalAnimState = LS_POLE_IDLE;
		item->itemFlags[2] = 0;
	}

	if (TrInput & IN_LEFT)
	{
		item->pos.yRot += 256;
	}
	else if (TrInput & IN_RIGHT)
	{
		item->pos.yRot -= 256;
	}

	if (item->animNumber == LA_POLE_DOWN_END)
	{
		item->itemFlags[2] -= SECTOR(1);
	}
	else
	{
		item->itemFlags[2] += 256;
	}

	// CHECK
	SoundEffect(SFX_LARA_ROPEDOWN_LOOP, &item->pos, 0);

	item->itemFlags[2] = CLAMP(item->itemFlags[2], 0, ANGLE(90.0f));

	item->pos.yPos += item->itemFlags[2] >> 8;
}

void lara_col_poleup(ITEM_INFO* item, COLL_INFO* coll)//170D8(<), 1720C(<) (F)
{
	coll->enableBaddiePush = false;
	coll->enableSpaz = false;

	if (TrInput & IN_LOOK)
		LookUpDown();

	if (!(TrInput & IN_ACTION) || !(TrInput & IN_FORWARD) || item->hitPoints <= 0)
		item->goalAnimState = LS_POLE_IDLE;

	short roomNumber = item->roomNumber;

	if (item->pos.yPos -
		GetCeiling(GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber),
			item->pos.xPos, item->pos.yPos, item->pos.zPos) < SECTOR(1))
		item->goalAnimState = LS_POLE_IDLE;
}

void lara_as_poleright(ITEM_INFO* item, COLL_INFO* coll)//1707C(<), 171B0(<) (F)
{
	coll->enableBaddiePush = false;
	coll->enableSpaz = false;
	if (!(TrInput & IN_RIGHT) || !(TrInput & IN_ACTION) || (TrInput & (IN_FORWARD | IN_BACK)) || item->hitPoints <= 0)
		item->goalAnimState = LS_POLE_IDLE;
	else
		item->pos.yRot -= 256;
}

void lara_as_poleleft(ITEM_INFO* item, COLL_INFO* coll)//17020(<), 17154(<) (F)
{
	coll->enableBaddiePush = false;
	coll->enableSpaz = false;
	if (!(TrInput & IN_LEFT) || !(TrInput & IN_ACTION) || (TrInput & (IN_FORWARD | IN_BACK)) || item->hitPoints <= 0)
		item->goalAnimState = LS_POLE_IDLE;
	else
		item->pos.yRot += 256;
}

void lara_col_polestat(ITEM_INFO* item, COLL_INFO* coll)//16DFC, 16F30 (F)
{
	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_FREEFALL;
		return;
	}

	coll->enableSpaz = false;
	coll->enableBaddiePush = false;

	if (item->animNumber == LA_POLE_IDLE)
	{
		coll->badPos = NO_BAD_POS;
		coll->badNeg = -STEPUP_HEIGHT;
		coll->badCeiling = BAD_JUMP_CEILING;

		Lara.moveAngle = item->pos.yRot;

		coll->facing = Lara.moveAngle;
		coll->radius = 100;
		coll->slopesAreWalls = true;

		GetLaraCollisionInfo(item, coll);

		if (TrInput & IN_ACTION)
		{
			item->goalAnimState = LS_POLE_IDLE;

			if (TrInput & IN_LEFT)
			{
				item->goalAnimState = LS_POLE_TURN_CLOCKWISE;
			}
			else if (TrInput & IN_RIGHT)
			{
				item->goalAnimState = LS_POLE_TURN_COUNTER_CLOCKWISE;
			}

			if (TrInput & IN_LOOK)
				LookUpDown();

			if (TrInput & IN_FORWARD)
			{
				short roomNum = item->roomNumber;

				if (item->pos.yPos - GetCeiling(GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNum),
					item->pos.xPos, item->pos.yPos, item->pos.zPos) > SECTOR(1))
				{
					item->goalAnimState = LS_POLE_UP;
				}
			}
			else if (TrInput & IN_BACK && coll->midFloor > 0)
			{
				item->goalAnimState = LS_POLE_DOWN;
				item->itemFlags[2] = 0;
			}

			if (TrInput & IN_JUMP)
				item->goalAnimState = LS_JUMP_BACK;
		}
		else if (coll->midFloor <= 0)
		{
			item->goalAnimState = LS_STOP;
		}
		else
		{
			item->pos.xPos -= (phd_sin(item->pos.yRot)) << 6 >> W2V_SHIFT;
			item->pos.zPos -= (phd_cos(item->pos.yRot)) << 6 >> W2V_SHIFT;
			item->goalAnimState = LS_FREEFALL;
		}
	}
}

void lara_col_monkey180(ITEM_INFO* item, COLL_INFO* coll)//16DDC, 16F10 (F)
{
	lara_col_monkeyswing(item, coll);
}

void lara_as_monkey180(ITEM_INFO* item, COLL_INFO* coll)//16DB8(<), 16EEC(<) (F)
{
	coll->enableBaddiePush = false;
	coll->enableSpaz = false;
	item->goalAnimState = LS_MONKEYSWING_IDLE;
}

void lara_as_hangturnr(ITEM_INFO* item, COLL_INFO* coll)//16D64(<), 16E98(<) (F)
{
	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_MONKEYSWING_IDLE;
		return;
	}
	
	Camera.targetElevation = 1820;
	Lara.torsoYrot = 0;
	Lara.torsoXrot = 0;
	item->pos.yRot += ANGLE(1.5f);

	if (!(TrInput & IN_RIGHT))
		item->goalAnimState = LS_MONKEYSWING_IDLE;
}

void lara_col_hangturnlr(ITEM_INFO* item, COLL_INFO* coll)//16C94(<), 16DC8(<) (F)
{
	if ((TrInput & IN_ACTION) && Lara.canMonkeySwing)
	{
		coll->badPos = NO_BAD_POS;
		coll->badNeg = -STEPUP_HEIGHT;
		coll->badCeiling = 0;

		Lara.moveAngle = item->pos.yRot;

		coll->facing = item->pos.yRot;
		coll->radius = 100;
		coll->slopesAreWalls = true;

		GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, 600);
		MonkeySwingSnap(item, coll);
	}
	else
	{
		MonkeySwingFall(item);
	}
}

void lara_as_hangturnl(ITEM_INFO* item, COLL_INFO* coll)//16C40(<), 16D74(<) (F)
{
	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_MONKEYSWING_IDLE;
		return;
	}

	Camera.targetElevation = 1820;
	Lara.torsoYrot = 0;
	Lara.torsoXrot = 0;
	item->pos.yRot -= ANGLE(1.5f);

	if (!(TrInput & IN_LEFT))
		item->goalAnimState = LS_MONKEYSWING_IDLE;
}

void lara_col_monkeyr(ITEM_INFO* item, COLL_INFO* coll)//16B9C(<), 16CD0(<) (F)
{
	if ((TrInput & IN_ACTION) && Lara.canMonkeySwing)
	{
		if (TestMonkeyRight(item, coll))
		{
			MonkeySwingSnap(item, coll);
		}
		else
		{
			item->animNumber = LA_MONKEYSWING_IDLE;
			item->currentAnimState = LS_MONKEYSWING_IDLE;
			item->goalAnimState = LS_MONKEYSWING_IDLE;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
		}
	}
	else
	{
		MonkeySwingFall(item);
	}
}

void lara_as_monkeyr(ITEM_INFO* item, COLL_INFO* coll)//16B24(<), 16C58(<) (F)
{
	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_MONKEYSWING_IDLE;
		return;
	}

	Lara.torsoYrot = 0;
	Lara.torsoXrot = 0;

	coll->enableBaddiePush = false;
	coll->enableSpaz = false;

	if (TrInput & IN_RSTEP)
	{
		item->goalAnimState = LS_MONKEYSWING_RIGHT;
		Camera.targetElevation = ANGLE(10.0f);
	}
	else
	{
		item->goalAnimState = LS_MONKEYSWING_IDLE;
		Camera.targetElevation = ANGLE(10.0f);
	}
}

void lara_col_monkeyl(ITEM_INFO* item, COLL_INFO* coll)//16A80(<), 16BB4(<) (F)
{
	if ((TrInput & IN_ACTION) && Lara.canMonkeySwing)
	{
		if (TestMonkeyLeft(item, coll))
		{
			MonkeySwingSnap(item, coll);
		}
		else
		{
			item->animNumber = LA_MONKEYSWING_IDLE;
			item->currentAnimState = LS_MONKEYSWING_IDLE;
			item->goalAnimState = LS_MONKEYSWING_IDLE;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
		}
	}
	else
	{
		MonkeySwingFall(item);
	}
}

void lara_as_monkeyl(ITEM_INFO* item, COLL_INFO* coll)//16A0C(<), 16B40(<) (F)
{
	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_MONKEYSWING_IDLE;

		return;
	}

	Lara.torsoYrot = 0;
	Lara.torsoXrot = 0;

	coll->enableBaddiePush = false;
	coll->enableSpaz = false;

	if (TrInput & IN_LSTEP)
	{
		item->goalAnimState = LS_MONKEYSWING_LEFT;
		Camera.targetElevation = ANGLE(10.0f);
	}
	else
	{
		item->goalAnimState = LS_MONKEYSWING_IDLE;
		Camera.targetElevation = ANGLE(10.0f);
	}
}

void lara_col_monkeyswing(ITEM_INFO* item, COLL_INFO* coll)//16828, 1695C (F)
{
	if (TrInput & IN_ACTION && Lara.canMonkeySwing)
	{
		coll->badPos = NO_BAD_POS;
		coll->badNeg = NO_HEIGHT;
		coll->badCeiling = 0;

		Lara.moveAngle = item->pos.yRot;

		coll->enableSpaz = false;
		coll->enableBaddiePush = false;

		coll->facing = Lara.moveAngle;
		coll->radius = 100;

		GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, 600);

		if (coll->collType == CT_FRONT
			|| abs(coll->midCeiling - coll->frontCeiling) > 50)
		{
			item->animNumber = LA_MONKEYSWING_IDLE;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			item->currentAnimState = LS_MONKEYSWING_IDLE;
			item->goalAnimState = LS_MONKEYSWING_IDLE;
		}
		else
		{
			if (abs(coll->midCeiling - coll->leftCeiling2) <= 50)
			{
				if (abs(coll->midCeiling - coll->rightCeiling2) > 50)
				{
					ShiftItem(item, coll);
					item->pos.yRot -= ANGLE(5.0f);
				}
			}
			else
			{
				ShiftItem(item, coll);
				item->pos.yRot += ANGLE(5.0f);
			}

			Camera.targetElevation = ANGLE(10.0f);
			MonkeySwingSnap(item, coll);
		}
	}
	else
	{
		MonkeySwingFall(item);
	}
}

void lara_as_monkeyswing(ITEM_INFO* item, COLL_INFO* coll)//1670C, 16840 (F)
{
	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_MONKEYSWING_IDLE;
		return;
	}

	coll->enableSpaz = false;
	coll->enableBaddiePush = false;

	Lara.torsoYrot = 0;
	Lara.torsoXrot = 0;

	if (TrInput & IN_LOOK)
		LookUpDown();

	if (TrInput & IN_FORWARD)
		item->goalAnimState = LS_MONKEYSWING_FORWARD;
	else
		item->goalAnimState = LS_MONKEYSWING_IDLE;

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

void lara_col_hang2(ITEM_INFO* item, COLL_INFO* coll)//163DC, 16510 (F)
{
	item->fallspeed = 0;
	item->gravityStatus = false;

	if (Lara.canMonkeySwing)
	{
		coll->badPos = NO_BAD_POS;
		coll->badNeg = NO_HEIGHT;
		coll->badCeiling = 0;

		coll->slopesAreWalls = 0;
		coll->facing = Lara.moveAngle;
		coll->radius = 100;

		Lara.moveAngle = item->pos.yRot;

		GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, 600);

		// FOR DEBUG PURPOSES UNTIL SCRIPTING IS READY-
		EnableMonkeyRoll = true;


		if (TrInput & IN_FORWARD && coll->collType != CT_FRONT && abs(coll->midCeiling - coll->frontCeiling) < 50)
		{
			item->goalAnimState = LS_MONKEYSWING_FORWARD;
		}
		else if (TrInput & IN_LSTEP && TestMonkeyLeft(item, coll))
		{
			item->goalAnimState = LS_MONKEYSWING_LEFT;
		}
		else if (TrInput & IN_RSTEP && TestMonkeyRight(item, coll))
		{
			item->goalAnimState = LS_MONKEYSWING_RIGHT;
		}
		else if (TrInput & IN_LEFT)
		{
			item->goalAnimState = LS_MONKEYSWING_TURN_LEFT;
		}
		else if (TrInput & IN_RIGHT)
		{
			item->goalAnimState = LS_MONKEYSWING_TURN_RIGHT;
		}
		else if ((TrInput & IN_ROLL) && EnableMonkeyRoll == true)
			{
				item->currentAnimState = LS_MONKEYSWING_TURN_180;
				item->goalAnimState = LS_MONKEYSWING_IDLE;
				item->animNumber = LA_MONKEYSWING_TURN_180;
				item->frameNumber = g_Level.Anims[LA_MONKEYSWING_TURN_180].frameBase;
			}

		MonkeySwingSnap(item, coll);
	}
	else
	{
		LaraHangTest(item, coll);

		if (item->goalAnimState == LS_MONKEYSWING_IDLE)
		{
			TestForObjectOnLedge(item, coll);

			if (!(TrInput & IN_FORWARD) ||
				coll->frontFloor <= -850 ||
				coll->frontFloor >= -650 ||
				coll->frontFloor < coll->frontCeiling ||
				coll->leftFloor2 < coll->leftCeiling2 ||
				coll->rightFloor2 < coll->rightCeiling2 ||
				coll->hitStatic)
			{
				if (!(TrInput & IN_FORWARD) ||
					coll->frontFloor <= -850 ||
					coll->frontFloor >= -650 ||
					coll->frontFloor - coll->frontCeiling < -256 ||
					coll->leftFloor2 - coll->leftCeiling2 < -256 ||
					coll->rightFloor2 - coll->rightCeiling2 < -256 ||
					coll->hitStatic)
				{
					if (TrInput & IN_LEFT || TrInput & IN_LSTEP)
					{
						item->goalAnimState = LS_SHIMMY_LEFT;
					}
					else if (TrInput & IN_RIGHT || TrInput & IN_RSTEP)
					{
						item->goalAnimState = LS_SHIMMY_RIGHT;
					}
				}
				else
				{
					item->goalAnimState = LS_HANG_TO_CRAWL;
					item->requiredAnimState = LS_CROUCH_IDLE;
				}
			}
			else if (TrInput & IN_WALK)
			{
				item->goalAnimState = LS_HANDSTAND;
			}
			else if (TrInput & IN_DUCK)
			{
				item->goalAnimState = LS_HANG_TO_CRAWL;
				item->requiredAnimState = LS_CROUCH_IDLE;
			}
			else
			{
				item->goalAnimState = LS_GRABBING;
			}
		}
	}
}

void lara_as_hang2(ITEM_INFO* item, COLL_INFO* coll)//1630C(<), 16440(<) (F)
{
	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_STOP;
		return;
	}

	coll->enableBaddiePush = false;
	coll->enableSpaz = false;

	Lara.torsoYrot = 0;
	Lara.torsoXrot = 0;

	if (Lara.canMonkeySwing)
	{
		if (!(TrInput & IN_ACTION) || item->hitPoints <= 0)
			MonkeySwingFall(item);

		Camera.targetAngle = 0;
		Camera.targetElevation = -ANGLE(45.0f);
	}

	if (TrInput & IN_LOOK)
		LookUpDown();
}

short TestMonkeyRight(ITEM_INFO* item, COLL_INFO* coll)//161EC(<), 16320(<) (F)
{
	short oct;

	coll->badPos = NO_BAD_POS;
	coll->badNeg = -STEPUP_HEIGHT;
	coll->badCeiling = 0;
	Lara.moveAngle = item->pos.yRot + ANGLE(90.0f);
	coll->slopesAreWalls = 0;
	coll->facing = Lara.moveAngle;
	coll->radius = 100;

	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, 600);
	
	if (abs(coll->midCeiling - coll->frontCeiling) > 50)
		return 0;

	if (!coll->collType)
		return 1;

	oct = GetDirOctant(item->pos.yRot);
	if (oct)
	{
		if (oct != 1)
			return 1;
		if (coll->collType != CT_FRONT && coll->collType != CT_RIGHT && coll->collType != CT_LEFT)
			return 1;
	}
	else if (coll->collType != CT_FRONT)
	{
		return 1;
	}

	return 0;
}

short TestMonkeyLeft(ITEM_INFO* item, COLL_INFO* coll)//160CC(<), 16200(<) (F)
{
	short oct;

	coll->badPos = NO_BAD_POS;
	coll->badNeg = NO_HEIGHT;
	coll->badCeiling = 0;
	Lara.moveAngle = item->pos.yRot - ANGLE(90.0f);
	coll->slopesAreWalls = 0;
	coll->facing = Lara.moveAngle;
	coll->radius = 100;

	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, 600);
	
	if (abs(coll->midCeiling - coll->frontCeiling) > 50)
		return 0;
	
	if (!coll->collType)
		return 1;
	
	oct = GetDirOctant(item->pos.yRot);
	if (oct)
	{
		if (oct != 1)
			return 1;
		if (coll->collType != CT_RIGHT && coll->collType != CT_LEFT)
			return 1;
	}
	else
	{
		if (coll->collType != CT_FRONT && coll->collType != CT_LEFT)
			return 1;
	}

	return 0;
}

short GetDirOctant(int rot)//160B4(<), 161E8(<) (F)
{
	return abs(rot) >= ANGLE(45) && abs(rot) <= ANGLE(135.0f);
}

void MonkeySwingSnap(ITEM_INFO* item, COLL_INFO* coll)//1605C(<), 16190(<) (F)
{
	short roomNum = item->roomNumber;
	item->pos.yPos = GetCeiling(GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNum),
		item->pos.xPos, item->pos.yPos, item->pos.zPos) + 704;
}

void MonkeySwingFall(ITEM_INFO* item)//16004(<), 16138(<) (F)
{
	item->goalAnimState = LS_JUMP_UP;
	item->currentAnimState = LS_JUMP_UP;
	item->animNumber = LA_JUMP_UP;
	item->frameNumber = g_Level.Anims[item->animNumber].frameBase + 9;

	item->speed = 2;
	item->gravityStatus = true;
	item->fallspeed = 1;
	item->pos.yPos += 256;

	Lara.gunStatus = LG_NO_ARMS;
}

void lara_col_dashdive(ITEM_INFO* item, COLL_INFO* coll)//15E5C, 15F90 (F)
{
	if (item->speed < 0)
		Lara.moveAngle = item->pos.yRot - ANGLE(180.0f);
	else
		Lara.moveAngle = item->pos.yRot;

	coll->badPos = NO_BAD_POS;
	coll->badNeg = -256;
	coll->badCeiling = BAD_JUMP_CEILING;

	coll->slopesAreWalls = true;

	GetLaraCollisionInfo(item, coll);
	LaraDeflectEdgeJump(item, coll);

	if (!LaraFallen(item, coll))
	{
		if (item->speed < 0)
			Lara.moveAngle = item->pos.yRot;

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
	Lara.moveAngle = item->pos.yRot;

	coll->badPos = NO_BAD_POS;
	coll->badNeg = -STEPUP_HEIGHT;
	coll->badCeiling = 0;

	coll->slopesAreWalls = true;

	GetLaraCollisionInfo(item, coll);

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

void lara_col_crawl2hang(ITEM_INFO* item, COLL_INFO* coll)//15770, 158A4 (F)
{
	Camera.targetAngle = 0;
	Camera.targetElevation = -ANGLE(45.0f);

	coll->enableSpaz = false;
	coll->enableBaddiePush = false;

	if (item->animNumber == LA_CRAWL_TO_HANG_END)
	{
		int edgeCatch;
		int edge;

		item->fallspeed = 512;
		item->pos.yPos += 255;

		coll->badPos = NO_BAD_POS;
		coll->badNeg = -STEPUP_HEIGHT;
		coll->badCeiling = BAD_JUMP_CEILING;

		Lara.moveAngle = item->pos.yRot;
		coll->facing = Lara.moveAngle;

		GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, 870);
		edgeCatch = LaraTestEdgeCatch(item, coll, &edge);

		if (edgeCatch)
		{
			if (edgeCatch >= 0 || LaraTestHangOnClimbWall(item, coll))
			{
				short angle = item->pos.yRot;

				if (abs(angle) > ANGLE(35.0f))
				{
					if (angle >= 10014 && angle <= 22754)
					{
						angle = ANGLE(90.0f);
					}
					else
					{
						if (abs(angle) >= 26397)
						{
							angle = -ANGLE(180.0f);
						}
						else if (angle >= -22754 && angle <= -10014)
						{
							angle = -ANGLE(90.0f);
						}
					}
				}
				else
				{
					angle = 0;
				}

				if ((angle & 0x3FFF) == 0)
				{
					BOUNDING_BOX* bounds;

					if (TestHangSwingIn(item, angle))
					{

							Lara.headYrot = 0;
							Lara.headXrot = 0;
							Lara.torsoYrot = 0;
							Lara.torsoXrot = 0;
							item->animNumber = LA_JUMP_UP_TO_MONKEYSWING;
							item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
							item->currentAnimState = LS_MONKEYSWING_IDLE;
							item->goalAnimState = LS_MONKEYSWING_IDLE;
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

					bounds = GetBoundsAccurate(item);

					if (edgeCatch <= 0)
					{
						item->pos.yPos = edge - bounds->Y1;
					}
					else
					{

/*						item->pos.xPos += coll->shift.x;
						item->pos.zPos += coll->shift.z;
						@ORIGINAL_BUG: these two caused teleportation when Lara performed crawl2hang on triangulated geometry. replacing with shifts to the edges of blocks solved it*/

						short angl = (unsigned short)(item->pos.yRot + ANGLE(45.0f)) / ANGLE(90.0f);
						switch (angl)
						{
						case NORTH:
							item->pos.zPos = (item->pos.zPos | (WALL_SIZE - 1)) - LARA_RAD;
							break;

						case EAST:
							item->pos.xPos = (item->pos.xPos | (WALL_SIZE - 1)) - LARA_RAD;
							break;

						case SOUTH:
							item->pos.zPos = (item->pos.zPos & -WALL_SIZE) + LARA_RAD;
							break;

						case WEST:
							item->pos.xPos = (item->pos.xPos & -WALL_SIZE) + LARA_RAD;
							break;
						}
					}
					item->pos.yPos += coll->frontFloor - bounds->Y1;
					item->pos.yRot = angle;

					item->gravityStatus = true;
					item->speed = 2;
					item->fallspeed = 1;

					Lara.gunStatus = LG_HANDS_BUSY;
				}
			}
		}
	}
}

void lara_col_crawlb(ITEM_INFO* item, COLL_INFO* coll)//15614, 15748 (F)
{
	item->gravityStatus = false;
	item->fallspeed = 0;

	coll->radius = 250;
	coll->badPos = 255;
	coll->badNeg = -127;
	coll->badCeiling = 400;
	coll->slopesArePits = true;
	coll->slopesAreWalls = true;

	Lara.moveAngle = item->pos.yRot - ANGLE(180.0f);

	coll->facing = Lara.moveAngle;

	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, 400);

	if (LaraDeflectEdgeDuck(item, coll))
	{
		item->currentAnimState = LS_CRAWL_IDLE;
		item->goalAnimState = LS_CRAWL_IDLE;

		if (item->animNumber != LA_CRAWL_IDLE)
		{
			item->animNumber = LA_CRAWL_IDLE;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
		}
	}
	else if (LaraFallen(item, coll))
	{
		Lara.gunStatus = LG_NO_ARMS;
	}
	else if (!TestLaraSlide(item, coll))
	{
		ShiftItem(item, coll);

		if (coll->midFloor != NO_HEIGHT && coll->midFloor > -256)
			item->pos.yPos += coll->midFloor;

		Lara.moveAngle = item->pos.yRot;
	}
}

void lara_as_crawlb(ITEM_INFO* item, COLL_INFO* coll)//154F0, 15624 (F)
{
	if (item->hitPoints <= 0 || Lara.waterStatus == 4)
	{
		item->goalAnimState = LS_CRAWL_IDLE;
		return;
	}

	if (TrInput & IN_LOOK)
		LookUpDown();

	coll->enableSpaz = false;
	coll->enableBaddiePush = true;

	Lara.torsoYrot = 0;
	Lara.torsoXrot = 0;

	Camera.targetElevation = -ANGLE(23.0f);

	if (TrInput & IN_BACK)
	{
		if (TrInput & IN_RIGHT)
		{
			Lara.turnRate -= LARA_TURN_RATE;
			if (Lara.turnRate < -ANGLE(3.0f))
				Lara.turnRate = -ANGLE(3.0f);
		}
		else if (TrInput & IN_LEFT)
		{
			Lara.turnRate += LARA_TURN_RATE;
			if (Lara.turnRate > ANGLE(3.0f))
				Lara.turnRate = ANGLE(3.0f);
		}
	}
	else
	{
		item->goalAnimState = LS_CRAWL_IDLE;
	}
}

void lara_as_all4turnr(ITEM_INFO* item, COLL_INFO* coll)//15484(<), 155B8(<) (F)
{
	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_CRAWL_IDLE;
		return;
	}

	coll->enableSpaz = 0;
	coll->enableBaddiePush = 1;
	Lara.torsoYrot = 0;
	Lara.torsoXrot = 0;
	Camera.targetElevation = -ANGLE(23.0f);
	item->pos.yRot += ANGLE(1.5f);

	if (!(TrInput & IN_RIGHT))
		item->goalAnimState = LS_CRAWL_IDLE;
}

void lara_col_all4turnlr(ITEM_INFO* item, COLL_INFO* coll)//153FC, 15530 (F)
{
	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, 400);

	if (!TestLaraSlide(item, coll))
	{
		if (coll->midFloor != NO_HEIGHT && coll->midFloor > -256)
			item->pos.yPos += coll->midFloor;
	}
}

void lara_as_all4turnl(ITEM_INFO* item, COLL_INFO* coll)//15390(<), 154C4(<) (F)
{
	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_CRAWL_IDLE;
		return;
	}

	coll->enableSpaz = 0;
	coll->enableBaddiePush = 1;
	Lara.torsoYrot = 0;
	Lara.torsoXrot = 0;
	Camera.targetElevation = -ANGLE(23.0f);
	item->pos.yRot -= ANGLE(1.5f);

	if (!(TrInput & IN_LEFT))
		item->goalAnimState = LS_CRAWL_IDLE;
}

void lara_col_crawl(ITEM_INFO* item, COLL_INFO* coll)//1523C, 15370 (F)
{
	item->gravityStatus = false;
	item->fallspeed = 0;

	Lara.moveAngle = item->pos.yRot;

	coll->radius = 200;

	coll->badPos = 255;
	coll->badNeg = -127;
	coll->badCeiling = 400;

	coll->slopesArePits = true;
	coll->slopesAreWalls = true;

	coll->facing = Lara.moveAngle;

	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, -400);

	if (LaraDeflectEdgeDuck(item, coll))
	{
		item->currentAnimState = LS_CRAWL_IDLE;
		item->goalAnimState = LS_CRAWL_IDLE;

		if (item->animNumber != LA_CRAWL_IDLE)
		{
			item->animNumber = LA_CRAWL_IDLE;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
		}
	}
	else if (LaraFallen(item, coll))
	{
		Lara.gunStatus = LG_NO_ARMS;
	}
	else if (!TestLaraSlide(item, coll))
	{
		ShiftItem(item, coll);

		if (coll->midFloor != NO_HEIGHT && coll->midFloor > -256)
			item->pos.yPos += coll->midFloor;
	}
}

void lara_as_crawl(ITEM_INFO* item, COLL_INFO* coll)//150F4, 15228 (F)
{
	if (item->hitPoints <= 0 || TrInput & IN_JUMP)
	{
		item->goalAnimState = LS_CRAWL_IDLE;
		return;
	}

	if (TrInput & IN_LOOK)
		LookUpDown();

	Lara.torsoXrot = 0;
	Lara.torsoYrot = 0;

	coll->enableSpaz = false;
	coll->enableBaddiePush = true;

	Camera.targetElevation = -ANGLE(23.0f);

	if (TrInput & IN_FORWARD
		&& (TrInput & IN_DUCK || Lara.keepDucked)
		&& Lara.waterStatus != LW_WADE)
	{
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
	else
	{
		item->goalAnimState = LS_CRAWL_IDLE;
	}
}

void lara_col_all4s(ITEM_INFO* item, COLL_INFO* coll)//14B40, 14C74 (F)
{
	item->fallspeed = 0;
	item->gravityStatus = false;

	if (item->goalAnimState != LS_CRAWL_TO_HANG)
	{
		Lara.moveAngle = item->pos.yRot;
		coll->facing = Lara.moveAngle;

		coll->radius = 200;
		coll->badPos = 255;
		coll->badNeg = -127;
		coll->badCeiling = 400;

		coll->slopesAreWalls = true;
		coll->slopesArePits = true;

		GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, 400);

		if (LaraFallen(item, coll))
		{
			Lara.gunStatus = LG_NO_ARMS;
		}
		else if (!TestLaraSlide(item, coll))
		{
			int slope = abs(coll->leftFloor2 - coll->rightFloor2);

			Lara.keepDucked = coll->midCeiling >= -362;

			ShiftItem(item, coll);

			if (coll->midFloor != NO_HEIGHT && coll->midFloor > -256)
				item->pos.yPos += coll->midFloor;

			if (TrInput & IN_DUCK || Lara.keepDucked &&
				(!(TrInput & IN_FLARE) && !(TrInput & IN_DRAW) || TrInput & IN_FORWARD) &&
				Lara.waterStatus != LW_WADE)
			{
				if (item->animNumber == LA_CRAWL_IDLE ||
				item->animNumber == LA_CROUCH_TO_CRAWL_END ||
				item->animNumber == LA_CRAWL_TO_IDLE_END_RIGHT_POINTLESS ||
				item->animNumber == LA_CRAWL_TO_IDLE_END_LEFT_POINTLESS)
				{
					if (TrInput & IN_FORWARD)
					{
						if (abs(LaraFloorFront(item, item->pos.yRot, 256)) < 127 && HeightType != BIG_SLOPE)
							item->goalAnimState = LS_CRAWL_FORWARD;
					}
					else if (TrInput & IN_BACK)
					{
						short height = LaraCeilingFront(item, item->pos.yRot, -300, 128);
						short heightl = 0;
						short heightr = 0;

						if (height != NO_HEIGHT && height <= 256)
						{
							if (TrInput & IN_ACTION)
							{
								int x = item->pos.xPos;
								int z = item->pos.zPos;

								item->pos.xPos += 128 * phd_sin(item->pos.yRot - ANGLE(90.0f)) >> W2V_SHIFT;
								item->pos.zPos += 128 * phd_cos(item->pos.yRot - ANGLE(90.0f)) >> W2V_SHIFT;

								heightl = LaraFloorFront(item, item->pos.yRot, -300);

								item->pos.xPos += 256 * phd_sin(item->pos.yRot + ANGLE(90.0f)) >> W2V_SHIFT;
								item->pos.zPos += 256 * phd_cos(item->pos.yRot + ANGLE(90.0f)) >> W2V_SHIFT;

								heightr = LaraFloorFront(item, item->pos.yRot, -300);

								item->pos.xPos = x;
								item->pos.zPos = z;
							}

							height = LaraFloorFront(item, item->pos.yRot, -300);

							if (abs(height) >= 255 || HeightType == BIG_SLOPE)
							{
								if (TrInput & IN_ACTION)
								{
									if (height > 768 &&
										heightl > 768 &&
										heightr > 768 &&
										slope < 120)
									{
										int tmp;
										ITEM_INFO* tmp1;
										MESH_INFO* tmp2;
										int x = item->pos.xPos;
										int z = item->pos.zPos;

										item->pos.xPos -= 100 * phd_sin(coll->facing) >> W2V_SHIFT;
										item->pos.zPos -= 100 * phd_cos(coll->facing) >> W2V_SHIFT;

										//tmp = GetCollidedObjects(item, 100, 1, wat, wat, 0);
										//S_Warn("[lara_col_all4s] - Warning: Core Design function call shittery\n");
										tmp = GetCollidedObjects(item, 100, 1, &tmp1, &tmp2, 0);

										item->pos.xPos = x;
										item->pos.zPos = z;

										if (!tmp)
										{
											switch ((unsigned short) (item->pos.yRot + ANGLE(45.0f)) / ANGLE(90.0f))
											{
											case 0:
												item->pos.yRot = 0;
												item->pos.zPos = (item->pos.zPos & 0xFFFFFC00) + 225;
												break;
											case 1:
												item->pos.yRot = ANGLE(90.0f);
												item->pos.xPos = (item->pos.xPos & 0xFFFFFC00) + 225;
												break;
											case 2:
												item->pos.yRot = -ANGLE(180.0f);
												item->pos.zPos = (item->pos.zPos | 0x3FF) - 225;
												break;
											case 3:
												item->pos.yRot = -ANGLE(90.0f);
												item->pos.xPos = (item->pos.xPos | 0x3FF) - 225;
												break;
											}

											item->goalAnimState = LS_CRAWL_TO_HANG;
										}
									}
								}
							}
							else if (!(abs(height) >= 127))
							{
								item->goalAnimState = LS_CRAWL_BACK;
							}
						}
					}
					else if (TrInput & IN_LEFT)
					{
						item->animNumber = LA_CRAWL_TURN_LEFT;
						item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
						item->currentAnimState = LS_CRAWL_TURN_LEFT;
						item->goalAnimState = LS_CRAWL_TURN_LEFT;
					}
					else if (TrInput & IN_RIGHT)
					{
						item->animNumber = LA_CRAWL_TURN_RIGHT;
						item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
						item->currentAnimState = LS_CRAWL_TURN_RIGHT;
						item->goalAnimState = LS_CRAWL_TURN_RIGHT;
					}
				}
			}
			else
			{
				item->goalAnimState = LS_CROUCH_IDLE;
			}
		}
	}
}

void lara_as_all4s(ITEM_INFO* item, COLL_INFO* coll)//14970, 14A78 (F)
{
	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_DEATH;
		return;
	}

	// FOR DEBUG PURPOSES UNTIL SCRIPTING IS FINISHED
		EnableCrawlFlex1clickdown = true;
		EnableCrawlFlex1clickup = true;
		EnableCrawlFlex3clickE = true;
		EnableCrawlFlex2clickE = true;
		EnableCrawlFlex1clickE = true;

	



	if (TrInput & IN_JUMP)
	{
		GAME_VECTOR s, d;
		MESH_INFO* StaticMesh;
		PHD_VECTOR v;

		if (LaraFloorFront(item, item->pos.yRot, 512) > 768 &&
			LaraCeilingFront(item, item->pos.yRot, 768, 512) != NO_HEIGHT &&
			LaraCeilingFront(item, item->pos.yRot, 768, 512) <= 0)
		{

			s.x = LaraItem->pos.xPos;
			s.y = LaraItem->pos.yPos - 96;
			s.z = LaraItem->pos.zPos;
			s.roomNumber = LaraItem->roomNumber;

			d.x = s.x + (768 * phd_sin(LaraItem->pos.yRot) >> W2V_SHIFT);
			d.y = s.y + 160;
			d.z = s.z + (768 * phd_cos(LaraItem->pos.yRot) >> W2V_SHIFT);

			if (LOS(&s, &d) && item->animNumber != LA_CROUCH_TO_CRAWL_START && item->animNumber != LA_CROUCH_TO_CRAWL_CONTINUE)
			{
				// TODO: fix ObjectOnLOS2
				/*if (ObjectOnLOS2(&s, &d, &v, (PHD_VECTOR*)&StaticMesh) == 999)
				{*/
					item->animNumber = LA_CRAWL_JUMP_FLIP_DOWN;
					item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
					item->goalAnimState = LS_MISC_CONTROL;
					item->currentAnimState = LS_MISC_CONTROL;
					Lara.gunStatus = LG_HANDS_BUSY;
				/*}*/
			}
		}
		else if (LaraFloorFront(item, item->pos.yRot, 256) == 768 &&
			LaraCeilingFront(item, item->pos.yRot, 768, 512) != NO_HEIGHT &&
			LaraCeilingFront(item, item->pos.yRot, 768, 512) <= 0)
		{
			s.x = LaraItem->pos.xPos;
			s.y = LaraItem->pos.yPos - 96;
			s.z = LaraItem->pos.zPos;
			s.roomNumber = LaraItem->roomNumber;

			d.x = s.x + (768 * phd_sin(LaraItem->pos.yRot) >> W2V_SHIFT);
			d.y = s.y + 160;
			d.z = s.z + (768 * phd_cos(LaraItem->pos.yRot) >> W2V_SHIFT);

			if (LOS(&s, &d) && item->animNumber != LA_CROUCH_TO_CRAWL_START && item->animNumber != LA_CROUCH_TO_CRAWL_CONTINUE && EnableCrawlFlex3clickE == true)
			{
				item->animNumber = LA_CRAWL_JUMP_DOWN_23CLICK;
				item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
				item->goalAnimState = LS_MISC_CONTROL;
				item->currentAnimState = LS_MISC_CONTROL;
				Lara.gunStatus = LG_HANDS_BUSY;

			}
		}	
		else	if (LaraFloorFront(item, item->pos.yRot, 256) == 512 &&
				LaraCeilingFront(item, item->pos.yRot, 768, 512) != NO_HEIGHT &&
				LaraCeilingFront(item, item->pos.yRot, 768, 512) <= 0)
			{

			s.x = LaraItem->pos.xPos;
			s.y = LaraItem->pos.yPos - 96;
			s.z = LaraItem->pos.zPos;
			s.roomNumber = LaraItem->roomNumber;

			d.x = s.x + (768 * phd_sin(LaraItem->pos.yRot) >> W2V_SHIFT);
			d.y = s.y + 160;
			d.z = s.z + (768 * phd_cos(LaraItem->pos.yRot) >> W2V_SHIFT);

			if (LOS(&s, &d) && item->animNumber != LA_CROUCH_TO_CRAWL_START && item->animNumber != LA_CROUCH_TO_CRAWL_CONTINUE && EnableCrawlFlex2clickE == true)
			{
				item->animNumber = LA_CRAWL_JUMP_DOWN_23CLICK;
				item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
				item->goalAnimState = LS_MISC_CONTROL;
				item->currentAnimState = LS_MISC_CONTROL;
				Lara.gunStatus = LG_HANDS_BUSY;

			}
			}
		else if (LaraFloorFront(item, item->pos.yRot, 256) == 256 &&
			LaraCeilingFront(item, item->pos.yRot, 768, 512) != NO_HEIGHT &&
			LaraCeilingFront(item, item->pos.yRot, 768, 512) <= 0)
		{
			s.x = LaraItem->pos.xPos;
			s.y = LaraItem->pos.yPos - 96;
			s.z = LaraItem->pos.zPos;
			s.roomNumber = LaraItem->roomNumber;

			d.x = s.x + (768 * phd_sin(LaraItem->pos.yRot) >> W2V_SHIFT);
			d.y = s.y + 160;
			d.z = s.z + (768 * phd_cos(LaraItem->pos.yRot) >> W2V_SHIFT);

			if (LOS(&s, &d) && item->animNumber != LA_CROUCH_TO_CRAWL_START && item->animNumber != LA_CROUCH_TO_CRAWL_CONTINUE && EnableCrawlFlex1clickE == true)
			{
				item->animNumber = LA_CRAWL_JUMP_DOWN_1CLICK;
				item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
				item->goalAnimState = LS_MISC_CONTROL;
				item->currentAnimState = LS_MISC_CONTROL;
				Lara.gunStatus = LG_HANDS_BUSY;

			}
		}
		
	}

	if ((TrInput & IN_ACTION) && (TrInput & IN_FORWARD) && item->animNumber != LA_CROUCH_TO_CRAWL_START && item->animNumber != LA_CROUCH_TO_CRAWL_CONTINUE)
	{
		if (LaraFloorFront(item, item->pos.yRot, 256) == -256 &&
			LaraCeilingFront(item, item->pos.yRot, 256, 256) != NO_HEIGHT &&
			LaraCeilingFront(item, item->pos.yRot, 256, 256) <= -512 &&
			EnableCrawlFlex1clickup == true)
		{
			item->animNumber = LA_CRAWL_UP_STEP;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			item->goalAnimState = LS_MISC_CONTROL;
			item->currentAnimState = LS_MISC_CONTROL;
		}
		else
			if (LaraFloorFront(item, item->pos.yRot, 256) == 256 &&
				LaraCeilingFront(item, item->pos.yRot, 256, 256) != NO_HEIGHT &&
				LaraCeilingFront(item, item->pos.yRot, 256, -256) <= -512 &&
				EnableCrawlFlex1clickdown == true)
			{
				item->animNumber = LA_CRAWL_DOWN_STEP;
				item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
				item->goalAnimState = LS_MISC_CONTROL;
				item->currentAnimState = LS_MISC_CONTROL;
			}
	}

	Lara.gunStatus = LG_HANDS_BUSY;

	if (TrInput & IN_LOOK)
		LookUpDown();

	Lara.torsoXrot = 0;
	Lara.torsoYrot = 0;

	coll->enableSpaz = false;
	coll->enableBaddiePush = true;

	if (item->animNumber == LA_CROUCH_TO_CRAWL_START)
		Lara.gunStatus = LG_HANDS_BUSY;

	Camera.targetElevation = -ANGLE(23.0f);

	if (g_Level.Rooms[LaraItem->roomNumber].flags & ENV_FLAG_WATER)
	{
		item->goalAnimState = LS_CROUCH_IDLE;
		item->requiredAnimState = LS_STOP;
	}
}

void lara_col_duck(ITEM_INFO* item, COLL_INFO* coll)//147C4, 148CC (F)
{
	item->gravityStatus = false;
	item->fallspeed = 0;

	Lara.moveAngle = item->pos.yRot;

	coll->facing = item->pos.yRot;
	coll->badPos = 384;
	coll->badNeg = -STEPUP_HEIGHT;
	coll->badCeiling = 0;

	coll->slopesAreWalls = true;

	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, 400);

	if (LaraFallen(item, coll))
	{
		Lara.gunStatus = LG_NO_ARMS;
	}
	else if (!TestLaraSlide(item, coll))
	{
		Lara.keepDucked = coll->midCeiling >= -362;

		ShiftItem(item, coll);

		if (coll->midFloor != NO_HEIGHT)
			item->pos.yPos += coll->midFloor;

		if (TrInput & IN_DUCK && Lara.waterStatus != LW_WADE || 
			Lara.keepDucked || 
			item->animNumber != LA_CROUCH_IDLE)
		{
			if (TrInput & IN_LEFT)
			{
				item->goalAnimState = LS_CROUCH_TURN_LEFT;
			}
			else if (TrInput & IN_RIGHT)
			{
				item->goalAnimState = LS_CROUCH_TURN_RIGHT;
			}
		}
		else
		{
			item->goalAnimState = LS_STOP;
		}
	}
}

void lara_as_duck(ITEM_INFO* item, COLL_INFO* coll)//14688, 14738 (F)
{
	short roomNum;

	coll->enableSpaz = false;
	coll->enableBaddiePush = true;

	Lara.isDucked = true;

	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_CRAWL_IDLE;
		return;
	}

	roomNum = LaraItem->roomNumber;

	if (TrInput & IN_LOOK)
		LookUpDown();

	GetFloor(LaraItem->pos.xPos, LaraItem->pos.yPos, LaraItem->pos.zPos, &roomNum);

	// FOR DEBUG PURPOSES UNTIL SCRIPTING IS FINISHED-
	EnableCrouchRoll = true;


	if ((TrInput & IN_FORWARD || TrInput & IN_BACK)
		&& (TrInput & IN_DUCK || Lara.keepDucked)
		&& Lara.gunStatus == LG_NO_ARMS
		&& Lara.waterStatus != LW_WADE
		|| Lara.waterSurfaceDist == 256
		&& !(Lara.waterSurfaceDist > 256))
	{

		if ((item->animNumber == LA_CROUCH_IDLE
			|| item->animNumber == LA_STAND_TO_CROUCH_END)
			&& !(TrInput & IN_FLARE || TrInput & IN_DRAW)
			&& (Lara.gunType != WEAPON_FLARE || Lara.flareAge < 900 && Lara.flareAge != 0))
		{
			Lara.torsoYrot = 0;
			Lara.torsoXrot = 0;
			item->goalAnimState = LS_CRAWL_IDLE;
		}

	}
	else if ((TrInput & IN_SPRINT) /*crouch roll*/
		&& (TrInput & IN_DUCK || Lara.keepDucked)
		&& Lara.gunStatus == LG_NO_ARMS
		&& Lara.waterStatus != LW_WADE
		|| Lara.waterSurfaceDist == 256
		&& !(Lara.waterSurfaceDist > 256)
		&& EnableCrouchRoll == true)
	{
		if (LaraFloorFront(item, item->pos.yRot, 1024) >= 384 ||  //4 clicks away from holes in the floor
			TestWall(item, 1024, 0, -256))			//4 clicks away from walls 
			return;

		if (!(TrInput & IN_FLARE || TrInput & IN_DRAW) //avoids some flare spawning/wep stuff
			&& (Lara.gunType != WEAPON_FLARE || Lara.flareAge < 900 && Lara.flareAge != 0))

		{
			Lara.torsoYrot = 0;
			Lara.torsoXrot = 0;
			item->goalAnimState = LS_CROUCH_ROLL;
		}
	}
}

void lara_as_crouch_roll(ITEM_INFO* item, COLL_INFO* coll)
{
	Camera.targetElevation = -ANGLE(20.0f);
	item->goalAnimState = LS_CROUCH_IDLE;
}


void lara_col_crouch_roll(ITEM_INFO* item, COLL_INFO* coll)
{
	Lara.isDucked = true;
	item->gravityStatus = false;
	item->fallspeed = 0;
	Lara.moveAngle = item->pos.yRot;
	coll->facing = item->pos.yRot;
	coll->badPos = STEPUP_HEIGHT;
	coll->badNeg = -STEPUP_HEIGHT;
	coll->badCeiling = 0;
	coll->slopesAreWalls = true;
	GetLaraCollisionInfo(item, coll);

	ShiftItem(item, coll);
	if (coll->midFloor != NO_HEIGHT)
		item->pos.yPos += coll->midFloor;
}


void lara_col_ducklr(ITEM_INFO* item, COLL_INFO* coll)//14534, 145E4 (F)
{
	// FIXED
	Lara.isDucked = true;
	if (TrInput & IN_LOOK)
		LookUpDown();

	item->gravityStatus = false;
	item->fallspeed = 0;

	Lara.moveAngle = item->pos.yRot;

	coll->facing = item->pos.yRot;
	coll->badPos = 384;
	coll->badNeg = -STEPUP_HEIGHT;
	coll->badCeiling = 0;
	coll->slopesAreWalls = true;

	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, 400);

	if (LaraFallen(item, coll))
	{
		Lara.gunStatus = LG_NO_ARMS;
	}
	else if (!TestLaraSlide(item, coll))
	{
		if (coll->midCeiling < -362)
			Lara.keepDucked = false;
		else
			Lara.keepDucked = true;

		ShiftItem(item, coll);

		if (coll->midFloor != NO_HEIGHT)
			item->pos.yPos += coll->midFloor;
	}
}

void lara_as_duckr(ITEM_INFO* item, COLL_INFO* coll) // (F) (D)
{
	coll->enableSpaz = false;
	if ((TrInput & (IN_DUCK | IN_LEFT)) != (IN_DUCK | IN_LEFT) || item->hitPoints <= 0) /* @ORIGINAL_BUG: the condition checks for IN_LEFT instead of IN_RIGHT */
		item->goalAnimState = LS_CROUCH_IDLE;
	item->pos.yRot += ANGLE(1.5f);
}

void lara_as_duckl(ITEM_INFO* item, COLL_INFO* coll) // (F) (D)
{
	coll->enableSpaz = false;
	if ((TrInput & (IN_DUCK | IN_LEFT)) != (IN_DUCK | IN_LEFT) || item->hitPoints <= 0)
		item->goalAnimState = LS_CROUCH_IDLE;
	item->pos.yRot -= ANGLE(1.5f);
}

int TestHangSwingIn(ITEM_INFO* item, short angle)//14104, 141B4 (F)
{
	int x = item->pos.xPos;
	int y = item->pos.yPos;
	int z = item->pos.zPos;
	short roomNum = item->roomNumber;
	FLOOR_INFO* floor;
	int h, c;

	//debug till scripting be ready
//	TR12_OSCILLATE_HANG = true;

	if (angle == ANGLE(180.0f))
	{
		z -= 256;
	}
	else if (angle == -ANGLE(90.0f))
	{
		x -= 256;
	}
	else if (angle == ANGLE(90.0f))
	{
		x += 256;
	}
	else if (angle == ANGLE(0.0f))
	{
		z += 256;
	}

	floor = GetFloor(x, y, z, &roomNum);
	h = GetFloorHeight(floor, x, y, z);
	c = GetCeiling(floor, x, y, z);

	if (h != NO_HEIGHT)
	{
		if (TR12_OSCILLATE_HANG == true)
		{
			if (((h - y) > 0)
				&& ((c - y) < -400))
				return(1);
		}
		else
		{
			if (((h - y) > 0)
				&& ((c - y) < -400)
				&& ((y - 819 - c) > -72))
				return(1);
		}
	}
	return(0);
}

int LaraHangLeftCornerTest(ITEM_INFO* item, COLL_INFO* coll) // (F) (D)
{
	if (item->animNumber != LA_REACH_TO_HANG && item->animNumber != LA_HANG_FEET_IDLE)
		return 0;

	if (coll->hitStatic)
		return 0;

	int x;
	int z;

	int oldXpos = item->pos.xPos;
	int oldZpos = item->pos.zPos;
	short oldYrot = item->pos.yRot;
	int oldFrontFloor = coll->frontFloor;

	short angle = (unsigned short) (item->pos.yRot + ANGLE(45.0f)) / ANGLE(90.0f);
	if (angle != NORTH && angle != SOUTH)
	{
		x = item->pos.xPos ^ (item->pos.xPos ^ item->pos.zPos) & 0x3FF;
		z = item->pos.zPos ^ (item->pos.xPos ^ item->pos.zPos) & 0x3FF;
	}
	else
	{
		x = (item->pos.xPos & 0xFFFFFC00) - (item->pos.zPos & 0x3FF) + SECTOR(1);
		z = (item->pos.zPos & 0xFFFFFC00) - (item->pos.xPos & 0x3FF) + SECTOR(1);
	}

	item->pos.xPos = x;
	Lara.cornerX = x;
	item->pos.zPos = z;
	Lara.cornerZ = z;
	item->pos.yRot -= ANGLE(90.0f);

	int result = -IsValidHangPos(item, coll);
	if (result)
	{
		if (Lara.climbStatus)
		{
			if (GetClimbTrigger(x, item->pos.yPos, z, item->roomNumber) & RightClimbTab[angle])
			{
				item->pos.xPos = oldXpos;
				item->pos.zPos = oldZpos;
				item->pos.yRot = oldYrot;
				Lara.moveAngle = oldYrot;
				return result;
			}
		}
		else
		{
			if (abs(oldFrontFloor - coll->frontFloor) <= 60)
			{
				item->pos.xPos = oldXpos;
				item->pos.zPos = oldZpos;
				item->pos.yRot = oldYrot;
				Lara.moveAngle = oldYrot;
				return result;
			}
		}
	}

	item->pos.xPos = oldXpos;
	item->pos.zPos = oldZpos;
	item->pos.yRot = oldYrot;
	Lara.moveAngle = oldYrot;

	if (LaraFloorFront(item, oldYrot - ANGLE(90.0f), 116) < 0)
		return 0;

	switch (angle)
	{
	case NORTH:
		x = ((item->pos.xPos ^ item->pos.zPos) & 0x3FF) ^ item->pos.xPos - SECTOR(1);
		z = ((item->pos.xPos ^ item->pos.zPos) & 0x3FF) ^ item->pos.zPos + SECTOR(1);
		break;

	case SOUTH:
		x = ((item->pos.xPos ^ item->pos.zPos) & 0x3FF) ^ (item->pos.xPos + SECTOR(1));
		z = ((item->pos.xPos ^ item->pos.zPos) & 0x3FF) ^ (item->pos.zPos - SECTOR(1));
		break;

	case WEST:
		x = (item->pos.xPos & 0xFFFFFC00) - (item->pos.zPos & 0x3FF);
		z = (item->pos.zPos & 0xFFFFFC00) - (item->pos.xPos & 0x3FF);
		break;

	default:
		x = ((item->pos.xPos + SECTOR(1)) & 0xFFFFFC00) - (item->pos.zPos & 0x3FF) + SECTOR(1);
		z = ((item->pos.zPos + SECTOR(1)) & 0xFFFFFC00) - (item->pos.xPos & 0x3FF) + SECTOR(1);
		break;

	}

	item->pos.xPos = x;
	Lara.cornerX = x;
	item->pos.zPos = z;
	Lara.cornerZ = z;
	item->pos.yRot += ANGLE(90.0f);

	result = IsValidHangPos(item, coll);
	if (!result)
	{
		item->pos.xPos = oldXpos;
		item->pos.zPos = oldZpos;
		item->pos.yRot = oldYrot;
		Lara.moveAngle = oldYrot;
		return result;
	}

	item->pos.xPos = oldXpos;
	item->pos.zPos = oldZpos;
	item->pos.yRot = oldYrot;
	Lara.moveAngle = oldYrot;

	if (!Lara.climbStatus)
	{
		if (abs(oldFrontFloor - coll->frontFloor) <= 60)
		{
			switch (angle)
			{
			case NORTH:
				if ((oldXpos & 0x3FF) > 512)
					result = 0;
				break;
			case EAST:
				if ((oldZpos & 0x3FF) < 512)
					result = 0;
				break;
			case SOUTH:
				if ((oldXpos & 0x3FF) < 512)
					result = 0;
				break;
			case WEST:
				if ((oldZpos & 0x3FF) > 512)
					result = 0;
				break;
			}
			return result;
		}
		return 0;
	}

	if (GetClimbTrigger(x, item->pos.yPos, z, item->roomNumber) & LeftClimbTab[angle])
		return result;

	short front = LaraFloorFront(item, item->pos.yRot, 116);
	if (abs(front - coll->frontFloor) > 60)
		return 0;

	if (front < -768)
		return 0;

	return result;
}

int LaraHangRightCornerTest(ITEM_INFO* item, COLL_INFO* coll) // (F) (D)
{
	if (item->animNumber != LA_REACH_TO_HANG && item->animNumber != LA_HANG_FEET_IDLE)
		return 0;

	if (coll->hitStatic)
		return 0;

	int x;
	int z;

	int oldXpos = item->pos.xPos;
	int oldZpos = item->pos.zPos;
	short oldYrot = item->pos.yRot;
	int oldFrontFloor = coll->frontFloor;

	short angle = (unsigned short) (item->pos.yRot + ANGLE(45.0f)) / ANGLE(90.0f);
	if (angle != NORTH && angle != SOUTH)
	{
		x = (item->pos.xPos & 0xFFFFFC00) - (item->pos.zPos & 0x3FF) + SECTOR(1);
		z = (item->pos.zPos & 0xFFFFFC00) - (item->pos.xPos & 0x3FF) + SECTOR(1);
	}
	else
	{
		x = item->pos.xPos ^ (item->pos.xPos ^ item->pos.zPos) & 0x3FF;
		z = item->pos.zPos ^ (item->pos.xPos ^ item->pos.zPos) & 0x3FF;
	}

	item->pos.xPos = x;
	Lara.cornerX = x;
	item->pos.zPos = z;
	Lara.cornerZ = z;
	item->pos.yRot += ANGLE(90.0f);

	int result = -IsValidHangPos(item, coll);
	if (result)
	{
		if (Lara.climbStatus)
		{
			if (GetClimbTrigger(x, item->pos.yPos, z, item->roomNumber) & LeftClimbTab[angle])
			{
				item->pos.xPos = oldXpos;
				item->pos.zPos = oldZpos;
				item->pos.yRot = oldYrot;
				Lara.moveAngle = oldYrot;
				return result;
			}
		}
		else
		{
			if (abs(oldFrontFloor - coll->frontFloor) <= 60)
			{
				item->pos.xPos = oldXpos;
				item->pos.zPos = oldZpos;
				item->pos.yRot = oldYrot;
				Lara.moveAngle = oldYrot;
				return result;
			}
		}
	}

	item->pos.xPos = oldXpos;
	item->pos.zPos = oldZpos;
	item->pos.yRot = oldYrot;
	Lara.moveAngle = oldYrot;

	if (LaraFloorFront(item, oldYrot + ANGLE(90.0f), 116) < 0)
		return 0;

	switch (angle)
	{
	case NORTH:
		x = ((item->pos.xPos + SECTOR(1)) & 0xFFFFFC00) - (item->pos.zPos & 0x3FF) + SECTOR(1);
		z = ((item->pos.zPos + SECTOR(1)) & 0xFFFFFC00) - (item->pos.xPos & 0x3FF) + SECTOR(1);
		break;

	case SOUTH:
		x = ((item->pos.xPos - SECTOR(1)) & 0xFFFFFC00) - (item->pos.zPos & 0x3FF) + SECTOR(1);
		z = ((item->pos.zPos - SECTOR(1)) & 0xFFFFFC00) - (item->pos.xPos & 0x3FF) + SECTOR(1);
		break;

	case WEST:
		x = (item->pos.xPos ^ item->pos.zPos) & 0x3FF ^ (item->pos.xPos - SECTOR(1));
		z = (item->pos.xPos ^ item->pos.zPos) & 0x3FF ^ (item->pos.zPos + SECTOR(1));
		break;

	default:
		x = ((item->pos.xPos ^ item->pos.zPos) & 0x3FF) ^ (item->pos.xPos + SECTOR(1));
		z = ((item->pos.xPos ^ item->pos.zPos) & 0x3FF) ^ (item->pos.zPos - SECTOR(1));
		break;

	}

	item->pos.xPos = x;
	Lara.cornerX = x;
	item->pos.zPos = z;
	Lara.cornerZ = z;
	item->pos.yRot -= ANGLE(90.0f);

	result = IsValidHangPos(item, coll);
	if (!result)
	{
		item->pos.xPos = oldXpos;
		item->pos.zPos = oldZpos;
		item->pos.yRot = oldYrot;
		Lara.moveAngle = oldYrot;
		return result;
	}

	item->pos.xPos = oldXpos;
	item->pos.zPos = oldZpos;
	item->pos.yRot = oldYrot;
	Lara.moveAngle = oldYrot;

	if (!Lara.climbStatus)
	{
		if (abs(oldFrontFloor - coll->frontFloor) <= 60)
		{
			switch (angle)
			{
			case NORTH:
				if ((oldXpos & 0x3FF) < 512)
					result = 0;
				break;
			case EAST:
				if ((oldZpos & 0x3FF) > 512)
					result = 0;
				break;
			case SOUTH:
				if ((oldXpos & 0x3FF) > 512)
					result = 0;
				break;
			case WEST:
				if ((oldZpos & 0x3FF) < 512)
					result = 0;
				break;
			}
			return result;
		}
		return 0;
	}

	if (GetClimbTrigger(x, item->pos.yPos, z, item->roomNumber) & RightClimbTab[angle])
		return result;

	short front = LaraFloorFront(item, item->pos.yRot, 116);
	if (abs(front - coll->frontFloor) > 60)
		return 0;

	if (front < -768)
		return 0;

	return result;
}

int IsValidHangPos(ITEM_INFO* item, COLL_INFO* coll) // (F) (D)
{
	if (LaraFloorFront(item, Lara.moveAngle, 100) < 200)
		return 0;

	short angle = (unsigned short) (item->pos.yRot + ANGLE(45.0f)) / ANGLE(90.0f);
	switch (angle)
	{
	case NORTH:
		item->pos.zPos += 4;
		break;
	case EAST:
		item->pos.xPos += 4;
		break;
	case SOUTH:
		item->pos.zPos -= 4;
		break;
	case WEST:
		item->pos.xPos -= 4;
		break;
	default:
		break;
	}

	coll->badPos = NO_BAD_POS;
	coll->badNeg = -512;
	coll->badCeiling = 0;

	Lara.moveAngle = item->pos.yRot;

	GetLaraCollisionInfo(item, coll);

	if (coll->midCeiling >= 0 || coll->collType != CT_FRONT || coll->hitStatic)
		return 0;

	return abs(coll->frontFloor - coll->rightFloor2) < 60;
}

void SnapLaraToEdgeOfBlock(ITEM_INFO* item, COLL_INFO* coll, short angle) // (F) (D)
{
	if (item->currentAnimState == LS_SHIMMY_RIGHT)
	{
		switch (angle)
		{
		case NORTH:
			item->pos.xPos = coll->old.x & 0xFFFFFF90 | 0x390;
			return;
		case EAST:
			item->pos.zPos = coll->old.z & 0xFFFFFC70 | 0x70;
			return;
		case SOUTH:
			item->pos.xPos = coll->old.x & 0xFFFFFC70 | 0x70;
			return;
		case WEST:
		default:
			item->pos.zPos = coll->old.z & 0xFFFFFF90 | 0x390;
			return;
		}
	}

	if (item->currentAnimState == LS_SHIMMY_LEFT)
	{
		switch (angle)
		{
		case NORTH:
			item->pos.xPos = coll->old.x & 0xFFFFFC70 | 0x70;
			return;
		case EAST:
			item->pos.zPos = coll->old.z & 0xFFFFFF90 | 0x390;
			return;
		case SOUTH:
			item->pos.xPos = coll->old.x & 0xFFFFFF90 | 0x390;
			return;
		case WEST:
		default:
			item->pos.zPos = coll->old.z & 0xFFFFFC70 | 0x70;
			return;
		}
	}

	if (item->currentAnimState == LS_SHIMMY_FEET_RIGHT)
	{
		switch (angle)
		{
		case NORTH:
			item->pos.xPos = coll->old.x & 0xFFFFFF90 | 0x720;
			return;
		case EAST:
			item->pos.zPos = coll->old.z & 0xFFFFFC70 | 0xE0;
			return;
		case SOUTH:
			item->pos.xPos = coll->old.x & 0xFFFFFC70 | 0xE0;
			return;
		case WEST:
		default:
			item->pos.zPos = coll->old.z & 0xFFFFFF90 | 0x720;
			return;
		}
	}

	if (item->currentAnimState == LS_SHIMMY_FEET_LEFT)
	{
		switch (angle)
		{
		case NORTH:
			item->pos.xPos = coll->old.x & 0xFFFFFC70 | 0xE0;
			return;
		case EAST:
			item->pos.zPos = coll->old.z & 0xFFFFFF90 | 0x720;
			return;
		case SOUTH:
			item->pos.xPos = coll->old.x & 0xFFFFFF90 | 0x720;
			return;
		case WEST:
		default:
			item->pos.zPos = coll->old.z & 0xFFFFFC70 | 0xE0;
			return;
		}
	}
}

int LaraTestHangOnClimbWall(ITEM_INFO* item, COLL_INFO* coll) // (F) (D)
{
	BOUNDING_BOX* bounds;
	int shift, result;

	if (Lara.climbStatus == 0)
		return 0;

	if (item->fallspeed < 0)
		return 0;

	switch ((unsigned short) (item->pos.yRot + ANGLE(45.0f)) / ANGLE(90.0f))
	{
	case NORTH:
	case SOUTH:
		item->pos.zPos += coll->shift.z;
		break;

	case EAST:
	case WEST:
		item->pos.xPos += coll->shift.x;
		break;

	default:
		break;
	}

	bounds = GetBoundsAccurate(item);

	if (Lara.moveAngle != item->pos.yRot)
	{
		short l = LaraCeilingFront(item, item->pos.yRot, 0, 0);
		short r = LaraCeilingFront(item, Lara.moveAngle, 128, 0);

		if (abs(l - r) > 60)
			return 0;
	}

	if (LaraTestClimbPos(item, coll->radius, coll->radius, bounds->Y1, bounds->Y2 - bounds->Y1, &shift) &&
		LaraTestClimbPos(item, coll->radius, -coll->radius, bounds->Y1, bounds->Y2 - bounds->Y1, &shift))
	{
		result = LaraTestClimbPos(item, coll->radius, 0, bounds->Y1, bounds->Y2 - bounds->Y1, &shift);
		if (result)
		{
			if (result != 1)
				item->pos.yPos += shift;
			return 1;
		}
	}

	return 0;
}

void LaraSlideEdgeJump(ITEM_INFO* item, COLL_INFO* coll)//12B18, 12BC8 (F)
{
	ShiftItem(item, coll);

	switch (coll->collType)
	{
	case CT_LEFT:
		item->pos.yRot += ANGLE(5.0f);
		break;

	case CT_RIGHT:
		item->pos.yRot -= ANGLE(5.0f);
		break;

	case CT_TOP:
	case CT_TOP_FRONT:
		if (item->fallspeed <= 0)
			item->fallspeed = 1;
		break;

	case CT_CLAMP:
		item->pos.zPos -= (400 * phd_cos(coll->facing)) >> W2V_SHIFT;
		item->pos.xPos -= (400 * phd_sin(coll->facing)) >> W2V_SHIFT;

		item->speed = 0;

		coll->midFloor = 0;

		if (item->fallspeed <= 0)
			item->fallspeed = 16;

		break;
	}
}

void LaraDeflectEdgeJump(ITEM_INFO* item, COLL_INFO* coll)//12904, 129B4 (F)
{
	ShiftItem(item, coll);

	switch (coll->collType)
	{
	case CT_FRONT:
	case CT_TOP_FRONT:
		if (!Lara.climbStatus || item->speed != 2)
		{
			if (coll->midFloor <= 512)
			{
				if (coll->midFloor <= 128)
				{
					item->goalAnimState = LS_GRAB_TO_FALL;
					item->currentAnimState = LS_GRAB_TO_FALL;

					item->animNumber = LA_JUMP_UP_LAND;
					item->frameNumber = g_Level.Anims[LA_JUMP_UP_LAND].frameBase;
				}
			}
			else
			{
				item->goalAnimState = LS_FREEFALL;
				item->currentAnimState = LS_FREEFALL;

				item->animNumber = LA_JUMP_WALL_SMASH_START;
				item->frameNumber = g_Level.Anims[LA_JUMP_WALL_SMASH_START].frameBase + 1;
			}

			item->speed /= 4;
			Lara.moveAngle -= ANGLE(180.0f);

			if (item->fallspeed <= 0)
				item->fallspeed = 1;
		}

		break;
	case CT_TOP:
		if (item->fallspeed <= 0)
			item->fallspeed = 1;

		break;
	case CT_LEFT:
		item->pos.yRot += ANGLE(5.0f);
		break;
	case CT_RIGHT:
		item->pos.yRot -= ANGLE(5.0f);
		break;
	case CT_CLAMP:
		item->pos.xPos -= (100 * 4 * phd_sin(coll->facing)) >> W2V_SHIFT;
		item->pos.zPos -= (100 * 4 * phd_cos(coll->facing)) >> W2V_SHIFT;

		item->speed = 0;
		coll->midFloor = 0;

		if (item->fallspeed <= 0)
			item->fallspeed = 16;

		break;
	}
}

void lara_slide_slope(ITEM_INFO* item, COLL_INFO* coll)//127BC, 1286C (F)
{
	coll->badPos = NO_BAD_POS;
	coll->badNeg = -512;
	coll->badCeiling = 0;

	GetLaraCollisionInfo(item, coll);

	if (!LaraHitCeiling(item, coll))
	{
		LaraDeflectEdge(item, coll);

		if (coll->midFloor <= 200)
		{
			TestLaraSlide(item, coll);

			item->pos.yPos += coll->midFloor;

			if (abs(coll->tiltX) <= 2 && abs(coll->tiltZ) <= 2)
			{
				if (TrInput & IN_FORWARD && item->currentAnimState != LS_SLIDE_BACK)
				{
					item->goalAnimState = LS_RUN_FORWARD;
				}
				else
				item->goalAnimState = LS_STOP;
				StopSoundEffect(SFX_LARA_SLIPPING);
			}
		}
		else
		{
			if (item->currentAnimState == LS_SLIDE_FORWARD)
			{
				item->animNumber = LA_FALL_START;
				item->frameNumber = g_Level.Anims[LA_FALL_START].frameBase;

				item->currentAnimState = LS_JUMP_FORWARD;
				item->goalAnimState = LS_JUMP_FORWARD;
			}
			else
			{
				item->animNumber = LA_FALL_BACK;
				item->frameNumber = g_Level.Anims[LA_FALL_BACK].frameBase;

				item->currentAnimState = LS_FALL_BACK;
				item->goalAnimState = LS_FALL_BACK;
			}

			StopSoundEffect(SFX_LARA_SLIPPING);

			item->gravityStatus = true;
			item->fallspeed = 0;
		}
	}
}

void LaraCollideStop(ITEM_INFO* item, COLL_INFO* coll)//126F0(<), 127A0(<) (F)
{
	switch (coll->oldAnimState)
	{
	case LS_STOP:
	case LS_TURN_RIGHT_SLOW:
	case LS_TURN_LEFT_SLOW:
	case LS_TURN_FAST:
		item->currentAnimState = coll->oldAnimState;
		item->animNumber = coll->oldAnimNumber;
		item->frameNumber = coll->oldFrameNumber;
		if (TrInput & IN_LEFT)
		{
			item->goalAnimState = LS_TURN_LEFT_SLOW;
		}
		else if (TrInput & IN_RIGHT)
		{
			item->goalAnimState = LS_TURN_RIGHT_SLOW;
		}
		else
		{
			item->goalAnimState = LS_STOP;
		}
		AnimateLara(item);
		break;
	default:
		item->animNumber = LA_STAND_SOLID;
		item->frameNumber = g_Level.Anims[LA_STAND_SOLID].frameBase;
		break;
	}
}

int TestWall(ITEM_INFO* item, int front, int right, int down)//12550, 12600 (F)
{
	int x = item->pos.xPos;
	int y = item->pos.yPos + down;
	int z = item->pos.zPos;

	short angle = (unsigned short) (item->pos.yRot + ANGLE(45.0f)) / ANGLE(90.0f);
	short roomNum = item->roomNumber;

	FLOOR_INFO* floor;
	int h, c;

	switch (angle)
	{
	case NORTH:
		x -= right;
		break;
	case EAST:
		z -= right;
		break;
	case SOUTH:
		x += right;
		break;
	case WEST:
		z += right;
		break;
	default:
		break;
	}

	GetFloor(x, y, z, &roomNum);

	switch (angle)
	{
	case NORTH:
		z += front;
		break;
	case EAST:
		x += front;
		break;
	case SOUTH:
		z -= front;
		break;
	case WEST:
		x -= front;
		break;
	default:
		break;
	}

	floor = GetFloor(x, y, z, &roomNum);
	h = GetFloorHeight(floor, x, y, z);
	c = GetCeiling(floor, x, y, z);

	if (h == NO_HEIGHT)
		return 1;

	if (y >= h || y <= c)
		return 2;

	return 0;
}

int LaraTestClimbStance(ITEM_INFO* item, COLL_INFO* coll)//11F78, 12028
{
	int shift_r, shift_l;

	if (LaraTestClimbPos(item, coll->radius, coll->radius + 120, -700, 512, &shift_r) != 1)
		return false;

	if (LaraTestClimbPos(item, coll->radius, -(coll->radius + 120), -700, 512, &shift_l) != 1)
		return false;

	if (shift_r)
	{
		if (shift_l)
		{
			if (shift_r < 0 != shift_l < 0)
				return false;

			if ((shift_r < 0 && shift_l < shift_r) ||
				(shift_r > 0 && shift_l > shift_r))
				/*if (SIGN(shift_r) == SIGN(shift_l) &&
					abs(shift_l) > abs(shift_r))*/
			{
				item->pos.yPos += shift_l;
				return true;
			}
		}

		item->pos.yPos += shift_r;
	}
	else if (shift_l)
	{
		item->pos.yPos += shift_l;
	}

	return true;
}

int LaraTestEdgeCatch(ITEM_INFO* item, COLL_INFO* coll, int* edge) // (F) (D)
{
	BOUNDING_BOX* bounds = GetBoundsAccurate(item);
	int hdif = coll->frontFloor - bounds->Y1;

	if (hdif < 0 == hdif + item->fallspeed < 0)
	{
		hdif = item->pos.yPos + bounds->Y1;

		if ((hdif + item->fallspeed & 0xFFFFFF00) != (hdif & 0xFFFFFF00))
		{
			if (item->fallspeed > 0)
				*edge = (hdif + item->fallspeed) & 0xFFFFFF00;
			else
				*edge = hdif & 0xFFFFFF00;

			return -1;
		}

		return 0;
	}

	if (abs(coll->leftFloor2 - coll->rightFloor2) >= SLOPE_DIF)
		return 0;

	return 1;
}

int LaraDeflectEdgeDuck(ITEM_INFO* item, COLL_INFO* coll) // (F) (D)
{
	if (coll->collType == CT_FRONT || coll->collType == CT_TOP_FRONT)
	{
		ShiftItem(item, coll);

		item->gravityStatus = false;
		item->speed = 0;

		return 1;
	}

	if (coll->collType == CT_LEFT)
	{
		ShiftItem(item, coll);
		item->pos.yRot += ANGLE(2.0f);
	}
	else if (coll->collType == CT_RIGHT)
	{
		ShiftItem(item, coll);
		item->pos.yRot -= ANGLE(2.0f);
	}

	return 0;
}

int LaraDeflectEdge(ITEM_INFO* item, COLL_INFO* coll) // (F) (D)
{
	if (coll->collType == CT_FRONT || coll->collType == CT_TOP_FRONT)
	{
		ShiftItem(item, coll);

		item->goalAnimState = LS_STOP;
		item->speed = 0;
		item->gravityStatus = false;

		return 1;
	}

	if (coll->collType == CT_LEFT)
	{
		ShiftItem(item, coll);
		item->pos.yRot += ANGLE(5.0f);
	}
	else if (coll->collType == CT_RIGHT)
	{
		ShiftItem(item, coll);
		item->pos.yRot -= ANGLE(5.0f);
	}

	return 0;
}

int LaraHitCeiling(ITEM_INFO* item, COLL_INFO* coll) // (F) (D)
{
	if (coll->collType == CT_TOP || coll->collType == CT_CLAMP)
	{
		item->pos.xPos = coll->old.x;
		item->pos.yPos = coll->old.y;
		item->pos.zPos = coll->old.z;

		item->goalAnimState = LS_STOP;
		item->currentAnimState = LS_STOP;

		item->animNumber = LA_STAND_SOLID;
		item->frameNumber = g_Level.Anims[item->animNumber].frameBase;

		item->speed = 0;
		item->fallspeed = 0;
		item->gravityStatus = false;

		return 1;
	}
	return 0;
}

int LaraLandedBad(ITEM_INFO* item, COLL_INFO* coll) // (F) (D)
{
	int landspeed = item->fallspeed - 140;

	if (landspeed > 0)
	{
		if (landspeed <= 14)
		{
			item->hitPoints -= 1000 * SQUARE(landspeed) / 196;
			return item->hitPoints <= 0;
		}
		else
		{
			item->hitPoints = -1;
			return 1;
		}
	}

	return 0;
}

int LaraFallen(ITEM_INFO* item, COLL_INFO* coll) // (F) (D)
{
	if (Lara.waterStatus == LW_WADE || coll->midFloor <= STEPUP_HEIGHT)
	{
		return 0;
	}

	item->animNumber = LA_FALL_START;
	item->currentAnimState = LS_JUMP_FORWARD;
	item->goalAnimState = LS_JUMP_FORWARD;
	item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
	item->fallspeed = 0;
	item->gravityStatus = true;
	return 1;
}

short LaraCeilingFront(ITEM_INFO* item, short ang, int dist, int h) // (F) (D)
{
	short room = item->roomNumber;

	int x = item->pos.xPos + ((dist * phd_sin(ang)) >> W2V_SHIFT);
	int y = item->pos.yPos - h;
	int z = item->pos.zPos + ((dist * phd_cos(ang)) >> W2V_SHIFT);

	int height = GetCeiling(GetFloor(x, y, z, &room), x, y, z);

	if (height != NO_HEIGHT)
		height += h - item->pos.yPos;

	return height;
}

short LaraFloorFront(ITEM_INFO* item, short ang, int dist) // (F) (D)
{
	short room = item->roomNumber;

	int x = item->pos.xPos + ((dist * phd_sin(ang)) >> W2V_SHIFT);
	int y = item->pos.yPos - 762;
	int z = item->pos.zPos + ((dist * phd_cos(ang)) >> W2V_SHIFT);

	int height = GetFloorHeight(GetFloor(x, y, z, &room), x, y, z);

	if (height != NO_HEIGHT)
		height -= item->pos.yPos;

	return height;
}

void GetLaraCollisionInfo(ITEM_INFO* item, COLL_INFO* coll) // (F) (D)
{
	coll->facing = Lara.moveAngle;
	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, LARA_HITE);
}

int TestLaraVault(ITEM_INFO* item, COLL_INFO* coll) // (F) (D)
{
	if (!(TrInput & IN_ACTION) || Lara.gunStatus != LG_NO_ARMS)
		return 0;

	// FOR DEBUG PURPOSES UNTIL SCRPTING IS FINISHED-
		EnableCrawlFlex1click = true;
		EnableCrawlFlex2click = true;
		EnableCrawlFlex3click = true;
		EnableMonkeyVault = true;



	if (coll->collType == CT_FRONT)
	{
		short angle = item->pos.yRot;
		if (angle >= -ANGLE(30.0f) && angle <= ANGLE(30.0f))
			angle = 0;
		else if (angle >= ANGLE(60.0f) && angle <= ANGLE(120.0f))
			angle = ANGLE(90.0f);
		else if (angle >= ANGLE(150.0f) || angle <= -ANGLE(150.0f))
			angle = ANGLE(180.0f);
		else if (angle >= -ANGLE(120.0f) && angle <= -ANGLE(60.0f))
			angle = -ANGLE(90.0f);

		if (angle & 0x3FFF)
			return 0;

		int slope = abs(coll->leftFloor2 - coll->rightFloor2) >= 60;

		if (coll->frontFloor < 0 && coll->frontFloor >= -256)
		{
			if (!slope && (abs(coll->frontCeiling - coll->frontFloor) < 256) && EnableCrawlFlex1click == true)
			{
				item->animNumber = LA_VAULT_TO_CROUCH_1CLICK;
				item->currentAnimState = LS_GRABBING;
				item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
				item->goalAnimState = LS_CROUCH_IDLE;
				item->pos.yPos += coll->frontFloor + 256;
				Lara.gunStatus = LG_HANDS_BUSY;
			}
		}
		else if (coll->frontFloor >= -640 && coll->frontFloor <= -384)
		{
			if (!slope &&
				coll->frontFloor - coll->frontCeiling >= 0 &&
				coll->leftFloor2 - coll->leftCeiling2 >= 0 &&
				coll->rightFloor2 - coll->rightCeiling2 >= 0)
			{
#if 0
				if (g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_SWAMP && Lara.waterSurfaceDist < -768)
					return 0;
#endif

				item->animNumber = LA_VAULT_TO_STAND_2CLICK_START;
				item->currentAnimState = LS_GRABBING;
				item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
				item->goalAnimState = LS_STOP;
				item->pos.yPos += coll->frontFloor + 512;
				Lara.gunStatus = LG_HANDS_BUSY;
			}
			else if((!slope && (abs(coll->frontCeiling - coll->frontFloor) < 256)) && EnableCrawlFlex2click == true)
			{
				item->animNumber = LA_VAULT_TO_CROUCH_2CLICK;
				item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
				item->currentAnimState = LS_GRABBING;
				item->goalAnimState = LS_CROUCH_IDLE;
				item->pos.yPos += coll->frontFloor + 512;
				Lara.gunStatus = LG_HANDS_BUSY;
			}
			else
			{
				return 0;
			}
		}
		else if (coll->frontFloor >= -896 && coll->frontFloor <= -640)
		{
			if (!slope &&
				coll->frontFloor - coll->frontCeiling >= 0 &&
				coll->leftFloor2 - coll->leftCeiling2 >= 0 &&
				coll->rightFloor2 - coll->rightCeiling2 >= 0)
			{
#if 0
				if (g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_SWAMP && Lara.waterSurfaceDist < -768)
					return 0;
#endif

				item->animNumber = LA_VAULT_TO_STAND_3CLICK;
				item->currentAnimState = LS_GRABBING;
				item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
				item->goalAnimState = LS_STOP;
				item->pos.yPos += coll->frontFloor + 768;
				Lara.gunStatus = LG_HANDS_BUSY;
			}
			else if ((!slope && (abs(coll->frontCeiling - coll->frontFloor) < 256) && EnableCrawlFlex3click == true ))
			{
				item->animNumber = LA_VAULT_TO_CROUCH_3CLICK;
				item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
				item->currentAnimState = LS_GRABBING;
				item->goalAnimState = LS_CROUCH_IDLE;
				item->pos.yPos += coll->frontFloor + 768;
				Lara.gunStatus = LG_HANDS_BUSY;
			}
			else
			{
				return 0;
			}
		}
		else if (!slope && coll->frontFloor >= -1920 && coll->frontFloor <= -896)
		{
#if 0
			if (g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_SWAMP)
				return 0;
#endif

			item->animNumber = LA_STAND_SOLID;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			item->goalAnimState = LS_JUMP_UP;
			item->currentAnimState = LS_STOP;
			Lara.calcFallSpeed = -3 - sqrt(-9600 - 12 * coll->frontFloor);
			AnimateLara(item);
		}
		else
		{
			if (!Lara.climbStatus)
				return 0;

			if (coll->frontFloor > -1920 || Lara.waterStatus == LW_WADE || coll->leftFloor2 > -1920 || coll->rightFloor2 > -2048 || coll->midCeiling > -1158)
			{
				if ((coll->frontFloor < -1024 || coll->frontCeiling >= 506) && coll->midCeiling <= -518)
				{
					ShiftItem(item, coll);

					if (LaraTestClimbStance(item, coll))
					{
						item->animNumber = LA_STAND_SOLID;
						item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
						item->goalAnimState = LS_LADDER_IDLE;
						item->currentAnimState = LS_STOP;
						AnimateLara(item);
						item->pos.yRot = angle;
						Lara.gunStatus = LG_HANDS_BUSY;
						return 1;
					}
				}
				return 0;
			}

			item->animNumber = LA_STAND_SOLID;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			item->goalAnimState = LS_JUMP_UP;
			item->currentAnimState = LS_STOP;
			Lara.calcFallSpeed = -116;
			AnimateLara(item);
		}

		item->pos.yRot = angle;
		ShiftItem(item, coll);

		short dir = (unsigned short)(item->pos.yRot + ANGLE(45.0f)) / ANGLE(90.0f);
		switch (dir)
		{
			case NORTH:
				item->pos.zPos = (item->pos.zPos | (WALL_SIZE - 1)) - LARA_RAD;
				break;

			case EAST:
				item->pos.xPos = (item->pos.xPos | (WALL_SIZE - 1)) - LARA_RAD;
				break;

			case SOUTH:
				item->pos.zPos = (item->pos.zPos & -WALL_SIZE) + LARA_RAD;
				break;

			case WEST:
				item->pos.xPos = (item->pos.xPos & -WALL_SIZE) + LARA_RAD;
				break;
		}
		return 1;
	}
	else if (EnableMonkeyVault == true)
	{
		if (Lara.canMonkeySwing)
		{
			FLOOR_INFO* F;
			int c, h;
			short roomNum = item->roomNumber;
			F = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNum);
			c = GetCeiling(F, item->pos.xPos, item->pos.yPos, item->pos.zPos);
			h = (c) - (item->pos.yPos);
			if (h > 1792 ||
				h < -1792 ||
				abs(h) == 768)
			{
				return 0;
			}
			item->animNumber = LA_STAND_IDLE;
			item->frameNumber = g_Level.Anims[LA_STAND_IDLE].frameBase;
			item->goalAnimState = LS_JUMP_UP;
			item->currentAnimState = LS_TEST_1;
			AnimateLara(item);
			return 1;
		}
	}
	else
	return 0;
}

void LaraClimbRope(ITEM_INFO* item, COLL_INFO* coll) // (F) (D)
{
	if (!(TrInput & IN_ACTION))
	{
		FallFromRope(item);
	}
	else
	{
		Camera.targetAngle = ANGLE(30.0f);
		if (Lara.ropeCount)
		{
			if (!Lara.ropeFlag)
			{
				--Lara.ropeCount;
				Lara.ropeOffset += Lara.ropeDownVel;
				if (!Lara.ropeCount)
					Lara.ropeFlag = 1;
				return;
			}
		}
		else
		{
			if (!Lara.ropeFlag)
			{
				ROPE_STRUCT* rope = &Ropes[Lara.ropePtr];
				Lara.ropeOffset = 0;
				Lara.ropeDownVel = (unsigned int) (rope->meshSegment[Lara.ropeSegment + 1].y - rope->meshSegment[Lara.ropeSegment].y) >> 17;
				Lara.ropeCount = 0;
				Lara.ropeOffset += Lara.ropeDownVel;
				Lara.ropeFlag = 1;
				return;
			}
		}

		if (item->animNumber == LA_ROPE_DOWN && item->frameNumber == g_Level.Anims[item->animNumber].frameEnd)
		{
			SoundEffect(SFX_LARA_ROPEDOWN_LOOP, &LaraItem->pos, 0);
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			Lara.ropeFlag = 0;
			++Lara.ropeSegment;
			Lara.ropeOffset = 0;
		}

		if (!(TrInput & IN_BACK) || Lara.ropeSegment >= 21)
			item->goalAnimState = LS_ROPE_IDLE;
	}
}

/*int GetLaraJointPos(PHD_VECTOR* vec, int mat)
{
#if !PSXPC_TEST///@FIXME excuse me but this doesn't work.
	MatrixPtr[0] = lara_joint_matrices[mat].m00;
	MatrixPtr[1] = lara_joint_matrices[mat].m01;
	MatrixPtr[2] = lara_joint_matrices[mat].m02;
	MatrixPtr[3] = lara_joint_matrices[mat].m10;
	MatrixPtr[4] = lara_joint_matrices[mat].m11;
	MatrixPtr[5] = lara_joint_matrices[mat].m12;
	MatrixPtr[6] = lara_joint_matrices[mat].m20;
	MatrixPtr[7] = lara_joint_matrices[mat].m21;
	MatrixPtr[8] = lara_joint_matrices[mat].m22;
	MatrixPtr[9] = lara_joint_matrices[mat].tx;
	MatrixPtr[10] = lara_joint_matrices[mat].ty;
	MatrixPtr[11] = lara_joint_matrices[mat].tz;

	mTranslateXYZ(vec->x, vec->y, vec->z);

	vec->x = phd_mxptr[3] >> W2V_SHIFT;
	vec->y = phd_mxptr[7] >> W2V_SHIFT; // todo this is wrong // todo actually not
	vec->z = phd_mxptr[11] >> W2V_SHIFT;

	vec->x += LaraItem->pos.xPos;
	vec->y += LaraItem->pos.yPos;
	vec->z += LaraItem->pos.zPos;

	mPopMatrix();
#endif
	return 48;
}*/

/*
void SetLaraUnderwaterNodes()//8596C(<), 879B0(<) (F) 
{
	return;//not used yet
	PHD_VECTOR joint;
	short roomNumber;//_18
	room_info* r;//$a1
	int flags;//$s1
	int current_joint;//$s0

	joint.x = 0;
	joint.y = 0;
	joint.z = 0;

	flags = 0;

	//loc_85988
	for (current_joint = 14; current_joint >= 0; current_joint--)
	{
		GetLaraJointPos(&joint, current_joint);

		roomNumber = LaraItem->roomNumber;
		GetFloor(joint.x, joint.y, joint.z, &roomNumber);

		r = &room[roomNumber];
		LaraNodeUnderwater[current_joint] = r->flags & RF_FILL_WATER;

		if (r->flags & RF_FILL_WATER)
		{
			Lara.wet[current_joint] = 0xFC;

			if (!(flags & 1))
			{
				flags |= 1;
				((int*)SRhandPtr)[3] = ((int*)& r->ambient)[0];
			}
		}
		else
		{
			//loc_85A1C
			if (!(flags & 2))
			{
				flags |= 2;
				((int*)SRhandPtr)[2] = ((int*)& r->ambient)[0];
			}
		}
	}
}

void SetPendulumVelocity(int x, int y, int z)// (F)
{
	if ((CurrentPendulum.node & 0xFFFFFFFE) < 24)
	{
		int val = 4096 / ((12 - (CurrentPendulum.node >> 1)) << 9 >> 8) << 8; // todo make this more beautiful

		x = (x * val) >> 16;
		y = (y * val) >> 16;
		z = (z * val) >> 16;
	}

	CurrentPendulum.Velocity.x += x;
	CurrentPendulum.Velocity.y += y;
	CurrentPendulum.Velocity.z += z;
}

void LaraClimbRope(ITEM_INFO* item, COLL_INFO* coll)
{
	UNIMPLEMENTED();
}

void FireChaff()
{
	UNIMPLEMENTED();
}

void GetLaraJointPosRot(PHD_VECTOR* a1, int a2, int a3, SVECTOR * a4)
{
	UNIMPLEMENTED();
}

void DoSubsuitStuff()
{
	UNIMPLEMENTED();
}*/

int TestLaraSlide(ITEM_INFO* item, COLL_INFO* coll) // (F) (D)
{
	if (abs(coll->tiltX) <= 2 && abs(coll->tiltZ) <= 2)
		return 0;

	short angle = ANGLE(0.0f);
	if (coll->tiltX > 2)                       		 
		angle = -ANGLE(90.0f);
	else if (coll->tiltX < -2)
		angle = ANGLE(90.0f);

	if (coll->tiltZ > 2 && coll->tiltZ > abs(coll->tiltX))
		angle = ANGLE(180.0f);
	else if (coll->tiltZ < -2 && -coll->tiltZ > abs(coll->tiltX))
		angle = ANGLE(0.0f);

	short delta = angle - item->pos.yRot;

	ShiftItem(item, coll);

	if (delta < -ANGLE(90.0f) || delta > ANGLE(90.0f))
	{
		if (item->currentAnimState == LS_SLIDE_BACK && OldAngle == angle)
			return 1;

		item->animNumber = LA_SLIDE_BACK_START;
		item->goalAnimState = LS_SLIDE_BACK;
		item->currentAnimState = LS_SLIDE_BACK;
		item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
		item->pos.yRot = angle + ANGLE(180.0f);
	}
	else
	{
		if (item->currentAnimState == LS_SLIDE_FORWARD && OldAngle == angle)
			return 1;

		item->animNumber = LA_SLIDE_FORWARD;
		item->goalAnimState = LS_SLIDE_FORWARD;
		item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
		item->currentAnimState = LS_SLIDE_FORWARD;
		item->pos.yRot = angle;
	}

	Lara.moveAngle = angle;
	OldAngle = angle;

	return 1;
}

int LaraHangTest(ITEM_INFO* item, COLL_INFO* coll) // (F) (D)
{
	int delta, flag, flag2, front, dfront, x, z, result;
	short angle, hdif, cdif, dir;
	ANIM_FRAME* frame;

	delta = 0;
	flag = 0;
	angle = Lara.moveAngle;
	if (angle == (short) (item->pos.yRot - ANGLE(90.0f)))
	{
		delta = -100;
	}
	else if (angle == (short) (item->pos.yRot + ANGLE(90.0f)))
	{
		delta = 100;
	}
	hdif = LaraFloorFront(item, angle, 100);
	if (hdif < 200)
		flag = 1;
	cdif = LaraCeilingFront(item, angle, 100, 0);
	dir = (unsigned short) (item->pos.yRot + ANGLE(45.0f)) / ANGLE(90.0f);
	switch (dir)
	{
	case NORTH:
		item->pos.zPos += 4;
		break;
	case EAST:
		item->pos.xPos += 4;
		break;
	case SOUTH:
		item->pos.zPos -= 4;
		break;
	case WEST:
		item->pos.xPos -= 4;
		break;
	}
	coll->badPos = NO_BAD_POS;
	coll->badNeg = -STEPUP_HEIGHT;
	coll->badCeiling = 0;
	Lara.moveAngle = item->pos.yRot;
	GetLaraCollisionInfo(item, coll);
	result = 0;
	if (Lara.climbStatus)
	{
		if (TrInput & IN_ACTION && item->hitPoints > 0)
		{
			Lara.moveAngle = angle;
			if (!LaraTestHangOnClimbWall(item, coll))
			{
				if (item->animNumber != LA_LADDER_TO_HANG_RIGHT && item->animNumber != LA_LADDER_TO_HANG_LEFT)
				{
					SnapLaraToEdgeOfBlock(item, coll, dir);
					item->pos.yPos = coll->old.y;
					item->currentAnimState = LS_HANG;
					item->goalAnimState = LS_HANG;
					item->animNumber = LA_REACH_TO_HANG;
					item->frameNumber = g_Level.Anims[item->animNumber].frameBase + 21;
				}
				result = 1;
			}
			else
			{
				if (item->animNumber == LA_REACH_TO_HANG && item->frameNumber == g_Level.Anims[LA_REACH_TO_HANG].frameBase + 21 && LaraTestClimbStance(item, coll))
					item->goalAnimState = LS_LADDER_IDLE;
			}
		}
		else
		{
			item->animNumber = LA_FALL_START;
			item->currentAnimState = LS_JUMP_FORWARD;
			item->goalAnimState = LS_JUMP_FORWARD;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			item->pos.yPos += 256;
			item->gravityStatus = true;
			item->speed = 2;
			item->fallspeed = 1;
			Lara.gunStatus = LG_NO_ARMS;
		}
	}
	else
	{
		if (TrInput & IN_ACTION && item->hitPoints > 0 && coll->frontFloor <= 0)
		{
			if (flag && hdif > 0 && delta > 0 == coll->leftFloor > coll->rightFloor)
				flag = 0;
			frame = (ANIM_FRAME*) GetBoundsAccurate(item);
			front = coll->frontFloor;
			dfront = coll->frontFloor - frame->boundingBox.Y1;
			flag2 = 0;
			x = item->pos.xPos;
			z = item->pos.zPos;
			switch (dir)
			{
			case NORTH:
				x += delta;
				break;
			case EAST:
				z -= delta;
				break;
			case SOUTH:
				x -= delta;
				break;
			case WEST:
				z += delta;
				break;
			}
			Lara.moveAngle = angle;
			if (256 << dir & GetClimbTrigger(x, item->pos.yPos, z, item->roomNumber))
			{
				if (!LaraTestHangOnClimbWall(item, coll))
					dfront = 0;
			}
			else if (abs(coll->leftFloor2 - coll->rightFloor2) >= 60)
			{
				if (delta < 0 && coll->leftFloor2 != coll->frontFloor || delta > 0 && coll->rightFloor2 != coll->frontFloor)
					flag2 = 1;
			}
			coll->frontFloor = front;
			if (!flag2 && coll->midCeiling < 0 && coll->collType == CT_FRONT && !flag && !coll->hitStatic && cdif <= -950 && dfront >= -60 && dfront <= 60)
			{
				switch (dir)
				{
				case NORTH:
				case SOUTH:
					item->pos.zPos += coll->shift.z;
					break;
				case EAST:
				case WEST:
					item->pos.xPos += coll->shift.x;
					break;
				}
				item->pos.yPos += dfront;
			}
			else
			{
				item->pos.xPos = coll->old.x;
				item->pos.yPos = coll->old.y;
				item->pos.zPos = coll->old.z;
				if (item->currentAnimState == LS_SHIMMY_LEFT || item->currentAnimState == LS_SHIMMY_RIGHT)
				{
					item->currentAnimState = LS_HANG;
					item->goalAnimState = LS_HANG;
					item->animNumber = LA_REACH_TO_HANG;
					item->frameNumber = g_Level.Anims[item->animNumber].frameBase + 21;
				}
				else if (item->currentAnimState == LS_SHIMMY_FEET_LEFT || item->currentAnimState == LS_SHIMMY_FEET_RIGHT)
					{
						item->currentAnimState = LS_HANG_FEET;
						item->goalAnimState = LS_HANG_FEET;
						item->animNumber = LA_HANG_FEET_IDLE;
						item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
					}
				result = 1;
			}
		}
		else
		{
			item->currentAnimState = LS_JUMP_UP;
			item->goalAnimState = LS_JUMP_UP;
			item->animNumber = LA_JUMP_UP;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase + 9;
			frame = (ANIM_FRAME*) GetBoundsAccurate(item);
			item->pos.xPos += coll->shift.x;
			item->pos.yPos += frame->boundingBox.Y2;
			item->pos.zPos += coll->shift.z;
			item->gravityStatus = true;
			item->speed = 2;
			item->fallspeed = 1;
			Lara.gunStatus = LG_NO_ARMS;
		}
	}
	return result;
}
/**********************

Current problems with with the feet hanging:
-going around corners isn't working. code is commented for now.
-obviously, not all animations were made yet- we still need crouch pull up (works well, tested with placeholder anim) and corner anims and handstand (not tested)

***********************/
int TestHangFeet(ITEM_INFO* item, short angle)
{
	if (Lara.climbStatus)
		return 0;

//	EnableFeetHang = true;


	if (!EnableFeetHang)
		return 0;

	int x = item->pos.xPos;
	int y = item->pos.yPos;
	int z = item->pos.zPos;
	short roomNum = item->roomNumber;
	FLOOR_INFO* floor;
	int h, c, g, m, j;

	if (angle == ANGLE(180.0f))
	{
		z -= 256;
	}
	else if (angle == -ANGLE(90.0f))
	{
		x -= 256;
	}
	else if (angle == ANGLE(90.0f))
	{
		x += 256;
	}
	else if (angle == ANGLE(0.0f))
	{
		z += 256;
	}

	floor = GetFloor(x, y, z, &roomNum);
	h = GetFloorHeight(floor, x, y, z);
	c = GetCeiling(floor, x, y, z);
	g = h - y;
	m = c - y;
	j = y - 128 - c;

	if (item->currentAnimState == LS_CRAWL_TO_HANG)
	{
		if (h != NO_HEIGHT)
		{
			if (((g) > 0)
				&& ((m) < -128)
				&& ((j) > -72))
				return(1);
		}
		return(0);
	}
	else
	{
	if (h != NO_HEIGHT)
	{
		if (((g) > 0)
			&& ((m) < -128)
			&& ((j) > -72))
			return(0);
	}
	return(1);
	}
}

void lara_as_hang_feet(ITEM_INFO* item, COLL_INFO* coll)
{
	Lara.isClimbing = false;

	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_STOP;
		return;
	}

	if (TrInput & IN_LOOK)
		LookUpDown();

	coll->enableBaddiePush = false;
	coll->enableSpaz = false;
	Camera.targetAngle = 0;
	Camera.targetElevation = -ANGLE(45.0f);
}
void lara_col_hang_feet(ITEM_INFO* item, COLL_INFO* coll)
{
	item->fallspeed = 0;
	item->gravityStatus = false;

	Lara.moveAngle = item->pos.yRot;

	LaraHangTest(item, coll);

	if (!(TrInput & IN_ACTION))
		item->goalAnimState = LS_JUMP_UP;

	if (item->animNumber == LA_HANG_FEET_IDLE)
	{
		int flag;

		if (TrInput & IN_LEFT || TrInput & IN_LSTEP)
		{
			if (CanLaraHangSideways(item, coll, -ANGLE(90.0f)))
			{
				item->goalAnimState = LS_SHIMMY_FEET_LEFT;
				return;
			}
			flag = LaraHangLeftCornerTest(item, coll);
			if (flag != 0)
			{
				if (flag <= 0)
					item->goalAnimState = LS_SHIMMY_FEET_INNER_LEFT;
				else
					item->goalAnimState = LS_SHIMMY_FEET_OUTER_LEFT;
				return;
			}

		}


		if (TrInput & IN_RIGHT || TrInput & IN_RSTEP)
		{
			if (CanLaraHangSideways(item, coll, ANGLE(90.0f)))
			{
				item->goalAnimState = LS_SHIMMY_FEET_RIGHT;

				return;
			}
			flag = LaraHangRightCornerTest(item, coll);
			if (flag != 0)
			{
				if (flag <= 0)
					item->goalAnimState = LS_SHIMMY_FEET_INNER_RIGHT;
				else
					item->goalAnimState = LS_SHIMMY_FEET_OUTER_RIGHT;
				return;
			}
		}


		TestForObjectOnLedge(item, coll);
		if (TrInput & IN_FORWARD)
		{
			if (coll->frontFloor > -850)
			{
				if (coll->frontFloor < -650 &&
					coll->frontFloor >= coll->frontCeiling &&
					coll->frontFloor >= coll->leftCeiling2 &&
					coll->frontFloor >= coll->rightCeiling2)
				{
					if (abs(coll->leftFloor2 - coll->rightFloor2) < 60 && !coll->hitStatic)
					{
						if (TrInput & IN_WALK)
						{
							//till anim							item->goalAnimState = LS_HANDSTAND;
						}
						else if (TrInput & IN_DUCK)
						{
							item->goalAnimState = LS_HANG_TO_CRAWL;
							item->requiredAnimState = LS_CROUCH_IDLE;
						}
						else
						{
							item->goalAnimState = LS_GRABBING;
						}
						return;
					}
				}
			}
			if (coll->frontFloor < -650 &&
				coll->frontFloor - coll->frontCeiling >= -256 &&
				coll->frontFloor - coll->leftCeiling2 >= -256 &&
				coll->frontFloor - coll->rightCeiling2 >= -256)
			{
				if (abs(coll->leftFloor2 - coll->rightFloor2) < 60 && !coll->hitStatic)
				{
					item->goalAnimState = LS_HANG_TO_CRAWL;
					item->requiredAnimState = LS_CROUCH_IDLE;
					return;
				}
			}
		}
		/*
			if (Lara.climbStatus != 0 &&
				coll->midCeiling <= -256 &&
				abs(coll->leftCeiling2 - coll->rightCeiling2) < 60)
			{
				if (LaraTestClimbStance(item, coll))
				{
					item->goalAnimState = LS_LADDER_IDLE;
				}
				else
				{
					item->animNumber = LA_LADDER_UP_HANDS;
					item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
					item->goalAnimState = LS_HANG;
					item->currentAnimState = LS_HANG;
				}
			}
			return;
					}*///commenting till daniel makes anims

		Lara.moveAngle = item->pos.yRot;
		LaraHangTest(item, coll);
	}
}


void lara_as_hang_feet_shimmyr(ITEM_INFO* item, COLL_INFO* coll)
{
		coll->enableBaddiePush = false;
		coll->enableSpaz = false;
		Camera.targetAngle = 0;
		Camera.targetElevation = -ANGLE(45.0f);

		if (!(TrInput & (IN_RIGHT | IN_RSTEP)))
			item->goalAnimState = LS_HANG_FEET;
}

void lara_col_hang_feet_shimmyr(ITEM_INFO* item, COLL_INFO* coll)
{
	Lara.moveAngle = item->pos.yRot + ANGLE(90.0f);
	coll->radius = LARA_RAD;
	LaraHangTest(item, coll);
	Lara.moveAngle = item->pos.yRot + ANGLE(90.0f);
}

void lara_as_hang_feet_shimmyl(ITEM_INFO* item, COLL_INFO* coll)
{

	coll->enableBaddiePush = false;
	coll->enableSpaz = false;
	Camera.targetAngle = 0;
	Camera.targetElevation = -ANGLE(45.0f);
	if (!(TrInput & (IN_LEFT | IN_LSTEP)))
		item->goalAnimState = LS_HANG_FEET;
}

void lara_col_hang_feet_shimmyl(ITEM_INFO* item, COLL_INFO* coll)
{
	Lara.moveAngle = item->pos.yRot - ANGLE(90.0f);
	coll->radius = LARA_RAD;
	LaraHangTest(item, coll);
	Lara.moveAngle = item->pos.yRot - ANGLE(90.0f);
}

void lara_as_hang_feet_inRcorner(ITEM_INFO* item, COLL_INFO* coll)
{
	Camera.laraNode = 8;
	Camera.targetElevation = ANGLE(33.0f);
	SetCornerAnimFeet(item, coll, ANGLE(90.0f),
		item->animNumber = LA_SHIMMY_FEET_RIGHT_CORNER_INNER);
}
void lara_as_hang_feet_inLcorner(ITEM_INFO* item, COLL_INFO* coll)
{
	Camera.laraNode = 8;
	Camera.targetElevation = ANGLE(33.0f);
	SetCornerAnimFeet(item, coll, -ANGLE(90.0f),
		item->animNumber = LA_SHIMMY_FEET_LEFT_CORNER_INNER);
}
void lara_as_hang_feet_outRcorner(ITEM_INFO* item, COLL_INFO* coll)
{
	Camera.laraNode = 8;
	Camera.targetElevation = ANGLE(33.0f);
	SetCornerAnimFeet(item, coll, -ANGLE(90.0f),
		item->animNumber = LA_SHIMMY_FEET_RIGHT_CORNER_OUTER);
}
void lara_as_hang_feet_outLcorner(ITEM_INFO* item, COLL_INFO* coll)
{
	Camera.laraNode = 8;
	Camera.targetElevation = ANGLE(33.0f);
	SetCornerAnimFeet(item, coll, ANGLE(90.0f),
		item->animNumber = LA_SHIMMY_FEET_LEFT_CORNER_OUTER);
}

void SetCornerAnimFeet(ITEM_INFO* item, COLL_INFO* coll, short rot, short flip)
{
	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_JUMP_FORWARD;
		item->currentAnimState = LS_JUMP_FORWARD;
		item->animNumber = LA_FALL_START;
		item->frameNumber = g_Level.Anims[item->animNumber].frameBase;

		item->gravityStatus = true;
		item->speed = 2;
		item->pos.yPos += 256;
		item->fallspeed = 1;

		Lara.gunStatus = LG_NO_ARMS;

		item->pos.yRot += rot / 2;
	}
	else if (flip)
	{

			item->animNumber = LA_HANG_TO_HANG_FEET;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase + 24;
			item->goalAnimState = LS_HANG_FEET;
			item->currentAnimState = LS_HANG_FEET;

		coll->old.x = Lara.cornerX;
		item->pos.xPos = Lara.cornerX;

		coll->old.z = Lara.cornerZ;
		item->pos.zPos = Lara.cornerZ;

		item->pos.yRot += rot;
	}
}