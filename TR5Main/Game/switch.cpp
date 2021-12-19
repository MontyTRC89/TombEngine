#include "framework.h"
#include "switch.h"
#include "items.h"
#include "control/lot.h"
#include "objects.h"
#include "Lara.h"
#include "animation.h"
#include "collision/sphere.h"
#include "camera.h"
#include "setup.h"
#include "level.h"
#include "input.h"
#include "Sound\sound.h"

// NOTE: we need to decompile/inspect if these functions are still needed

void ProcessExplodingSwitchType8(ITEM_INFO* item) 
{
	PHD_VECTOR pos;
	pos.x = 0;
	pos.y = 0;
	pos.z = 0;
	GetJointAbsPosition(item, &pos, 0);
	TestTriggers(pos.x, pos.y, pos.z, item->roomNumber, true);
	ExplodeItemNode(item, Objects[item->objectNumber].nmeshes - 1, 0, 64);
	item->meshBits |= 1 << ((Objects[item->objectNumber].nmeshes & 0xFF) - 2);
}

void InitialiseShootSwitch(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];
	if (item->triggerFlags == 444)
		item->meshBits &= ~(1 << (Objects[item->objectNumber].nmeshes - 2));
}

void ShootSwitchCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* coll)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	if (item->objectNumber == ID_SHOOT_SWITCH1 && !(item->meshBits & 1))
		item->status = ITEM_INVISIBLE;
}
