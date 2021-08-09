#include "framework.h"
#include "GameScriptInventoryObject.h"

/***
Represents the properties of an object as it appears in the inventory.

@classmod InventoryObject
@pragma nostrip
*/

GameScriptInventoryObject::GameScriptInventoryObject(std::string const& a_name, ItemEnumPair a_slot, float a_yOffset, float a_scale, float a_xRot, float a_yRot, float a_zRot, short a_rotationFlags, int a_meshBits, __int64 a_operation) :
	name{ a_name },
	slot{ a_slot.m_pair.second },
	yOffset{ a_yOffset },
	scale{ a_scale },
	xRot{ a_xRot },
	yRot{ a_yRot },
	zRot{ a_zRot },
	rotationFlags{ a_rotationFlags },
	meshBits{ a_meshBits },
	operation{ a_operation }
{}

void GameScriptInventoryObject::Register(sol::state * lua)
{
	lua->new_usertype<GameScriptInventoryObject>("InventoryObject",
		sol::constructors<GameScriptInventoryObject(std::string const &, ItemEnumPair, float, float, float, float, float, short, int, __int64)>(),

		/// (string) string key for the item's (localised) name. Corresponds to an entry in strings.lua.
		//@mem nameKey
		"nameKey", &GameScriptInventoryObject::name,

		/// (float) y-axis offset.
		// A value of about 100 will cause the item to display directly below its usual position.
		//@mem nameKey
		"yOffset", &GameScriptInventoryObject::yOffset,

		/// (float) Item's size when displayed in the inventory as a multiple of its "regular" size.
		// A value of 0.5 will cause the item to render at half the size,
		// and a value of 2 will cause the item to render at twice the size.
		//@mem scale
		"scale", &GameScriptInventoryObject::scale,

		"xRot", &GameScriptInventoryObject::xRot,
		"yRot", &GameScriptInventoryObject::yRot,
		"zRot", &GameScriptInventoryObject::zRot,
		"rotationFlags", &GameScriptInventoryObject::rotationFlags,
		"meshBits", &GameScriptInventoryObject::meshBits,
		"operation", &GameScriptInventoryObject::operation
		);
}
