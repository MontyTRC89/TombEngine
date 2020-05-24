#include "framework.h"
#include "misc.h"
#include "setup.h"
#include "level.h"

short GF(short animIndex, short frameToStart)
{
	return short(Anims[animIndex].frameBase + frameToStart);
}

short GF2(short objectID, short animIndex, short frameToStart)
{
	return short(Anims[Objects[objectID].animIndex + animIndex].frameBase + frameToStart);
}

CREATURE_INFO* GetCreatureInfo(ITEM_INFO* item)
{
    return (CREATURE_INFO*)item->data;
}

void GetRoomList(short roomNumber, short* roomArray, short* numRooms)
{
	short numDoors, *door, adjoiningRoom;
	int i, j;
	bool adjoiningRoomFound;

	roomArray[0] = roomNumber;
	door = Rooms[roomNumber].door;
	if (door)
	{
		numDoors = *door;
		door++;

		for (i = 0; i < numDoors; i++)
		{
			adjoiningRoom = *door;
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

			door += 16;
		}
	}
}

void GetRoomList(short roomNumber, vector<short>* destRoomList)
{
	vector<short> roomList;
	short numDoors, *door, adjoiningRoom;
	int i, j;
	bool adjoiningRoomFound;

	roomList.push_back(roomNumber);
	door = Rooms[roomNumber].door;
	if (door)
	{
		numDoors = *door;
		door++;

		for (i = 0; i < numDoors; i++)
		{
			adjoiningRoom = *door;
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

			door += 16;
		}
	}

	*destRoomList = roomList;
}