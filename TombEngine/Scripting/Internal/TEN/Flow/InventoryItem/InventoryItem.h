#pragma once
#include "Game/Gui.h"
#include "Scripting/Internal/TEN/Rotation/Rotation.h"

using namespace TEN::Gui;

enum GAME_OBJECT_ID : short;

static const std::unordered_map<std::string, RotationFlags> kRotAxes
{
	{ "X", RotationFlags::INV_ROT_X },
	{ "Y", RotationFlags::INV_ROT_Y },
	{ "Z", RotationFlags::INV_ROT_Z }
};

static const std::unordered_map<std::string, ItemOptions> kItemActions
{
	{ "USE", ItemOptions::OPT_USE },
	{ "EQUIP", ItemOptions::OPT_EQUIP },
	{ "EXAMINE", ItemOptions::OPT_EXAMINABLE },
	{ "COMBINE", ItemOptions::OPT_COMBINABLE }
};

namespace sol
{
	class state;
}

struct InventoryItem
{
	static void Register(sol::table& lua);

	InventoryItem(const std::string& name, GAME_OBJECT_ID objectID, short yOffset, float scale, const Rotation& rot, RotationFlags rotFlags, int meshBits, ItemOptions menuAction);

	std::string Name = {};
	InventoryObjectTypes slot{ INV_OBJECT_PISTOLS };
	short yOffset{ 0 };
	float scale{ 1.0f };
	Rotation rot{};
	RotationFlags rotationFlags{ RotationFlags::INV_ROT_X };
	int meshBits{ 0 };
	ItemOptions MenuAction = ItemOptions::OPT_USE;

	void SetAction(ItemOptions action);
};
