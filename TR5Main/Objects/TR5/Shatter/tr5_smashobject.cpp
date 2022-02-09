#include "framework.h"
#include "Objects/TR5/Shatter/tr5_smashobject.h"
#include "Specific/level.h"
#include "Game/control/box.h"
#include "Sound/sound.h"
#include "Game/effects/tomb4fx.h"
#include "Game/items.h"

void InitialiseSmashObject(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];
	item->Flags = 0;
	item->MeshBits = 1;

	ROOM_INFO* r = &g_Level.Rooms[item->RoomNumber];
	FLOOR_INFO* floor = GetSector(r, item->Position.xPos - r->x, item->Position.zPos - r->z);
	BOX_INFO* box = &g_Level.Boxes[floor->Box];
	if (box->flags & 0x8000)
		box->flags |= BLOCKED;
}

void SmashObject(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];
	ROOM_INFO* r = &g_Level.Rooms[item->RoomNumber];
	int sector = ((item->Position.zPos - r->z) / 1024) + r->zSize * ((item->Position.xPos - r->x) / 1024);

	BOX_INFO* box = &g_Level.Boxes[r->floor[sector].Box];
	if (box->flags & 0x8000)
		box->flags &= ~BOX_BLOCKED;

	SoundEffect(SFX_TR5_SMASH_GLASS, &item->Position, 0);

	item->Collidable = 0;
	item->MeshBits = 0xFFFE;

	ExplodingDeath(itemNumber, -1, 257);

	item->Flags |= IFLAG_INVISIBLE;

	if (item->Status == ITEM_ACTIVE)
		RemoveActiveItem(itemNumber);
	item->Status = ITEM_DEACTIVATED;
}

void SmashObjectControl(short itemNumber)
{
	SmashObject(itemNumber * 65536);
}