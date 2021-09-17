#pragma once

// This might not be the exact amount of time that has passed, but giving it a
// value of 1/30 keeps it in lock-step with the rest of the game logic,
// which assumes 30 iterations per second.
static constexpr float DELTA_TIME = 1.0f / 30.0f;

int Sync();
bool TimeInit();
bool TimeReset();