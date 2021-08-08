#pragma once
#include <string>
#include "ItemEnumPair.h"

namespace sol {
	class state;
}

struct GameScriptInventoryObject
{
	std::string name;
	inv_objects slot;
	float yOffset;
	float scale;
	float xRot;
	float yRot;
	float zRot;
	short rotationFlags;
	int meshBits;
	__int64 operation;

	GameScriptInventoryObject(std::string const & name, ItemEnumPair slot, float yOffset, float scale, float xRot, float yRot, float zRot, short rotationFlags, int meshBits, __int64 operation);

	static void Register(sol::state* lua);
};
