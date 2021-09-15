#include "framework.h"
#include "flipmap.h"
#include "lot.h"
#include "Renderer11.h"

using namespace TEN::Renderer;

byte FlipStatus = 0;
int FlipStats[MAX_FLIPMAP];
int FlipMap[MAX_FLIPMAP];


void DoFlipMap(short group)
{
	ROOM_INFO temp;

	for (size_t i = 0; i < g_Level.Rooms.size(); i++)
	{
		ROOM_INFO* r = &g_Level.Rooms[i];

		if (r->flippedRoom >= 0 && r->flipNumber == group)
		{
			RemoveRoomFlipItems(r);

			ROOM_INFO* flipped = &g_Level.Rooms[r->flippedRoom];

			temp = *r;
			*r = *flipped;
			*flipped = temp;

			r->flippedRoom = flipped->flippedRoom;
			flipped->flippedRoom = -1;

			r->itemNumber = flipped->itemNumber;
			r->fxNumber = flipped->fxNumber;

			AddRoomFlipItems(r);

			g_Renderer.flipRooms(static_cast<short>(i), r->flippedRoom);

			for (auto& fd : r->floor)
				fd.Room = i;
			for (auto& fd : flipped->floor)
				fd.Room = r->flippedRoom;
		}
	}

	int status = FlipStats[group] == 0;
	FlipStats[group] = status;
	FlipStatus = status;

	for (int i = 0; i < ActiveCreatures.size(); i++)
	{
		ActiveCreatures[i]->LOT.targetBox = NO_BOX;
	}
}

void AddRoomFlipItems(ROOM_INFO* r)
{
	for (short linkNum = r->itemNumber; linkNum != NO_ITEM; linkNum = g_Level.Items[linkNum].nextItem)
	{
		ITEM_INFO* item = &g_Level.Items[linkNum];

		//if (item->objectNumber == ID_RAISING_BLOCK1 && item->itemFlags[1])
		//	AlterFloorHeight(item, -1024);

		if (item->objectNumber == ID_RAISING_BLOCK2)
		{
			//if (item->itemFlags[1])
			//	AlterFloorHeight(item, -2048);
		}
	}
}

void RemoveRoomFlipItems(ROOM_INFO* r)
{
	for (short linkNum = r->itemNumber; linkNum != NO_ITEM; linkNum = g_Level.Items[linkNum].nextItem)
	{
		ITEM_INFO* item = &g_Level.Items[linkNum];

		if (item->flags & 0x100 && Objects[item->objectNumber].intelligent && item->hitPoints <= 0 && item->hitPoints != NOT_TARGETABLE)
		{
			KillItem(linkNum);
		}
	}
}
