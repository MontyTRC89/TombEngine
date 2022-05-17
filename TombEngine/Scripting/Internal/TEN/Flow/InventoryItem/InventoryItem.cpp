#include "framework.h"
#include "InventoryItem.h"
#include "ScriptAssert.h"
#include "ReservedScriptNames.h"
#include <string>

/***
Represents the properties of an object as it appears in the inventory.

@tenclass Flow.InventoryItem
@pragma nostrip
*/

/*** Create an InventoryItem.
	@function InventoryItem
	@tparam string nameKey key for the item's (localised) name.<br />
Corresponds to an entry in strings.lua.
	@tparam Flow.InvItem slot slot of inventory object to change
	@tparam int yOffset y-axis offset (positive values move the item down).<br />
A value of about 100 will cause the item to display directly below its usual position.
	@tparam float scale item size (1 being standard size).<br />
A value of 0.5 will cause the item to render at half the size,
and a value of 2 will cause the item to render at twice the size.
	@tparam Rotation rot rotation about x, y, and z axes
	@tparam RotationAxis axis Axis to rotate about when the item is being looked at in the inventory.<br />
Note that this is entirely separate from the `rot` field described above.
Must be RotationAxis.X, RotationAxis.Y or RotationAxis.Z.
e.g. `myItem.rotAxisWhenCurrent = RotationAxis.X`
	@tparam int meshBits __Not currently implemented__ (will have no effect regardless of what you set it to)
	@tparam ItemAction action is this usable, equippable, or examinable?<br/>
Must be one of:
	EQUIP
	USE
	EXAMINE
e.g. `myItem.action = ItemAction.EXAMINE`
	@return an InventoryItem
*/
InventoryItem::InventoryItem(std::string const& a_name, ItemEnumPair a_slot, short a_yOffset, float a_scale, Rotation const & a_rot, RotationFlags a_rotationFlags, int a_meshBits, ItemOptions a_action) :
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

void InventoryItem::Register(sol::table & parent)
{
	using ctors = sol::constructors<InventoryItem(std::string const&, ItemEnumPair, short, float, Rotation const&, RotationFlags, int, ItemOptions)>;
	parent.new_usertype<InventoryItem>(ScriptReserved_InventoryItem,
	ctors(),
		sol::call_constructor, ctors()
	);
}

// Add validation so the user can't choose something unimplemented
void InventoryItem::SetAction(ItemOptions a_action)
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

void InventoryItem::SetSlot(ItemEnumPair a_slot)
{
	slot = a_slot.m_pair.second;
}
