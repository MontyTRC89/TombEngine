#include "framework.h"
#include "Game/items.h"

#include "Game/control/control.h"
#include "Game/collision/floordata.h"
#include "Game/effects/effects.h"
#include "Game/Lara/lara.h"
#include "Specific/setup.h"
#include "Specific/level.h"

using namespace TEN::Floordata;

void ClearItem(short itemNum)
{
	ITEM_INFO* item = &g_Level.Items[itemNum];
	ROOM_INFO* room = &g_Level.Rooms[item->RoomNumber];

	item->Collidable = true;
	item->Data = nullptr;
	item->StartPosition = item->Position;
}

void KillItem(short itemNum)
{
	if (InItemControlLoop)
	{
		ItemNewRooms[2 * ItemNewRoomNo] = itemNum | 0x8000;
		ItemNewRoomNo++;
	}
	else// if (NextItemActive != NO_ITEM)
	{
		ITEM_INFO* item = &g_Level.Items[itemNum];

		DetatchSpark(itemNum, SP_ITEM);

		item->Active = false;

		if (NextItemActive == itemNum)
		{
			NextItemActive = item->NextActive;
		}
		else
		{
			short linknum;
			for (linknum = NextItemActive; linknum != NO_ITEM; linknum = g_Level.Items[linknum].NextActive)
			{
				if (g_Level.Items[linknum].NextActive == itemNum)
				{
					g_Level.Items[linknum].NextActive = item->NextActive;
					break;
				}
			}
		}

		if (item->RoomNumber != NO_ROOM)
		{
			if (g_Level.Rooms[item->RoomNumber].itemNumber == itemNum)
			{
				g_Level.Rooms[item->RoomNumber].itemNumber = item->NextItem;
			}
			else
			{
				short linknum;
				for (linknum = g_Level.Rooms[item->RoomNumber].itemNumber; linknum != NO_ITEM; linknum = g_Level.Items[linknum].NextItem)
				{
					if (g_Level.Items[linknum].NextItem == itemNum)
					{
						g_Level.Items[linknum].NextItem = item->NextItem;
						break;
					}
				}
			}
		}

		if (item == Lara.target)
			Lara.target = NULL;

		if (Objects[item->ObjectNumber].floor != nullptr)
			UpdateBridgeItem(itemNum, true);

		if (itemNum >= g_Level.NumItems)
		{
			item->NextItem = NextItemFree;
			NextItemFree = itemNum;
		}
		else
		{
			item->Flags |= IFLAG_KILLED;
		}
	}
}

void RemoveAllItemsInRoom(short roomNumber, short objectNumber)
{
	ROOM_INFO* room = &g_Level.Rooms[roomNumber];
	short currentItemNum = room->itemNumber;

	while (currentItemNum != NO_ITEM)
	{
		ITEM_INFO* item = &g_Level.Items[currentItemNum];

		if (item->ObjectNumber == objectNumber)
		{
			RemoveActiveItem(currentItemNum);
			item->Status = ITEM_NOT_ACTIVE;
			item->Flags &= 0xC1;
		}

		currentItemNum = item->NextItem;
	}
}

void AddActiveItem(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	item->Flags |= 0x20;

	if (Objects[item->ObjectNumber].control == NULL)
	{
		item->Status = ITEM_NOT_ACTIVE;
		return;
	}

	if (!item->Active)
	{
		item->Active = true;
		item->NextActive = NextItemActive;
		NextItemActive = itemNumber;
	}
}

void ItemNewRoom(short itemNumber, short roomNumber)
{
	if (InItemControlLoop)
	{
		ItemNewRooms[2 * ItemNewRoomNo] = itemNumber;
		ItemNewRooms[2 * ItemNewRoomNo + 1] = roomNumber;
		ItemNewRoomNo++;
	}
	else
	{
		ITEM_INFO* item = &g_Level.Items[itemNumber];

		if (item->RoomNumber != NO_ROOM)
		{
			ROOM_INFO* r = &g_Level.Rooms[item->RoomNumber];

			if (r->itemNumber == itemNumber)
			{
				r->itemNumber = item->NextItem;
			}
			else
			{
				for (short linknum = r->itemNumber; linknum != -1; linknum = g_Level.Items[linknum].NextItem)
				{
					if (g_Level.Items[linknum].NextItem == itemNumber)
					{
						g_Level.Items[linknum].NextItem = item->NextItem;
						break;
					}
				}
			}
		}

		item->RoomNumber = roomNumber;
		item->NextItem = g_Level.Rooms[roomNumber].itemNumber;
		g_Level.Rooms[roomNumber].itemNumber = itemNumber;
	}
}

void EffectNewRoom(short fxNumber, short roomNumber)
{
	if (InItemControlLoop)
	{
		ItemNewRooms[2 * ItemNewRoomNo] = fxNumber;
		ItemNewRooms[2 * ItemNewRoomNo + 1] = roomNumber;
		ItemNewRoomNo++;
	}
	else
	{
		FX_INFO* fx = &EffectList[fxNumber];
		ROOM_INFO* r = &g_Level.Rooms[fx->roomNumber];

		if (r->fxNumber == fxNumber)
		{
			r->fxNumber = fx->nextFx;
		}
		else
		{
			short linknum;
			for (linknum = r->fxNumber; linknum != -1; linknum = EffectList[linknum].nextFx)
			{
				if (EffectList[linknum].nextFx == fxNumber)
				{
					EffectList[linknum].nextFx = fx->nextFx;
					break;
				}
			}
		}

		fx->roomNumber = roomNumber;
		fx->nextFx = g_Level.Rooms[roomNumber].fxNumber;
		g_Level.Rooms[roomNumber].fxNumber = fxNumber;
	}
}

void KillEffect(short fxNumber)
{
	if (InItemControlLoop)
	{
		ItemNewRooms[2 * ItemNewRoomNo] = fxNumber | 0x8000;
		ItemNewRoomNo++;
	}
	else
	{
		FX_INFO* fx = &EffectList[fxNumber];
		DetatchSpark(fxNumber, SP_FX);

		if (NextFxActive == fxNumber)
		{
			NextFxActive = fx->nextActive;
		}
		else
		{
			for (short linknum = NextFxActive; linknum != NO_ITEM; linknum = EffectList[linknum].nextActive)
			{
				if (EffectList[linknum].nextActive == fxNumber)
				{
					EffectList[linknum].nextActive = fx->nextActive;
					break;
				}
			}
		}

		if (g_Level.Rooms[fx->roomNumber].fxNumber == fxNumber)
		{
			g_Level.Rooms[fx->roomNumber].fxNumber = fx->nextFx;
		}
		else
		{
			for (short linknum = g_Level.Rooms[fx->roomNumber].fxNumber; linknum != NO_ITEM; linknum = EffectList[linknum].nextFx)
			{
				if (EffectList[linknum].nextFx == fxNumber)
				{
					EffectList[linknum].nextFx = fx->nextFx;
					break;
				}
			}
		}

		fx->nextFx = NextFxFree;
		NextFxFree = fxNumber;
	}
}

short CreateNewEffect(short roomNum) 
{
	short fxNumber = NextFxFree;

	if (NextFxFree != NO_ITEM)
	{
		FX_INFO* fx = &EffectList[NextFxFree];
		NextFxFree = fx->nextFx;
		ROOM_INFO* r = &g_Level.Rooms[roomNum];
		fx->roomNumber = roomNum;
		fx->nextFx = r->fxNumber;
		r->fxNumber = fxNumber;
		fx->nextActive = NextFxActive;
		NextFxActive = fxNumber;
		fx->shade = GRAY555;
	}

	return fxNumber;
}

void InitialiseFXArray(int allocmem)
{

	FX_INFO* fx;

	NextFxActive = NO_ITEM;
	NextFxFree = 0;
	for (int i = 0; i < NUM_EFFECTS; i++)
	{
		fx = &EffectList[i];
		fx->nextFx = i + 1;
	}
	EffectList[NUM_EFFECTS - 1].nextFx = NO_ITEM;
}

void RemoveDrawnItem(short itemNum) 
{
	ITEM_INFO* item = &g_Level.Items[itemNum];

	if (g_Level.Rooms[item->RoomNumber].itemNumber == itemNum)
	{
		g_Level.Rooms[item->RoomNumber].itemNumber = item->NextItem;
	}
	else
	{
		for (short linknum = g_Level.Rooms[item->RoomNumber].itemNumber; linknum != NO_ITEM; linknum = g_Level.Items[linknum].NextItem)
		{
			if (g_Level.Items[linknum].NextItem == itemNum)
			{
				g_Level.Items[linknum].NextItem = item->NextItem;
				break;
			}
		}
	}
}

void RemoveActiveItem(short itemNum) 
{
	if (g_Level.Items[itemNum].Active)
	{
		g_Level.Items[itemNum].Active = false;

		if (NextItemActive == itemNum)
		{
			NextItemActive = g_Level.Items[itemNum].NextActive;
		}
		else
		{
			for (short linknum = NextItemActive; linknum != NO_ITEM; linknum = g_Level.Items[linknum].NextActive)
			{
				if (g_Level.Items[linknum].NextActive == itemNum)
				{
					g_Level.Items[linknum].NextActive = g_Level.Items[itemNum].NextActive;
					break;
				}
			}
		}
	}
}

void InitialiseItem(short itemNum) 
{
	ITEM_INFO* item = &g_Level.Items[itemNum];

	item->AnimNumber = Objects[item->ObjectNumber].animIndex;
	item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;

	item->RequiredState = 0;
	item->TargetState = g_Level.Anims[item->AnimNumber].ActiveState;
	item->ActiveState = g_Level.Anims[item->AnimNumber].ActiveState;

	item->Position.zRot = 0;
	item->Position.xRot = 0;

	item->VerticalVelocity = 0;
	item->Velocity = 0;

	item->ItemFlags[3] = 0;
	item->ItemFlags[2] = 0;
	item->ItemFlags[1] = 0;
	item->ItemFlags[0] = 0;

	item->Active = false;
	item->Status = ITEM_NOT_ACTIVE;
	item->Airborne = false;
	item->HitStatus = false;
	item->Collidable = true;
	item->LookedAt = false;
	item->Poisoned = false;
	item->AIBits = 0;

	item->Timer = 0;

	item->HitPoints = Objects[item->ObjectNumber].HitPoints;

	if (item->ObjectNumber == ID_HK_ITEM ||
		item->ObjectNumber == ID_HK_AMMO_ITEM ||
		item->ObjectNumber == ID_CROSSBOW_ITEM ||
		item->ObjectNumber == ID_REVOLVER_ITEM)
	{
		item->MeshBits = 1;
	}
	else
	{
		item->MeshBits = -1;
	}

	item->TouchBits = 0;
	item->AfterDeath = 0;
	item->FiredWeapon = 0;
	item->SwapMeshFlags = 0;

	if (item->Flags & IFLAG_INVISIBLE)
	{
		item->Flags &= ~IFLAG_INVISIBLE;
		item->Status = ITEM_INVISIBLE;
	}
	else if (Objects[item->ObjectNumber].intelligent)
	{
		item->Status = ITEM_INVISIBLE;
	}

	if ((item->Flags & IFLAG_ACTIVATION_MASK) == IFLAG_ACTIVATION_MASK)
	{
		item->Flags &= ~IFLAG_ACTIVATION_MASK;
		item->Flags |= IFLAG_REVERSE;
		AddActiveItem(itemNum);
		item->Status = ITEM_ACTIVE;
	}

	ROOM_INFO* r = &g_Level.Rooms[item->RoomNumber];

	item->NextItem = r->itemNumber;
	r->itemNumber = itemNum;

	FLOOR_INFO* floor = GetSector(r, item->Position.xPos - r->x, item->Position.zPos - r->z);
	item->Floor = floor->FloorHeight(item->Position.xPos, item->Position.zPos);
	item->BoxNumber = floor->Box;

	if (Objects[item->ObjectNumber].nmeshes > 0)
	{
		item->Mutator.resize(Objects[item->ObjectNumber].nmeshes);
		for (int i = 0; i < item->Mutator.size(); i++)
			item->Mutator[i] = {};
	}
	else
		item->Mutator.clear();

	if (Objects[item->ObjectNumber].initialise != NULL)
		Objects[item->ObjectNumber].initialise(itemNum);
}

short CreateItem()
{
	short itemNum = 0;

	if (NextItemFree == -1) return NO_ITEM;

	itemNum = NextItemFree;
	g_Level.Items[NextItemFree].Flags = 0;
	g_Level.Items[NextItemFree].LuaName = "";
	NextItemFree = g_Level.Items[NextItemFree].NextItem;

	return itemNum;
}

void InitialiseItemArray(int numitems)
{
	ITEM_INFO* item = &g_Level.Items[g_Level.NumItems];

	NextItemActive = NO_ITEM;
	NextItemFree = g_Level.NumItems;

	if (g_Level.NumItems + 1 < numitems)
	{
		for (int i = g_Level.NumItems + 1; i < numitems; i++, item++)
		{
			item->NextItem = i;
			item->Active = false;
		}
	}

	item->NextItem = NO_ITEM;
}

short SpawnItem(ITEM_INFO* item, GAME_OBJECT_ID objectNumber)
{
	short itemNumber = CreateItem();
	if (itemNumber != NO_ITEM)
	{
		ITEM_INFO* spawn = &g_Level.Items[itemNumber];

		spawn->ObjectNumber = objectNumber;
		spawn->RoomNumber = item->RoomNumber;
		memcpy(&spawn->Position, &item->Position, sizeof(PHD_3DPOS));

		InitialiseItem(itemNumber);

		spawn->Status = ITEM_NOT_ACTIVE;
		spawn->Shade = 0x4210;
	}

	return itemNumber;
}

int GlobalItemReplace(short search, GAME_OBJECT_ID replace)
{
	int changed = 0;
	for (int i = 0; i < g_Level.Rooms.size(); i++)
	{
		ROOM_INFO* room = &g_Level.Rooms[i];
		for (short itemNumber = room->itemNumber; itemNumber != NO_ITEM; itemNumber = g_Level.Items[itemNumber].NextItem)
		{
			if (g_Level.Items[itemNumber].ObjectNumber == search)
			{
				g_Level.Items[itemNumber].ObjectNumber = replace;
				changed++;
			}
		}
	}

	return changed;
}

// Offset values are used to account for the fact that room traversal can ONLY occur at portals.
// TODO: There is one edge case offsets don't fix. @Sezz 2022.02.02
void UpdateItemRoom(ITEM_INFO* item, int height, int xOffset, int zOffset)
{
	float c = phd_cos(item->Position.yRot);
	float s = phd_sin(item->Position.yRot);

	int x = item->Position.xPos + roundf(c * xOffset + s * zOffset);
	int y = height + item->Position.yPos;
	int z = item->Position.zPos + roundf(-s * xOffset + c * zOffset);
	item->Location = GetRoom(item->Location, x, y, z);
	item->Floor = GetFloorHeight(item->Location, x, z).value_or(NO_HEIGHT);

	if (item->RoomNumber != item->Location.roomNumber)
		ItemNewRoom(FindItem(item), item->Location.roomNumber);
}

std::vector<int> FindAllItems(short objectNumber)
{
	std::vector<int> itemList;

	for (int i = 0; i < g_Level.NumItems; i++)
	{
		if (g_Level.Items[i].ObjectNumber == objectNumber)
			itemList.push_back(i);
	}

	return itemList;
}

ITEM_INFO* FindItem(int object_number)
{
	ITEM_INFO* item;

	for (int i = 0; i < g_Level.NumItems; i++)
	{
		item = &g_Level.Items[i];

		if (item->ObjectNumber == object_number)
			return item;
	}

	return 0;
}

int FindItem(ITEM_INFO* item)
{
	if (item == LaraItem)
		return Lara.itemNumber;

	for (int i = 0; i < g_Level.NumItems; i++)
		if (item == &g_Level.Items[i])
			return i;

	return -1;
}
