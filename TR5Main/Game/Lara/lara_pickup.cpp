#include "framework.h"
#include "lara.h"

void lara_as_pickup(ITEM_INFO* item, COLL_INFO* coll)//1AB00(<), 1AC34(<) (F)
{
	/*state 39, 98*/
	/*collision: lara_default_col*/
	Lara.look = false;
	coll->enableBaddiePush = false;
	coll->enableSpaz = false;
	Camera.targetAngle = -ANGLE(130.0f);
	Camera.targetElevation = -ANGLE(15.0f);
	Camera.targetDistance = SECTOR(1);
}

void lara_as_pickupflare(ITEM_INFO* item, COLL_INFO* coll)//1AB5C(<), 1AC90(<) (F)
{
	/*state 67*/
	/*collison: lara_default_col*/
	Lara.look = false;
	coll->enableBaddiePush = false;
	coll->enableSpaz = false;
	Camera.targetAngle = ANGLE(130.0f);
	Camera.targetElevation = -ANGLE(15.0f);
	Camera.targetDistance = SECTOR(1);
	if (item->frameNumber == g_Level.Anims[item->animNumber].frameEnd - 1)
		Lara.gunStatus = LG_NO_ARMS;
}
