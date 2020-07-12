#include "framework.h"
#include "misc.h"
#include "setup.h"
#include "level.h"
#include "lara.h"
#include "draw.h"
using std::vector;
short GF(short animIndex, short frameToStart)
{
	return Anims[animIndex].frameBase + frameToStart;
}

short GF2(short objectID, short animIndex, short frameToStart)
{
	return Anims[Objects[objectID].animIndex + animIndex].frameBase + frameToStart;
}

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
	for (int i = 0; i < NumItems; i++)
	{
		target = &Items[i];
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
	if (*numRooms <= 0)
		return;
	short numDoors, * door, adjoiningRoom;
	int i, j;
	bool adjoiningRoomFound;

	roomArray[0] = roomNumber;
	ROOM_INFO* room = &Rooms[roomNumber];

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
			roomArray[*(numRooms++)] = adjoiningRoom;
	}
}

void GetRoomList(short roomNumber, vector<short>* destRoomList)
{
	vector<short> roomList;
	short adjoiningRoom;
	int i, j;
	bool adjoiningRoomFound;

	roomList.push_back(roomNumber);
	ROOM_INFO* room = &Rooms[roomNumber];

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