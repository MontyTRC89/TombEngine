#include "framework.h"
#include "Objects/Generic/Switches/switch.h"
#include "Game/items.h"
#include "Game/control/lot.h"
#include "Objects/Generic/Object/objects.h"
#include "Game/Lara/lara.h"
#include "Game/animation.h"
#include "Game/collision/sphere.h"
#include "Game/camera.h"
#include "Specific/setup.h"
#include "Specific/level.h"
#include "Specific/input.h"
#include "Sound/sound.h"

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
