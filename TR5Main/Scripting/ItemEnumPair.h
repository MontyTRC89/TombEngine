#pragma once
#include "Game/gui.h"
#include "Objects/objectslist.h"
#include <utility>

struct ItemEnumPair
{
	std::pair<GAME_OBJECT_ID, InventoryObjectTypes> m_pair;
	ItemEnumPair(GAME_OBJECT_ID id, InventoryObjectTypes id2) : m_pair { id, id2 } {}
};
