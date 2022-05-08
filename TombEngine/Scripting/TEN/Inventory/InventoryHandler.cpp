#include "framework.h"

#include "InventoryHandler.h"
#include "ReservedScriptNames.h"
#include "pickup.h"
#include "ItemEnumPair.h"

/***
Inventory manipulation
@tentable Inventory 
@pragma nostrip
*/

namespace InventoryHandler
{
	static void InventoryAdd(ItemEnumPair slot, sol::optional<int> count)
	{
		// If 0 is passed in, then the amount added will be the default amount
		// for that pickup - i.e. the amount you would get from picking up the
		// item in-game (e.g. 1 for medipacks, 12 for flares).
		PickedUpObject(slot.m_pair.first, count.value_or(0));
	}

	static void InventoryRemove(ItemEnumPair slot, sol::optional<int> count)
	{
		// 0 is default for the same reason as in InventoryAdd.
		RemoveObjectFromInventory(slot.m_pair.first, count.value_or(0));
	}

	static int InventoryGetCount(ItemEnumPair slot)
	{
		return GetInventoryCount(slot.m_pair.first);
	}

	static void InventorySetCount(ItemEnumPair slot, int count)
	{
		// add the amount we'd need to add to get to count
		int currAmt = GetInventoryCount(slot.m_pair.first);
		InventoryAdd(slot, count - currAmt);
	}

	static void InventoryCombine(int slot1, int slot2)
	{

	}

	static void InventorySeparate(int slot)
	{

	}

	void Register(sol::state* state, sol::table& parent)
	{
		sol::table table_inventory{ state->lua_state(), sol::create };
		parent.set(ScriptReserved_Inventory, table_inventory);

		///Add x of an item to the inventory.
		//A count of 0 will add the "default" amount of that item
		//(i.e. the amount the player would get from a pickup of that type).
		//For example, giving "zero" crossbow ammo would give the player
		//10 instead, whereas giving "zero" medkits would give the player 1 medkit.
		//@function GiveItem
		//@tparam InvID item the item to be added
		//@tparam int count the number of items to add (default: 0)
		table_inventory.set_function(ScriptReserved_GiveInvItem, &InventoryAdd);


		//Remove x of a certain item from the inventory.
		//As in @{GiveItem}, a count of 0 will remove the "default" amount of that item.
		//@function TakeItem
		//@tparam InvID item the item to be removed
		//@tparam int count the number of items to remove (default: 0)
		table_inventory.set_function(ScriptReserved_TakeInvItem, &InventoryRemove);


		///Get the amount the player holds of an item.
		//@function GetItemCount
		//@tparam InvID item the ID item to check
		//@treturn int the amount of the item the player has in the inventory
		table_inventory.set_function(ScriptReserved_GetInvItemCount, &InventoryGetCount);


		///Set the amount of a certain item the player has in the inventory.
		//Similar to @{GiveItem} but replaces with the new amount instead of adding it.
		//@function SetItemCount
		//@tparam @{InvID} item the ID of the item to be set
		//@tparam int count the number of items the player will have
		table_inventory.set_function(ScriptReserved_SetInvItemCount, &InventorySetCount);
	}
}
