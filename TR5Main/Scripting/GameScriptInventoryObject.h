#pragma once
#include <string>
#include "ItemEnumPair.h"
#include "GameScriptRotation.h"
#include "newinv2.h"

static const std::unordered_map<std::string, rotflags> kRotAxes{
	{"X", rotflags::INV_ROT_X},
	{"Y", rotflags::INV_ROT_Y},
	{"Z", rotflags::INV_ROT_Z}
};

static const std::unordered_map<std::string, item_options> kItemActions{
	{"USE", item_options::OPT_USE},
	{"EQUIP", item_options::OPT_EQUIP},
	{"EXAMINE", item_options::OPT_EXAMINABLE}
};

namespace sol {
	class state;
}

struct GameScriptInventoryObject
{
	std::string name;
	inv_objects slot;
	float yOffset;
	float scale;
	GameScriptRotation rot;
	rotflags rotationFlags;
	int meshBits;
	item_options action;

	GameScriptInventoryObject(std::string const & a_name, ItemEnumPair a_slot, float a_yOffset, float a_scale, GameScriptRotation const & a_rot, rotflags a_rotationFlags, int a_meshBits, item_options a_actions);

	static void Register(sol::state* lua);

	void SetAction(item_options a_action);
};
