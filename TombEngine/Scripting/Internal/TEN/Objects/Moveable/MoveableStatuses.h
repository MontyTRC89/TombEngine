#pragma once
#include <string>
#include <unordered_map>

#include "Game/items.h"

/***
Constants for moveable statuses.
@enum Objects.MoveableStatus
@pragma nostrip
*/

/*** Objects.MoveableStatus constants.

The following constants are inside Objects.MoveableStatus.

	INACTIVE
	ACTIVE
	DEACTIVATED
	INVISIBLE

@section Objects.MoveableStatus
*/

/*** Table of moveable statuses.
@table MoveableStatus
*/

static const std::unordered_map<std::string, ItemStatus> MOVEABLE_STATUSES
{
	{ "INACTIVE", ItemStatus::ITEM_NOT_ACTIVE },
	{ "ACTIVE", ItemStatus::ITEM_ACTIVE },
	{ "DEACTIVATED", ItemStatus::ITEM_DEACTIVATED },
	{ "INVISIBLE", ItemStatus::ITEM_INVISIBLE }
};
