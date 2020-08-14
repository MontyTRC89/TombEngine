#include "framework.h"
#include "Lara.h"
#include "lara_basic.h"
#include "lara_tests.h"
#include "lara_monkey.h"
#include "lara_crawl.h"
#include "lara_objects.h"
#include "lara_hang.h"
#include "lara_slide.h"
#include "inventory.h"
#include "lara_fire.h"
#include "lara_surface.h"
#include "lara_swim.h"
#include "lara_one_gun.h"
#include "lara_two_guns.h"
#include "laramisc.h"
#include "lara_climb.h"

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
LaraInfo Lara;
ITEM_INFO* LaraItem;
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
