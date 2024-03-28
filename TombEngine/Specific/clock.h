#pragma once
#include "Game/control/control.h"

// This might not be the exact amount of time that has passed, but giving it a
// value of 1/30 keeps it in lock-step with the rest of the game logic,
// which assumes 30 iterations per second.
constexpr auto FPS		  = 30;
constexpr auto DELTA_TIME = 1.0f / FPS;
constexpr auto TIME_UNIT  = 60;
constexpr auto DAY_UNIT   = 24;

struct GameTime
{
	int Days	= 0;
	int Hours	= 0;
	int Minutes = 0;
	int Seconds = 0;
};

int	 Sync();
bool TimeInit();
bool TimeReset();

GameTime GetGameTime(int frameCount);

bool TestGlobalTimeInterval(float seconds);
