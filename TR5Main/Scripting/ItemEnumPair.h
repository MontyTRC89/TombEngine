#pragma once
#include "gui.h"
#include "objectslist.h"
#include <utility>

struct ItemEnumPair
{
	std::pair<GAME_OBJECT_ID, InventoryObjectTypes> m_pair;
	ItemEnumPair(GAME_OBJECT_ID id, InventoryObjectTypes id2) : m_pair { id, id2 } {}
};
