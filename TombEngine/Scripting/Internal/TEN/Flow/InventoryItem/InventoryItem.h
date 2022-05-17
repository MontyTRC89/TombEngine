#pragma once
#include <string>
#include "ItemEnumPair.h"
#include "Rotation/Rotation.h"
#include "Game/gui.h"

static const std::unordered_map<std::string, RotationFlags> kRotAxes{
	{"X", RotationFlags::INV_ROT_X},
	{"Y", RotationFlags::INV_ROT_Y},
	{"Z", RotationFlags::INV_ROT_Z}
};

static const std::unordered_map<std::string, ItemOptions> kItemActions{
	{"USE", ItemOptions::OPT_USE},
	{"EQUIP", ItemOptions::OPT_EQUIP},
	{"EXAMINE", ItemOptions::OPT_EXAMINABLE}
};

namespace sol {
	class state;
}

struct InventoryItem
{
	InventoryItem(std::string const & a_name, ItemEnumPair a_slot, short a_yOffset, float a_scale, Rotation const & a_rot, RotationFlags a_rotationFlags, int a_meshBits, ItemOptions a_actions);

	static void Register(sol::table& lua);

	std::string name{};
	InventoryObjectTypes slot{ INV_OBJECT_PISTOLS };
	short yOffset{ 0 };
	float scale{ 1.0f };
	Rotation rot{};
	RotationFlags rotationFlags{ RotationFlags::INV_ROT_X };
	int meshBits{ 0 };
	ItemOptions action{ ItemOptions::OPT_USE };

	void SetAction(ItemOptions a_action);
	void SetSlot(ItemEnumPair a_slot);
};
