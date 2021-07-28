#pragma once
#include <string>

namespace sol {
	class state;
}

struct GameScriptInventoryObject
{
	std::string name;
	short slot;
	float yOffset;
	float scale;
	float xRot;
	float yRot;
	float zRot;
	short rotationFlags;
	int meshBits;
	__int64 operation;

	GameScriptInventoryObject::GameScriptInventoryObject(std::string name, short slot, float yOffset, float scale, float xRot, float yRot, float zRot, short rotationFlags, int meshBits, __int64 operation);

	static void Register(sol::state* lua);
};
