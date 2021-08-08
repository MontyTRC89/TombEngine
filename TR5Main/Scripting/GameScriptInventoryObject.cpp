#include "framework.h"
#include "GameScriptInventoryObject.h"

GameScriptInventoryObject::GameScriptInventoryObject(std::string const& name, ItemEnumPair slot, float yOffset, float scale, float xRot, float yRot, float zRot, short rotationFlags, int meshBits, __int64 operation) : name{ name }, slot{ slot.m_pair.second }, yOffset{ yOffset }, scale{ scale }, xRot{ xRot }, yRot{ yRot }, zRot{ zRot }, rotationFlags{ rotationFlags }, meshBits{ meshBits }, operation{ operation }
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
