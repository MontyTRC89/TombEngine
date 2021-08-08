#include "framework.h"
#include "GameScriptInventoryObject.h"

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
		"name", &GameScriptInventoryObject::name,
		"yOffset", &GameScriptInventoryObject::yOffset,
		"scale", &GameScriptInventoryObject::scale,
		"xRot", &GameScriptInventoryObject::xRot,
		"yRot", &GameScriptInventoryObject::yRot,
		"zRot", &GameScriptInventoryObject::zRot,
		"rotationFlags", &GameScriptInventoryObject::rotationFlags,
		"meshBits", &GameScriptInventoryObject::meshBits,
		"operation", &GameScriptInventoryObject::operation
		);
}
