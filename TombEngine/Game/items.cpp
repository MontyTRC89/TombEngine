#include "framework.h"
#include "Game/items.h"

#include "Game/collision/floordata.h"
#include "Game/collision/collide_room.h"
#include "Game/control/control.h"
#include "Game/control/volume.h"
#include "Game/effects/effects.h"
#include "Game/Lara/lara.h"
#include "Math/Math.h"
#include "Objects/ScriptInterfaceObjectsHandler.h"
#include "Scripting/Include/ScriptInterfaceGame.h"
#include "Sound/sound.h"
#include "Specific/Input/Input.h"
#include "Specific/level.h"
#include "Specific/setup.h"

using namespace TEN::Floordata;
using namespace TEN::Input;
using namespace TEN::Math::Random;
using namespace TEN::Control::Volumes;

bool ItemInfo::TestOcb(short ocbFlags)
{
	return ((TriggerFlags & ocbFlags) == ocbFlags);
}

void ItemInfo::RemoveOcb(short ocbFlags)
{
	TriggerFlags &= ~ocbFlags;
}

void ItemInfo::ClearAllOcb()
{
	TriggerFlags = 0;
}

bool ItemInfo::TestFlags(short id, short value)
{
	if (id < 0 || id > 7)
		return false;

	return (ItemFlags[id] == value);
}

void ItemInfo::SetFlags(short id, short value)
{
	if (id < 0 || id > 7)
		return;

	ItemFlags[id] = value;
}

bool ItemInfo::TestMeshSwapFlags(unsigned int flags)
{
	for (size_t i = 0; i < Model.MeshIndex.size(); i++)
	{
		if (flags & (1 << i))
		{
			if (Model.MeshIndex[i] == Model.BaseMesh + i)
				return false;
		}
	}

	return true;
}

bool ItemInfo::TestMeshSwapFlags(const std::vector<unsigned int> flags)
{
	BitField bits = {};
	bits.Set(flags);
	return TestMeshSwapFlags(bits.ToPackedBits());
}

void ItemInfo::SetMeshSwapFlags(unsigned int flags, bool clear)
{
	bool meshSwapPresent = Objects[ObjectNumber].meshSwapSlot != -1 && 
						   Objects[Objects[ObjectNumber].meshSwapSlot].loaded;


	for (size_t i = 0; i < Model.MeshIndex.size(); i++)
	{
		if (meshSwapPresent && (flags & (1 << i)))
		{
			if (clear)
				Model.MeshIndex[i] = Model.BaseMesh + i;
			else
				Model.MeshIndex[i] = Objects[Objects[ObjectNumber].meshSwapSlot].meshIndex + i;
		}
		else
			Model.MeshIndex[i] = Model.BaseMesh + i;
	}
}

void ItemInfo::SetMeshSwapFlags(const std::vector<unsigned int> flags, bool clear)
{
	BitField bits = {};
	bits.Set(flags);
	SetMeshSwapFlags(bits.ToPackedBits(), clear);
}

bool ItemInfo::IsLara()
{
	return this->Data.is<LaraInfo*>();
}

bool ItemInfo::IsCreature()
{
	return this->Data.is<CreatureInfo>();
}

bool TestState(int refState, const vector<int>& stateList)
{
	for (const auto& state : stateList)
	{
		if (state == refState)
			return true;
	}

	return false;
}

void ClearItem(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];
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
	else// if (NextItemActive != NO_ITEM)
	{
		auto* item = &g_Level.Items[itemNumber];

		DetatchSpark(itemNumber, SP_ITEM);

		item->Active = false;

		if (NextItemActive == itemNumber)
			NextItemActive = item->NextActive;
		else
		{
			short linkNumber;
			for (linkNumber = NextItemActive; linkNumber != NO_ITEM; linkNumber = g_Level.Items[linkNumber].NextActive)
			{
				if (g_Level.Items[linkNumber].NextActive == itemNumber)
				{
					g_Level.Items[linkNumber].NextActive = item->NextActive;
					break;
				}
			}
		}

		if (item->RoomNumber != NO_ROOM)
		{
			if (g_Level.Rooms[item->RoomNumber].itemNumber == itemNumber)
				g_Level.Rooms[item->RoomNumber].itemNumber = item->NextItem;
			else
			{
				short linkNumber;
				for (linkNumber = g_Level.Rooms[item->RoomNumber].itemNumber; linkNumber != NO_ITEM; linkNumber = g_Level.Items[linkNumber].NextItem)
				{
					if (g_Level.Items[linkNumber].NextItem == itemNumber)
					{
						g_Level.Items[linkNumber].NextItem = item->NextItem;
						break;
					}
				}
			}
		}

		if (item == Lara.TargetEntity)
			Lara.TargetEntity = NULL;

		if (Objects[item->ObjectNumber].floor != nullptr)
			UpdateBridgeItem(itemNumber, true);

		g_GameScriptEntities->NotifyKilled(item);
		g_GameScriptEntities->TryRemoveColliding(itemNumber, true);
		if (!item->Callbacks.OnKilled.empty())
			g_GameScript->ExecuteFunction(item->Callbacks.OnKilled, itemNumber);

		item->Name.clear();
		item->Callbacks.OnKilled.clear();
		item->Callbacks.OnHit.clear();
		item->Callbacks.OnObjectCollided.clear();
		item->Callbacks.OnRoomCollided.clear();

		if (itemNumber >= g_Level.NumItems)
		{
			item->NextItem = NextItemFree;
			NextItemFree = itemNumber;
		}
		else
			item->Flags |= IFLAG_KILLED;
	}
}

void RemoveAllItemsInRoom(short roomNumber, short objectNumber)
{
	auto* room = &g_Level.Rooms[roomNumber];

	short currentItemNumber = room->itemNumber;
	while (currentItemNumber != NO_ITEM)
	{
		auto* item = &g_Level.Items[currentItemNumber];

		if (item->ObjectNumber == objectNumber)
		{
			RemoveActiveItem(currentItemNumber);
			item->Status = ITEM_NOT_ACTIVE;
			item->Flags &= 0xC1;
		}

		currentItemNumber = item->NextItem;
	}
}

void AddActiveItem(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

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
		auto* item = &g_Level.Items[itemNumber];

		if (item->RoomNumber != NO_ROOM)
		{
			auto* room = &g_Level.Rooms[item->RoomNumber];

			if (room->itemNumber == itemNumber)
				room->itemNumber = item->NextItem;
			else
			{
				for (short linkNumber = room->itemNumber; linkNumber != -1; linkNumber = g_Level.Items[linkNumber].NextItem)
				{
					if (g_Level.Items[linkNumber].NextItem == itemNumber)
					{
						g_Level.Items[linkNumber].NextItem = item->NextItem;
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
		auto* fx = &EffectList[fxNumber];
		auto* room = &g_Level.Rooms[fx->roomNumber];

		if (room->fxNumber == fxNumber)
			room->fxNumber = fx->nextFx;
		else
		{
			for (short linkNumber = room->fxNumber; linkNumber != -1; linkNumber = EffectList[linkNumber].nextFx)
			{
				if (EffectList[linkNumber].nextFx == fxNumber)
				{
					EffectList[linkNumber].nextFx = fx->nextFx;
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
		auto* fx = &EffectList[fxNumber];
		DetatchSpark(fxNumber, SP_FX);

		if (NextFxActive == fxNumber)
			NextFxActive = fx->nextActive;
		else
		{
			for (short linkNumber = NextFxActive; linkNumber != NO_ITEM; linkNumber = EffectList[linkNumber].nextActive)
			{
				if (EffectList[linkNumber].nextActive == fxNumber)
				{
					EffectList[linkNumber].nextActive = fx->nextActive;
					break;
				}
			}
		}

		if (g_Level.Rooms[fx->roomNumber].fxNumber == fxNumber)
			g_Level.Rooms[fx->roomNumber].fxNumber = fx->nextFx;
		else
		{
			for (short linkNumber = g_Level.Rooms[fx->roomNumber].fxNumber; linkNumber != NO_ITEM; linkNumber = EffectList[linkNumber].nextFx)
			{
				if (EffectList[linkNumber].nextFx == fxNumber)
				{
					EffectList[linkNumber].nextFx = fx->nextFx;
					break;
				}
			}
		}

		fx->nextFx = NextFxFree;
		NextFxFree = fxNumber;
	}
}

short CreateNewEffect(short roomNumber) 
{
	short fxNumber = NextFxFree;

	if (NextFxFree != NO_ITEM)
	{
		auto* fx = &EffectList[NextFxFree];
		NextFxFree = fx->nextFx;

		auto* room = &g_Level.Rooms[roomNumber];

		fx->roomNumber = roomNumber;
		fx->nextFx = room->fxNumber;
		room->fxNumber = fxNumber;
		fx->nextActive = NextFxActive;
		NextFxActive = fxNumber;
		fx->color = Vector4::One;
	}

	return fxNumber;
}

void InitialiseFXArray(int allocateMemory)
{
	NextFxActive = NO_ITEM;
	NextFxFree = 0;

	for (int i = 0; i < NUM_EFFECTS; i++)
	{
		auto* fx = &EffectList[i];
		fx->nextFx = i + 1;
	}

	EffectList[NUM_EFFECTS - 1].nextFx = NO_ITEM;
}

void RemoveDrawnItem(short itemNumber) 
{
	auto* item = &g_Level.Items[itemNumber];

	if (g_Level.Rooms[item->RoomNumber].itemNumber == itemNumber)
		g_Level.Rooms[item->RoomNumber].itemNumber = item->NextItem;
	else
	{
		for (short linkNumber = g_Level.Rooms[item->RoomNumber].itemNumber; linkNumber != NO_ITEM; linkNumber = g_Level.Items[linkNumber].NextItem)
		{
			if (g_Level.Items[linkNumber].NextItem == itemNumber)
			{
				g_Level.Items[linkNumber].NextItem = item->NextItem;
				break;
			}
		}
	}
}

void RemoveActiveItem(short itemNumber) 
{
	auto& item = g_Level.Items[itemNumber];

	if (g_Level.Items[itemNumber].Active)
	{
		g_Level.Items[itemNumber].Active = false;

		if (NextItemActive == itemNumber)
		{
			NextItemActive = g_Level.Items[itemNumber].NextActive;
		}
		else
		{
			for (short linkNumber = NextItemActive; linkNumber != NO_ITEM; linkNumber = g_Level.Items[linkNumber].NextActive)
			{
				if (g_Level.Items[linkNumber].NextActive == itemNumber)
				{
					g_Level.Items[linkNumber].NextActive = g_Level.Items[itemNumber].NextActive;
					break;
				}
			}
		}

		g_GameScriptEntities->NotifyKilled(&item);
		if (!item.Callbacks.OnKilled.empty())
			g_GameScript->ExecuteFunction(item.Callbacks.OnKilled, itemNumber);
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

	item->Animation.Velocity.y = 0;
	item->Animation.Velocity.z = 0;

	item->ItemFlags[3] = 0;
	item->ItemFlags[2] = 0;
	item->ItemFlags[1] = 0;
	item->ItemFlags[0] = 0;

	item->Active = false;
	item->Status = ITEM_NOT_ACTIVE;
	item->Animation.IsAirborne = false;
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
		item->MeshBits = ALL_JOINT_BITS;

	item->TouchBits = NO_JOINT_BITS;
	item->AfterDeath = 0;

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

	item->NextItem = room->itemNumber;
	room->itemNumber = itemNumber;

	FloorInfo* floor = GetSector(room, item->Pose.Position.x - room->x, item->Pose.Position.z - room->z);
	item->Floor = floor->FloorHeight(item->Pose.Position.x, item->Pose.Position.z);
	item->BoxNumber = floor->Box;

	if (Objects[item->ObjectNumber].nmeshes > 0)
	{
		item->Model.BaseMesh = Objects[item->ObjectNumber].meshIndex;

		item->Model.MeshIndex.resize(Objects[item->ObjectNumber].nmeshes);
		for (int i = 0; i < item->Model.MeshIndex.size(); i++)
			item->Model.MeshIndex[i] = item->Model.BaseMesh + i;

		item->Model.Mutator.resize(Objects[item->ObjectNumber].nmeshes);
		for (int i = 0; i < item->Model.Mutator.size(); i++)
			item->Model.Mutator[i] = {};
	}
	else
	{
		item->Model.Mutator.clear();
		item->Model.MeshIndex.clear();
	}

	if (Objects[item->ObjectNumber].initialise != nullptr)
		Objects[item->ObjectNumber].initialise(itemNumber);
}

short CreateItem()
{
	short itemNumber = 0;

	if (NextItemFree == -1) return NO_ITEM;

	itemNumber = NextItemFree;
	g_Level.Items[NextItemFree].Flags = 0;
	NextItemFree = g_Level.Items[NextItemFree].NextItem;

	return itemNumber;
}

void InitialiseItemArray(int totalItem)
{
	g_Level.Items.clear();
	g_Level.Items.resize(totalItem);

	for (int i = 0; i < totalItem; i++)
		g_Level.Items[i].Index = i;

	auto* item = &g_Level.Items[g_Level.NumItems];

	if (g_Level.NumItems + 1 < totalItem)
	{
		for (int i = g_Level.NumItems + 1; i < totalItem; i++, item++)
		{
			item->NextItem = i;
			item->Active = false;
			item->Data = nullptr;
		}
	}

	item->NextItem = NO_ITEM;
	NextItemActive = NO_ITEM;
	NextItemFree = g_Level.NumItems;
}

short SpawnItem(ItemInfo* item, GAME_OBJECT_ID objectNumber)
{
	short itemNumber = CreateItem();
	if (itemNumber != NO_ITEM)
	{
		auto* spawn = &g_Level.Items[itemNumber];

		spawn->ObjectNumber = objectNumber;
		spawn->RoomNumber = item->RoomNumber;
		memcpy(&spawn->Pose, &item->Pose, sizeof(Pose));

		InitialiseItem(itemNumber);

		spawn->Status = ITEM_NOT_ACTIVE;
		spawn->Color = Vector4(0.5f, 0.5f, 0.5f, 1.0f);
	}

	return itemNumber;
}

int GlobalItemReplace(short search, GAME_OBJECT_ID replace)
{
	int changed = 0;
	for (int i = 0; i < g_Level.Rooms.size(); i++)
	{
		auto* room = &g_Level.Rooms[i];

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

	return nullptr;
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

void UpdateAllItems()
{
	InItemControlLoop = true;

	short itemNumber = NextItemActive;
	while (itemNumber != NO_ITEM)
	{
		auto* item = &g_Level.Items[itemNumber];
		short nextItem = item->NextActive;

		if (item->AfterDeath <= 128)
		{
			if (Objects[item->ObjectNumber].control)
				Objects[item->ObjectNumber].control(itemNumber);

			TestVolumes(itemNumber);

			if (item->AfterDeath > 0 && item->AfterDeath < 128 && !(Wibble & 3))
				item->AfterDeath++;
			if (item->AfterDeath == 128)
				KillItem(itemNumber);
		}
		else
			KillItem(itemNumber);

		itemNumber = nextItem;
	}

	InItemControlLoop = false;
	KillMoveItems();
}

void UpdateAllEffects()
{
	InItemControlLoop = true;

	short fxNumber = NextFxActive;
	while (fxNumber != NO_ITEM)
	{
		short nextFx = EffectList[fxNumber].nextActive;
		auto* fx = &EffectList[fxNumber];
		if (Objects[fx->objectNumber].control)
			Objects[fx->objectNumber].control(fxNumber);

		fxNumber = nextFx;
	}

	InItemControlLoop = false;
	KillMoveEffects();
}

void UpdateItemRoom(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	auto roomNumber = GetCollision(item->Pose.Position.x,
		item->Pose.Position.y - CLICK(2),
		item->Pose.Position.z,
		item->RoomNumber).RoomNumber;

	if (roomNumber != item->RoomNumber)
		ItemNewRoom(itemNumber, roomNumber);

	if (item->IsCreature() &&
		!Objects[item->ObjectNumber].waterCreature &&
		TestEnvironment(RoomEnvFlags::ENV_FLAG_WATER, &g_Level.Rooms[roomNumber]))
	{
		auto bounds = GameBoundingBox(item);
		auto height = item->Pose.Position.y - GetWaterHeight(item);

		if (abs(bounds.Y1 + bounds.Y2) < height)
			DoDamage(item, INT_MAX);
	}
}

void DoDamage(ItemInfo* item, int damage)
{
	static int lastHurtTime = 0;

	if (item->HitPoints <= 0)
		return;

	item->HitStatus = true;

	item->HitPoints -= damage;
	if (item->HitPoints < 0)
		item->HitPoints = 0;

	if (item->IsLara())
	{
		if (damage > 0)
		{
			float power = item->HitPoints ? GenerateFloat(0.1f, 0.4f) : 0.5f;
			Rumble(power, 0.15f);
		}

		if ((GlobalCounter - lastHurtTime) > (FPS * 2 + GenerateInt(0, FPS)))
		{
			SoundEffect(SFX_TR4_LARA_INJURY, &LaraItem->Pose);
			lastHurtTime = GlobalCounter;
		}
	}
}
