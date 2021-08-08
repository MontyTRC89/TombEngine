#pragma once
#include "newinv2.h"
#include "objectslist.h"
#include <utility>

struct ItemEnumPair
{
	std::pair<GAME_OBJECT_ID, inv_objects> m_pair;
	ItemEnumPair(GAME_OBJECT_ID id, inv_objects id2) : m_pair { id, id2 } {}
};
