#pragma once
#include "Game/Gui.h"
#include "Scripting/Internal/TEN/Rotation/Rotation.h"

namespace sol { class state; }
using namespace TEN::Gui;

enum GAME_OBJECT_ID : short;

static const std::unordered_map<std::string, RotationFlags> ROTATION_AXES
{
	{ "X", RotationFlags::INV_ROT_X },
	{ "Y", RotationFlags::INV_ROT_Y },
	{ "Z", RotationFlags::INV_ROT_Z }
};

static const std::unordered_map<std::string, ItemOptions> ITEM_MENU_ACTIONS
{
	{ "USE", ItemOptions::OPT_USE },
	{ "EQUIP", ItemOptions::OPT_EQUIP },
	{ "EXAMINE", ItemOptions::OPT_EXAMINABLE },
	{ "COMBINE", ItemOptions::OPT_COMBINABLE },
	{ "SEPARATE", ItemOptions::OPT_SEPERABLE },
	{ "LOAD", ItemOptions::OPT_LOAD },
	{ "SAVE", ItemOptions::OPT_SAVE },
	{ "STATS", ItemOptions::OPT_STATS },
	{ "DIARY", ItemOptions::OPT_DIARY },
	{ "ALWAYS_COMBINE", ItemOptions::OPT_ALWAYS_COMBINE },
	{ "AMMO_PISTOLS", ItemOptions::OPT_CHOOSE_AMMO_PISTOLS },
	{ "AMMO_SHOTGUN", ItemOptions::OPT_CHOOSE_AMMO_SHOTGUN },
	{ "AMMO_UZI", ItemOptions::OPT_CHOOSE_AMMO_UZI },
	{ "AMMO_REVOLVER", ItemOptions::OPT_CHOOSE_AMMO_REVOLVER },
	{ "AMMO_HARPOON", ItemOptions::OPT_CHOOSE_AMMO_HARPOON },
	{ "AMMO_CROSSBOW", ItemOptions::OPT_CHOOSE_AMMO_CROSSBOW },
	{ "AMMO_HK", ItemOptions::OPT_CHOOSE_AMMO_HK },
	{ "AMMO_GRENADEGUN", ItemOptions::OPT_CHOOSE_AMMO_GRENADEGUN },
	{ "AMMO_ROCKET", ItemOptions::OPT_CHOOSE_AMMO_ROCKET }
};

struct InventoryItem
{
	static void Register(sol::table& lua);

	InventoryItem(const std::string& name, GAME_OBJECT_ID objectID, float yOffset, float scale, const Rotation& rot, RotationFlags rotFlags, int meshBits, ItemOptions menuAction);

	void SetAction(ItemOptions action);

	std::string			 Name	  = {};
	InventoryObjectTypes ObjectID = INV_OBJECT_PISTOLS;

	float YOffset = 0.0f;
	float Scale	  = 1.0f;

	Rotation	  Rot		 = Rotation();
	RotationFlags RotFlags	 = RotationFlags::INV_ROT_X;
	int			  MeshBits	 = 0;
	ItemOptions	  MenuAction = ItemOptions::OPT_USE;
};
