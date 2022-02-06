#include "frameworkandsol.h"
#include "GameScriptInventoryObject.h"
#include "ScriptAssert.h"
#include <string>

/***
Represents the properties of an object as it appears in the inventory.

@pregameclass InventoryObject
@pragma nostrip
*/

/*** Create an inventoryObject item. Use this if you want to specify property values later later.
The default property values are not disclosed here, since at the time of writing, they are subject to change.
	@function InventoryObject.new
	@return an InventoryObject
*/

/*** For more information on each parameter, see the
associated getters and setters.
	@function InventoryObject.new
	@tparam string nameKey name key
	@tparam InvItem slot slot of inventory object to change
	@tparam int yOffset y-axis offset (positive values move the item down)
	@tparam float scale item size (1 being standard size) 
	@tparam Rotation rot rotation about x, y, and z axes
	@tparam RotationAxis rotAxisWhenCurrent axis to rotate around in inventory
	@tparam int meshBits not currently implemented
	@tparam ItemAction action is this usable, equippable, or examinable?
	@return an InventoryObject
*/
GameScriptInventoryObject::GameScriptInventoryObject(std::string const& a_name, ItemEnumPair a_slot, short a_yOffset, float a_scale, GameScriptRotation const & a_rot, RotationFlags a_rotationFlags, int a_meshBits, ItemOptions a_action) :
	name{ a_name },
	slot{ a_slot.m_pair.second },
	yOffset{ a_yOffset },
	scale{ a_scale },
	rot{ a_rot },
	rotationFlags{ a_rotationFlags },
	meshBits{ a_meshBits }
{
	SetAction(a_action);
}

void GameScriptInventoryObject::Register(sol::state * lua)
{
	lua->new_usertype<GameScriptInventoryObject>("InventoryObject",
		sol::constructors<GameScriptInventoryObject(std::string const &, ItemEnumPair, short, float, GameScriptRotation const &, RotationFlags, int, ItemOptions), GameScriptInventoryObject()>(),
/*** (string) string key for the item's (localised) name. Corresponds to an entry in strings.lua.
@mem nameKey
*/
		"nameKey", &GameScriptInventoryObject::name,

/*** (@{InvItem}) slot of item whose inventory display properties you wish to change
@mem slot
*/
		"slot", sol::property(&GameScriptInventoryObject::SetSlot),

/*** (float) y-axis offset (positive values will move the item lower).
A value of about 100 will cause the item to display directly below its usual position.
@mem yOffset
*/
		"yOffset", &GameScriptInventoryObject::yOffset,

/*** (float) Item's size when displayed in the inventory as a multiple of its "regular" size.
A value of 0.5 will cause the item to render at half the size,
and a value of 2 will cause the item to render at twice the size.
@mem scale
*/
		"scale", &GameScriptInventoryObject::scale,

/*** (@{Rotation}) Item's rotation about its origin when displayed in the inventory.
@mem rot
*/
		"rot", &GameScriptInventoryObject::rot,

/*** (RotationAxis) Axis to rotate about when the item is being looked at in the inventory.
Note that this is entirely separate from the `rot` field described above.
Must be RotationAxis.X, RotationAxis.Y or RotationAxis.Z.
e.g. `myItem.rotAxisWhenCurrent = RotationAxis.X`
@mem rotAxisWhenCurrent
*/
		"rotAxisWhenCurrent", &GameScriptInventoryObject::rotationFlags,

/*** (int) __Not currently implemented__ (will have no effect regardless of what you set it to)
@mem meshBits
*/
		"meshBits", &GameScriptInventoryObject::meshBits,

/*** (ItemAction) What can the player do with the item?
Must be one of:
	EQUIP
	USE
	EXAMINE
e.g. `myItem.action = ItemAction.EXAMINE`
@mem action
*/
		"action", sol::property(&GameScriptInventoryObject::SetAction)
		);
}

// Add validation so the user can't choose something unimplemented
void GameScriptInventoryObject::SetAction(ItemOptions a_action)
{
	bool isSupported = (a_action == ItemOptions::OPT_EQUIP) ||
		(a_action == ItemOptions::OPT_USE) ||
		(a_action == ItemOptions::OPT_EXAMINABLE);

	if (!ScriptAssert(isSupported, "Unsupported item action: " + std::to_string(a_action)))
	{
		ItemOptions def = ItemOptions::OPT_USE;
		ScriptWarn("Defaulting to " + std::to_string(def));
		action = def;
	}
	else
	{
		action = a_action;
	}
}

void GameScriptInventoryObject::SetSlot(ItemEnumPair a_slot)
{
	slot = a_slot.m_pair.second;
}
