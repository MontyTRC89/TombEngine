#include "framework.h"
#include "Objects/Generic/Switches/switch.h"
#include "Game/items.h"
#include "Game/control/lot.h"
#include "Game/effects/debris.h"
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

void ProcessExplodingSwitchType8(ItemInfo* item) 
{
	auto pos = Vector3Int();
	GetJointAbsPosition(item, &pos, 0);

	TestTriggers(pos.x, pos.y, pos.z, item->RoomNumber, true);
	ExplodeItemNode(item, Objects[item->ObjectNumber].nmeshes - 1, 0, 64);
	item->MeshBits |= 1 << ((Objects[item->ObjectNumber].nmeshes & 0xFF) - 2);
}

void InitialiseShootSwitch(short itemNumber)
{
	auto* switchItem = &g_Level.Items[itemNumber];

	if (switchItem->TriggerFlags == 444)
		switchItem->MeshBits &= ~(1 << (Objects[switchItem->ObjectNumber].nmeshes - 2));
}

void ShootSwitchCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
{
	auto* switchItem = &g_Level.Items[itemNumber];

	if (switchItem->ObjectNumber == ID_SHOOT_SWITCH1 && !(switchItem->MeshBits & 1))
		switchItem->Status = ITEM_INVISIBLE;
}
