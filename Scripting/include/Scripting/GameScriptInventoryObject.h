#pragma once
#include <string>
#include "ItemEnumPair.h"
#include "GameScriptRotation.h"
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

struct GameScriptInventoryObject
{
	std::string name{};
	InventoryObjectTypes slot{ INV_OBJECT_PISTOLS };
	short yOffset{ 0 };
	float scale{ 1.0f };
	GameScriptRotation rot{};
	RotationFlags rotationFlags{ RotationFlags::INV_ROT_X };
	int meshBits{ 0 };
	ItemOptions action{ ItemOptions::OPT_USE };

	GameScriptInventoryObject() = default;
	GameScriptInventoryObject(std::string const & a_name, ItemEnumPair a_slot, short a_yOffset, float a_scale, GameScriptRotation const & a_rot, RotationFlags a_rotationFlags, int a_meshBits, ItemOptions a_actions);

	static void Register(sol::state* lua);

	void SetAction(ItemOptions a_action);
	void SetSlot(ItemEnumPair a_slot);
};
