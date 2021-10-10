#include "framework.h"
#include "misc.h"
#include "setup.h"
#include "level.h"
#include "lara.h"
#include "animation.h"
#include "itemdata/creature_info.h"
#include "items.h"

using std::vector;
CREATURE_INFO* GetCreatureInfo(ITEM_INFO* item)
{
    return (CREATURE_INFO*)item->data;
}

void TargetNearestEntity(ITEM_INFO* item, CREATURE_INFO* creature)
{
	ITEM_INFO* target;
	int bestdistance;
	int distance;
	int x, z;

	bestdistance = MAXINT;
	for (int i = 0; i < g_Level.NumItems; i++)
	{
		target = &g_Level.Items[i];
		if (target != nullptr)
		{
			if (target != item && target->hitPoints > 0 && target->status != ITEM_INVISIBLE)
			{
				x = target->pos.xPos - item->pos.xPos;
				z = target->pos.zPos - item->pos.zPos;
				distance = SQUARE(z) + SQUARE(x);
				if (distance < bestdistance)
				{
					creature->enemy = target;
					bestdistance = distance;
				}
			}
		}
	}
}

void GetRoomList(short roomNumber, short* roomArray, short* numRooms)
{
	short numDoors, * door, adjoiningRoom;
	int i, j;
	bool adjoiningRoomFound;

	roomArray[0] = roomNumber;
	*numRooms = 1;

	ROOM_INFO* room = &g_Level.Rooms[roomNumber];

	for (i = 0; i < room->doors.size(); i++)
	{
		adjoiningRoom = room->doors[i].room;
		adjoiningRoomFound = false;

		for (j = 0; j < *numRooms; j++)
		{
			if (roomArray[i] == adjoiningRoom)
			{
				adjoiningRoomFound = true;
				break;
			}
		}

		if (!adjoiningRoomFound)
		{
			roomArray[*numRooms] = adjoiningRoom;
			*numRooms = *numRooms + 1;
		}
	}
}

void GetRoomList(short roomNumber, vector<short>* destRoomList)
{
	vector<short> roomList;
	short adjoiningRoom;
	int i, j;
	bool adjoiningRoomFound;

	roomList.push_back(roomNumber);
	ROOM_INFO* room = &g_Level.Rooms[roomNumber];

	for (i = 0; i < room->doors.size(); i++)
	{
		adjoiningRoom = room->doors[i].room;
		adjoiningRoomFound = false;

		for (j = 0; j < roomList.size(); j++)
		{
			if (roomList[i] == adjoiningRoom)
			{
				adjoiningRoomFound = true;
				break;
			}
		}

		if (!adjoiningRoomFound)
			roomList.push_back(adjoiningRoom);

	}

	*destRoomList = roomList;
}