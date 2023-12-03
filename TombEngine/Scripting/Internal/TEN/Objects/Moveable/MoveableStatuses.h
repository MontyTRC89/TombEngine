#pragma once
#include <string>
#include <unordered_map>

#include "Game/items.h"

/***
Constants for moveable statuses.
@enum MoveableEnums.Status
@pragma nostrip
*/

/*** MoveableEnums.Status constants.

The following constants are inside MoveableEnums.Status.

INACTIVE
ACTIVE
DEACTIVATED
INVISIBLE

@section MoveableEnums.Status
*/

/*** Table of moveable statuses.
@table CONSTANT_STRING_HERE
*/

static const std::unordered_map<std::string, ItemStatus> MOVEABLE_STATUSES
{
	{ "INACTIVE", ITEM_NOT_ACTIVE },
	{ "ACTIVE", ITEM_ACTIVE },
	{ "DEACTIVATED", ITEM_DEACTIVATED },
	{ "INVISIBLE", ITEM_INVISIBLE }
};
