#include "framework.h"
#include "GameScriptInventoryObject.h"

GameScriptInventoryObject::GameScriptInventoryObject(std::string name, short slot, float yOffset, float scale, float xRot, float yRot, float zRot, short rotationFlags, int meshBits, __int64 operation)
	{
		this->name = name;
		this->slot = slot;
		this->yOffset = yOffset;
		this->scale = scale;
		this->xRot = xRot;
		this->yRot = yRot;
		this->zRot = zRot;
		this->rotationFlags = rotationFlags;
		this->meshBits = meshBits;
		this->operation = operation;
	}
void GameScriptInventoryObject::Register(sol::state * lua)
{
	lua->new_usertype<GameScriptInventoryObject>("InventoryObject",
		sol::constructors<GameScriptInventoryObject(std::string, short, float, float, float, float, float, short, int, __int64)>(),
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
