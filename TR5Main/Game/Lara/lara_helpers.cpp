#include "framework.h"
#include "collide.h"
#include "items.h"
#include "level.h"
#include "lara.h"

bool IsStandingWeapon(LARA_WEAPON_TYPE gunType)
{
	if (gunType == WEAPON_SHOTGUN
		|| gunType == WEAPON_HK
		|| gunType == WEAPON_CROSSBOW
		|| gunType == WEAPON_TORCH
		|| gunType == WEAPON_GRENADE_LAUNCHER
		|| gunType == WEAPON_HARPOON_GUN
		|| gunType == WEAPON_ROCKET_LAUNCHER
		|| gunType == WEAPON_SNOWMOBILE)
	{
		return true;
	}

	return false;
}

// TODO: State dispatch to a new LS_FALL state. The issue is that goal states set in collision functions are only actuated on the following
// frame, resulting in an unacceptable delay. Changing the order in which routine functions are executed is not a viable solution. @Sezz 2021.09.26
void SetLaraFallState(ITEM_INFO* item)
{
	item->animNumber = LA_FALL_START;
	item->currentAnimState = LS_JUMP_FORWARD;
	item->goalAnimState = LS_JUMP_FORWARD;
	item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
	item->fallspeed = 0;
	item->gravityStatus = true;
}

// TODO: Get true slope direction for enhanced sliding mechanics.
// Krys, I'd like to have a look at what you did in your TRNG script. @Sezz 2021.09.26
short GetLaraSlideDirection(COLL_INFO* coll)
{
	short laraAngle = ANGLE(0.0f);

	if (coll->TiltX > 2)
		laraAngle = -ANGLE(90.0f);
	else if (coll->TiltX < -2)
		laraAngle = ANGLE(90.0f);

	if (coll->TiltZ > 2 && coll->TiltZ > abs(coll->TiltX))
		laraAngle = ANGLE(180.0f);
	else if (coll->TiltZ < -2 && -coll->TiltZ > abs(coll->TiltX))
		laraAngle = ANGLE(0.0f);

	return laraAngle;
}

// TODO: State dispatches to slide states. Same issue as with SetLaraFallState().
void SetLaraSlideState(ITEM_INFO* item, COLL_INFO* coll)
{
	auto angle = GetLaraSlideDirection(coll);
	auto polarity = angle - item->pos.yRot;

	ShiftItem(item, coll);

	// Slide back.
	if (polarity < -ANGLE(90.0f) || polarity > ANGLE(90.0f))
	{
		Lara.moveAngle = ANGLE(180);
		item->pos.yRot = angle + ANGLE(180.0f);

		item->animNumber = LA_SLIDE_BACK_START;
		item->goalAnimState = LS_SLIDE_BACK;
		item->currentAnimState = LS_SLIDE_BACK;
		item->frameNumber = g_Level.Anims[item->animNumber].frameBase;

	}
	// Slide forward.
	else [[likely]]
	{
		Lara.moveAngle = 0;
		item->pos.yRot = angle;

		item->animNumber = LA_SLIDE_FORWARD;
		item->goalAnimState = LS_SLIDE_FORWARD;
		item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
		item->currentAnimState = LS_SLIDE_FORWARD;
	}
}