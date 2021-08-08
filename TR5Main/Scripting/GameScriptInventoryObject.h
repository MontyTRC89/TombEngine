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

	GameScriptInventoryObject(std::string const & a_name, ItemEnumPair a_slot, float a_yOffset, float a_scale, float a_xRot, float a_yRot, float a_zRot, short a_rotationFlags, int a_meshBits, __int64 a_operation);

	static void Register(sol::state* lua);
};
