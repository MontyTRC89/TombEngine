#pragma once
#include "Specific/clock.h"

struct ItemInfo;

constexpr auto OPTICS_FADE_SPEED	= 6.0f / FPS;
constexpr auto OPTICS_RANGE_DEFAULT	= ANGLE(0.7f);
constexpr auto OPTICS_RANGE_MIN		= ANGLE(0.7f);
constexpr auto OPTICS_RANGE_MAX		= ANGLE(8.5f);
constexpr auto OPTICS_RANGE_RATE	= ANGLE(0.35f);

bool HandlePlayerOptics(ItemInfo& item);