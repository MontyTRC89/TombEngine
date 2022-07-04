#include "framework.h"
#include "Objects/TR5/Shatter/tr5_smashobject.h"
#include "Specific/level.h"
#include "Game/control/box.h"
#include "Sound/sound.h"
#include "Game/effects/tomb4fx.h"
#include "Game/items.h"

void InitialiseSmashObject(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];
	item->Flags = 0;
	item->MeshBits = 1;

	auto* room = &g_Level.Rooms[item->RoomNumber];

	FloorInfo* floor = GetSector(room, item->Pose.Position.x - room->x, item->Pose.Position.z - room->z);
	auto* box = &g_Level.Boxes[floor->Box];
	if (box->flags & 0x8000)
		box->flags |= BLOCKED;
}

void SmashObject(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];
	auto* room = &g_Level.Rooms[item->RoomNumber];

	int sector = ((item->Pose.Position.z - room->z) / 1024) + room->zSize * ((item->Pose.Position.x - room->x) / 1024);

	auto* box = &g_Level.Boxes[room->floor[sector].Box];
	if (box->flags & 0x8000)
		box->flags &= ~BOX_BLOCKED;

	SoundEffect(SFX_TR5_SMASH_GLASS, &item->Pose);

	item->Collidable = 0;
	item->MeshBits = 0xFFFE;

	ExplodingDeath(itemNumber, BODY_EXPLODE | BODY_NO_BOUNCE);

	item->Flags |= IFLAG_INVISIBLE;

	if (item->Status == ITEM_ACTIVE)
		RemoveActiveItem(itemNumber);

	item->Status = ITEM_DEACTIVATED;
}

void SmashObjectControl(short itemNumber)
{
	SmashObject(itemNumber * 65536);
}
