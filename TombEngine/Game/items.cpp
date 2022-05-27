#include "framework.h"
#include "Game/items.h"

#include "Game/control/control.h"
#include "Game/collision/floordata.h"
#include "Game/effects/effects.h"
#include "Game/Lara/lara.h"
#include "ScriptInterfaceGame.h"
#include "Specific/setup.h"
#include "Specific/level.h"
#include "Objects/ScriptInterfaceObjectsHandler.h"

using namespace TEN::Floordata;

void ClearItem(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];
	auto* room = &g_Level.Rooms[item->RoomNumber];

	item->Collidable = true;
	item->Data = nullptr;
	item->StartPose = item->Pose;
}

void KillItem(short const itemNumber)
{
	if (InItemControlLoop)
	{
		ItemNewRooms[2 * ItemNewRoomNo] = itemNumber | 0x8000;
		ItemNewRoomNo++;
	}
	else
	{
		auto* item = &g_Level.Items[itemNumber];

		DetatchSpark(itemNumber, SP_ITEM);

		item->Active = false;

		ActiveItems.erase(
			std::remove(
				ActiveItems.begin(),
				ActiveItems.end(),
				itemNumber),
			ActiveItems.end());

		if (item->RoomNumber != NO_ROOM)
		{
			g_Level.Rooms[item->RoomNumber].Items.erase(
				std::remove(
					g_Level.Rooms[item->RoomNumber].Items.begin(),
					g_Level.Rooms[item->RoomNumber].Items.end(),
					itemNumber),
				g_Level.Rooms[item->RoomNumber].Items.end());
		}

		if (item == Lara.TargetEntity)
			Lara.TargetEntity = NULL;

		if (Objects[item->ObjectNumber].floor != nullptr)
			UpdateBridgeItem(itemNumber, true);

		if (itemNumber >= g_Level.NumItems)
		{
			NextItemFree = itemNumber;
		}
		else
		{
			item->Flags |= IFLAG_KILLED;
		}

		g_GameScriptEntities->NotifyKilled(item);
		g_GameScriptEntities->TryRemoveColliding(itemNumber, true);
		if (!item->luaCallbackOnKilledName.empty())
		{
			g_GameScript->ExecuteFunction(item->luaCallbackOnKilledName, itemNumber);
		}
	}
}

void RemoveAllItemsInRoom(short roomNumber, short objectNumber)
{
	auto* room = &g_Level.Rooms[roomNumber];

	for (short currentItemNumber : room->Items)
	{
		auto* item = &g_Level.Items[currentItemNumber];

		if (item->ObjectNumber == objectNumber)
		{
			RemoveActiveItem(currentItemNumber);
			item->Status = ITEM_NOT_ACTIVE;
			item->Flags &= 0xC1;
		}
	}
}

void AddActiveItem(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	item->Flags |= 0x20;

	if (Objects[item->ObjectNumber].control == NULL)
	{
		item->Status = ITEM_NOT_ACTIVE;
	}
	else if (std::find(ActiveItems.begin(), ActiveItems.end(), itemNumber) == ActiveItems.end())
	{
		item->Active = true;
		ActiveItems.push_back(itemNumber);
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
		auto* item = &g_Level.Items[itemNumber];

		if (item->RoomNumber != NO_ROOM)
		{
			auto* oldRoom = &g_Level.Rooms[item->RoomNumber];
			oldRoom->Items.erase(
				std::remove(
					oldRoom->Items.begin(),
					oldRoom->Items.end(),
					itemNumber),
				oldRoom->Items.end());
		}

		item->RoomNumber = roomNumber;
		g_Level.Rooms[roomNumber].Items.push_back(itemNumber);
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
		auto* fx = &EffectList[fxNumber];

		if (fx->RoomNumber != NO_ROOM)
		{
			auto* oldRoom = &g_Level.Rooms[fx->RoomNumber];
			oldRoom->Effects.erase(
				std::remove(
					oldRoom->Effects.begin(),
					oldRoom->Effects.end(),
					fxNumber),
				oldRoom->Effects.end());
		}

		fx->RoomNumber = roomNumber;
		g_Level.Rooms[roomNumber].Effects.push_back(fxNumber);
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
		auto* fx = &EffectList[fxNumber];

		DetatchSpark(fxNumber, SP_FX);

		fx->Active = false;

		ActiveEffects.erase(
			std::remove(
				ActiveEffects.begin(),
				ActiveEffects.end(),
				fxNumber),
			ActiveEffects.end());

		if (fx->RoomNumber != NO_ROOM)
		{
			g_Level.Rooms[fx->RoomNumber].Items.erase(
				std::remove(
					g_Level.Rooms[fx->RoomNumber].Effects.begin(),
					g_Level.Rooms[fx->RoomNumber].Effects.end(),
					fxNumber),
				g_Level.Rooms[fx->RoomNumber].Effects.end());
		}

		NextFxFree = fxNumber;
	}
}

short CreateNewEffect(short roomNum) 
{
	if (NextFxFree != NO_ITEM)
	{
		short fxNumber = NextFxFree;
		auto* fx = &EffectList[fxNumber];

		fx->Active = true;
		fx->RoomNumber = roomNum;
		fx->shade = GRAY555;
		
		auto* room = &g_Level.Rooms[roomNum];
		room->Effects.push_back(fxNumber);

		NextFxFree = NO_ITEM;
		for (short currentFxNumber = 0; currentFxNumber < NUM_EFFECTS; currentFxNumber++)
		{
			auto* currentFx = &EffectList[currentFxNumber];
			if (!currentFx->Active)
			{
				NextFxFree = currentFxNumber;
				break;
			}
		}

		return fxNumber;
	}
	else
	{
		return NO_ITEM;
	}
}

void InitialiseFXArray(int allocateMemory)
{
	NextFxFree = 0;
	ActiveEffects.clear();

	for (short currentFxNumber = 0; currentFxNumber < NUM_EFFECTS; currentFxNumber++)
	{
		EffectList[currentFxNumber].Active = false;
	}
}

void RemoveDrawnItem(short itemNumber) 
{
	auto* item = &g_Level.Items[itemNumber];
	auto* oldRoom = &g_Level.Rooms[item->RoomNumber];
	oldRoom->Items.erase(
		std::remove(
			oldRoom->Items.begin(),
			oldRoom->Items.end(),
			itemNumber),
		oldRoom->Items.end());
}

void RemoveActiveItem(short itemNumber) 
{
	auto* item = &g_Level.Items[itemNumber];

	if (item->Active)
	{
		item->Active = false;

		ActiveItems.erase(
			std::remove(
				ActiveItems.begin(),
				ActiveItems.end(),
				itemNumber),
			ActiveItems.end());

		g_GameScriptEntities->NotifyKilled(item);
		if (!item->luaCallbackOnKilledName.empty())
		{
			g_GameScript->ExecuteFunction(item->luaCallbackOnKilledName, itemNumber);
		}
	}
}

void InitialiseItem(short itemNumber) 
{
	auto* item = &g_Level.Items[itemNumber];

	item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex;
	item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;

	item->Animation.RequiredState = 0;
	item->Animation.TargetState = g_Level.Anims[item->Animation.AnimNumber].ActiveState;
	item->Animation.ActiveState = g_Level.Anims[item->Animation.AnimNumber].ActiveState;

	item->Pose.Orientation.z = 0;
	item->Pose.Orientation.x = 0;

	item->Animation.VerticalVelocity = 0;
	item->Animation.Velocity = 0;

	item->ItemFlags[3] = 0;
	item->ItemFlags[2] = 0;
	item->ItemFlags[1] = 0;
	item->ItemFlags[0] = 0;

	item->Active = false;
	item->Status = ITEM_NOT_ACTIVE;
	item->Animation.Airborne = false;
	item->HitStatus = false;
	item->Collidable = true;
	item->LookedAt = false;

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
		item->MeshBits = -1;

	item->TouchBits = 0;
	item->AlphaOverride = 255;
	item->SwapMeshFlags = 0;

	if (item->Flags & IFLAG_INVISIBLE)
	{
		item->Flags &= ~IFLAG_INVISIBLE;
		item->Status = ITEM_INVISIBLE;
	}
	else if (Objects[item->ObjectNumber].intelligent)
		item->Status = ITEM_INVISIBLE;

	if ((item->Flags & IFLAG_ACTIVATION_MASK) == IFLAG_ACTIVATION_MASK)
	{
		item->Flags &= ~IFLAG_ACTIVATION_MASK;
		item->Flags |= IFLAG_REVERSE;
		AddActiveItem(itemNumber);
		item->Status = ITEM_ACTIVE;
	}

	auto* room = &g_Level.Rooms[item->RoomNumber];
	room->Items.push_back(itemNumber);

	FloorInfo* floor = GetSector(room, item->Pose.Position.x - room->x, item->Pose.Position.z - room->z);
	item->Floor = floor->FloorHeight(item->Pose.Position.x, item->Pose.Position.z);
	item->BoxNumber = floor->Box;

	if (Objects[item->ObjectNumber].nmeshes > 0)
	{
		item->Animation.Mutator.resize(Objects[item->ObjectNumber].nmeshes);
		for (int i = 0; i < item->Animation.Mutator.size(); i++)
			item->Animation.Mutator[i] = {};
	}
	else
		item->Animation.Mutator.clear();

	if (Objects[item->ObjectNumber].initialise != NULL)
		Objects[item->ObjectNumber].initialise(itemNumber);
}

short CreateItem()
{
	short itemNumber = 0;

	if (NextItemFree == NO_ITEM) return NO_ITEM;

	itemNumber = NextItemFree;
	g_Level.Items[NextItemFree].Flags = 0;
	g_Level.Items[NextItemFree].LuaName = "";

	for (short currentItemNumber = g_Level.NumItems; currentItemNumber < g_Level.Items.size(); currentItemNumber++)
	{
		auto* currentItem = &g_Level.Items[currentItemNumber];
		if (!currentItem->Active)
		{
			NextItemFree = currentItemNumber;
			break;
		}
	}

	return itemNumber;
}

void InitialiseItemArray(int totalItem)
{
	auto* item = &g_Level.Items[g_Level.NumItems];

	ActiveItems.clear();
	NextItemFree = g_Level.NumItems;

	if (g_Level.NumItems + 1 < totalItem)
	{
		for (int i = g_Level.NumItems + 1; i < totalItem; i++, item++)
		{
			item->Active = false;
		}
	}
}

short SpawnItem(ItemInfo* item, GAME_OBJECT_ID objectNumber)
{
	short itemNumber = CreateItem();
	if (itemNumber != NO_ITEM)
	{
		auto* spawn = &g_Level.Items[itemNumber];

		spawn->ObjectNumber = objectNumber;
		spawn->RoomNumber = item->RoomNumber;
		memcpy(&spawn->Pose, &item->Pose, sizeof(PHD_3DPOS));

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
		auto* room = &g_Level.Rooms[i];

		for (short itemNumber : room->Items)
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

// Offset values may be used to account for the quirk of room traversal only being able to occur at portals.
void UpdateItemRoom(ItemInfo* item, int height, int xOffset, int zOffset)
{
	float s = phd_sin(item->Pose.Orientation.y);
	float c = phd_cos(item->Pose.Orientation.y);

	int x = item->Pose.Position.x + roundf(c * xOffset + s * zOffset);
	int y = height + item->Pose.Position.y;
	int z = item->Pose.Position.z + roundf(-s * xOffset + c * zOffset);

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

ItemInfo* FindItem(int objectNumber)
{
	for (int i = 0; i < g_Level.NumItems; i++)
	{
		auto* item = &g_Level.Items[i];

		if (item->ObjectNumber == objectNumber)
			return item;
	}

	return 0;
}

int FindItem(ItemInfo* item)
{
	if (item == LaraItem)
		return Lara.ItemNumber;

	for (int i = 0; i < g_Level.NumItems; i++)
		if (item == &g_Level.Items[i])
			return i;

	return -1;
}
