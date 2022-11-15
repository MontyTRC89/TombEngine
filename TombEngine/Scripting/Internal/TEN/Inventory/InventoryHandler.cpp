#include "framework.h"

#include "InventoryHandler.h"
#include "ReservedScriptNames.h"
#include "Game/pickup/pickup.h"
#include "Game/gui.h"

using namespace TEN::Gui;

/***
Inventory manipulation
@tentable Inventory 
@pragma nostrip
*/

namespace InventoryHandler
{
	///Add x of an item to the inventory.
	//Omitting the second argument will give the "default" amount of the item
	//(i.e. the amount the player would get from a pickup of that type).
	//For example, giving crossbow ammo without specifying the number would give the player
	//10 instead.
	//Has no effect if the player has an infinite number of that item.
	//@function GiveItem
	//@tparam ObjID item the item to be added
	//@int[opt] count the number of items to add (default: the amount you would get from a pickup)
	static void InventoryAdd(GAME_OBJECT_ID slot, sol::optional<int> count)
	{
		// If nil is passed in, then the amount added will be the default amount
		// for that pickup - i.e. the amount you would get from picking up the
		// item in-game (e.g. 1 for medipacks, 12 for flares).

		// can't use value_or(std::nullopt) here because nullopt isn't an int
		if (count.has_value())
			PickedUpObject(slot, count.value());
		else
			PickedUpObject(slot, std::nullopt);
	}

	///Remove x of a certain item from the inventory.
	//As in @{GiveItem}, omitting the count will remove the "default" amount of that item.
	//Has no effect if the player has an infinite number of the item.
	//@function TakeItem
	//@tparam ObjID item the item to be removed
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
	//@tparam InvID item the ID item to check
	//@treturn int the amount of the item the player has in the inventory. -1 indicates an infinite amount of that item.
	static int InventoryGetCount(GAME_OBJECT_ID slot)
	{
		return GetInventoryCount(slot);
	}

	///Set the amount of a certain item the player has in the inventory.
	//Similar to @{GiveItem} but replaces with the new amount instead of adding it.
	//@function SetItemCount
	//@tparam InvID item the ID of the item to be set.
	//@tparam int count the number of items the player will have. A value of -1 will give an infinite amount of that item.
	static void InventorySetCount(GAME_OBJECT_ID slot, int count)
	{
		SetInventoryCount(slot, count);
	}

	static void InventoryCombine(int slot1, int slot2)
	{

	}

	static void InventorySeparate(int slot)
	{

	}

	///Enters inventory screen.
	//@function EnterInventoryScreen
	static void EnterInventoryScreen()
	{
		g_Gui.ProcessInventory(InventoryMode::InGame);
	}

	///Enters statistics screen.
	//@function EnterStatisticsScreen
	static void EnterStatisticsScreen()
	{
		g_Gui.ProcessInventory(InventoryMode::Statistics);
	}

	///Enters pause screen.
	//@function EnterPauseScreen
	static void EnterPauseScreen()
	{
		g_Gui.ProcessInventory(InventoryMode::Pause);
	}

	///Enters load screen.
	//@function EnterLoadScreen
	static void EnterLoadScreen()
	{
		g_Gui.ProcessInventory(InventoryMode::Load);
	}

	///Enters save screen.
	//@function EnterSaveScreen
	static void EnterSaveScreen()
	{
		g_Gui.ProcessInventory(InventoryMode::Save);
	}

	void Register(sol::state* state, sol::table& parent)
	{
		sol::table table_inventory{ state->lua_state(), sol::create };
		parent.set(ScriptReserved_Inventory, table_inventory);

		table_inventory.set_function(ScriptReserved_GiveInvItem, &InventoryAdd);
		table_inventory.set_function(ScriptReserved_TakeInvItem, &InventoryRemove);
		table_inventory.set_function(ScriptReserved_GetInvItemCount, &InventoryGetCount);
		table_inventory.set_function(ScriptReserved_SetInvItemCount, &InventorySetCount);

		table_inventory.set_function(ScriptReserved_EnterInventoryScreen, &EnterInventoryScreen);
		table_inventory.set_function(ScriptReserved_EnterStatisticsScreen, &EnterStatisticsScreen);
		table_inventory.set_function(ScriptReserved_EnterLoadScreen, &EnterLoadScreen);
		table_inventory.set_function(ScriptReserved_EnterSaveScreen, &EnterSaveScreen);
		table_inventory.set_function(ScriptReserved_EnterPauseScreen, &EnterPauseScreen);
	}
}
