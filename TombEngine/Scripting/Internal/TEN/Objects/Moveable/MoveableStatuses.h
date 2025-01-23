#pragma once

#include "Game/items.h"

/***
Constants for moveable statuses.
@enum Objects.MoveableStatus
@pragma nostrip
*/

/*** Table of Objects.MoveableStatus constants.

To be used with @{Objects.Moveable.GetStatus} and @{Objects.Moveable.SetStatus} functions.

 - `INACTIVE` - object was never activated.
 - `ACTIVE` - object is active.
 - `DEACTIVATED` - object was active before and was deactivated.
 - `INVISIBLE` - object is invisible.

@table Objects.MoveableStatus
*/

static const std::unordered_map<std::string, ItemStatus> MOVEABLE_STATUSES
{
	{ "INACTIVE", ItemStatus::ITEM_NOT_ACTIVE },
	{ "ACTIVE", ItemStatus::ITEM_ACTIVE },
	{ "DEACTIVATED", ItemStatus::ITEM_DEACTIVATED },
	{ "INVISIBLE", ItemStatus::ITEM_INVISIBLE }
};
