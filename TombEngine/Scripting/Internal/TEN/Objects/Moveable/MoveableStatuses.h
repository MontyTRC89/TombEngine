#pragma once

#include "Game/items.h"

/// Constants for moveable statuses.
// @enum Objects.MoveableStatus
// @pragma nostrip

/// Table of Objects.MoveableStatus constants.
//
// To be used with @{Objects.Moveable.GetStatus} and @{Objects.Moveable.SetStatus} functions.
//
// - `INACTIVE` - moveable is inactive (was never activated).
// - `ACTIVE` - moveable is active.
// - `DEACTIVATED` - moveable is deactivated (was previously active and later deactivated).
// - `INVISIBLE` - moveable is invisible.
//
// @table Objects.MoveableStatus

static const auto MOVEABLE_STATUSES = std::unordered_map<std::string, ItemStatus>
{
	{ "INACTIVE", ItemStatus::ITEM_NOT_ACTIVE },
	{ "ACTIVE", ItemStatus::ITEM_ACTIVE },
	{ "DEACTIVATED", ItemStatus::ITEM_DEACTIVATED },
	{ "INVISIBLE", ItemStatus::ITEM_INVISIBLE }
};
