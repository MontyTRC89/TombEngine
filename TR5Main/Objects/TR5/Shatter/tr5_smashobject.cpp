#include "framework.h"
#include "tr5_smashobject.h"
#include "level.h"
#include "box.h"
#include "Sound\sound.h"
#include "effects\tomb4fx.h"
#include "items.h"
#include "Specific\trmath.h"

void InitialiseSmashObject(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];
	item->flags = 0;
	item->meshBits = 1;

	ROOM_INFO* r = &g_Level.Rooms[item->roomNumber];
	FLOOR_INFO* floor = XZ_GET_SECTOR(r, item->pos.xPos - r->x, item->pos.zPos - r->z);
	BOX_INFO* box = &g_Level.Boxes[floor->box];
	if (box->flags & 0x8000)
		box->flags |= BLOCKED;
}

void SmashObject(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];
	ROOM_INFO* r = &g_Level.Rooms[item->roomNumber];
	int sector = ((item->pos.zPos - r->z) / 1024) + r->xSize * ((item->pos.xPos - r->x) / 1024);

	BOX_INFO* box = &g_Level.Boxes[r->floor[sector].box];
	if (box->flags & 0x8000)
		box->flags &= ~BOX_BLOCKED;

	SoundEffect(SFX_TR5_SMASH_GLASS, &item->pos, 0);

	item->collidable = 0;
	item->meshBits = 0xFFFE;

	ExplodingDeath(itemNumber, -1, 257);

	item->flags |= IFLAG_INVISIBLE;

	if (item->status == ITEM_ACTIVE)
		RemoveActiveItem(itemNumber);
	item->status = ITEM_DEACTIVATED;
}

void SmashObjectControl(short itemNumber)
{
	SmashObject(itemNumber * 65536);
}