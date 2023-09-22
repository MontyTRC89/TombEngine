#include "framework.h"

#include "InventoryHandler.h"
#include "Scripting/Internal/ReservedScriptNames.h"
#include "Game/pickup/pickup.h"
#include "Game/Lara/lara.h"
#include "Game/Hud/Hud.h"

/***
Inventory manipulation
@tentable Inventory 
@pragma nostrip
*/

using namespace TEN::Hud;

namespace InventoryHandler
{
	///Add x of an item to the inventory.
	//Omitting the second argument will give the "default" amount of the item
	//(i.e. the amount the player would get from a pickup of that type).
	//For example, giving crossbow ammo without specifying the number would give the player
	//10 instead.
	//Has no effect if the player has an infinite number of that item.
	//@function GiveItem
	//@tparam Objects.ObjID item the item to be added
	//@int[opt] count the number of items to add (default: the amount you would get from a pickup)
	//@bool[opt] display true if the item is to be displayed
	static void InventoryAdd(GAME_OBJECT_ID slot, sol::optional<int> count, sol::optional<bool> display)
	{
		// If nil is passed in, then the amount added will be the default amount
		// for that pickup - i.e. the amount you would get from picking up the
		// item in-game (e.g. 1 for medipacks, 12 for flares).

		// can't use value_or(std::nullopt) here because nullopt isn't an int
		if (count.has_value())
			PickedUpObject(slot, count.value());
		else
			PickedUpObject(slot, std::nullopt);

		if (display.has_value() && display.value())
		{
			auto pos = GetJointPosition(LaraItem, LM_HIPS);
			g_Hud.PickupSummary.AddDisplayPickup(slot, pos.ToVector3());
		}
	}

	///Remove x of a certain item from the inventory.
	//As in @{GiveItem}, omitting the count will remove the "default" amount of that item.
	//Has no effect if the player has an infinite number of the item.
	//@function TakeItem
	//@tparam Objects.ObjID item the item to be removed
	//@int[opt] count the number of items to remove (default: the amount you would get from a pickup)
	static void InventoryRemove(GAME_OBJECT_ID slot, sol::optional<int> count)
	{
		//can't use value_or(std::nullopt) here because nullopt isn't an int
		if (count.has_value())
			RemoveObjectFromInventory(slot, count.value());
		else
			RemoveObjectFromInventory(slot, std::nullopt);
	}

	///Get the amount the player holds of an item.
	//@function GetItemCount
	//@tparam Objects.ObjID item the ID item to check
	//@treturn int the amount of the item the player has in the inventory. -1 indicates an infinite amount of that item.
	static int InventoryGetCount(GAME_OBJECT_ID slot)
	{
		return GetInventoryCount(slot);
	}

	///Set the amount of a certain item the player has in the inventory.
	//Similar to @{GiveItem} but replaces with the new amount instead of adding it.
	//@function SetItemCount
	//@tparam Objects.ObjID item the ID of the item to be set.
	//@tparam int count the number of items the player will have. A value of -1 will give an infinite amount of that item.
	static void InventorySetCount(GAME_OBJECT_ID slot, int count)
	{
		SetInventoryCount(slot, count);
	}

	void Register(sol::state* state, sol::table& parent)
	{
		sol::table table_inventory{ state->lua_state(), sol::create };
		parent.set(ScriptReserved_Inventory, table_inventory);

		table_inventory.set_function(ScriptReserved_GiveInvItem, &InventoryAdd);
		table_inventory.set_function(ScriptReserved_TakeInvItem, &InventoryRemove);
		table_inventory.set_function(ScriptReserved_GetInvItemCount, &InventoryGetCount);
		table_inventory.set_function(ScriptReserved_SetInvItemCount, &InventorySetCount);
	}
}
