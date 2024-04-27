#pragma once
#include "Game/items.h"
#include "Math/Math.h"

using namespace TEN::Math;

void InitializeWaterfall(short itemNumber);
void TriggerWaterfallEmitterMist(const Vector3& pos, short room, short scalar, short size);
void WaterfallControl(short itemNumber);
