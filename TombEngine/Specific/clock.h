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

class HighFramerateSynchronizer
{
private:
	LARGE_INTEGER _lastTime;
	LARGE_INTEGER _currentTime;
	LARGE_INTEGER _frequency;
	double _controlDelay = 0.0;
	double _frameTime    = 0.0;

public:
	void Init();
	void Sync();
	void Step();
	bool Synced();
	float GetInterpolationFactor();
};

int	 TimeSync();
bool TimeInit();
bool TimeReset();

GameTime GetGameTime(int ticks);

bool TestGlobalTimeInterval(float intervalSecs, float offsetSecs = 0.0f);

extern HighFramerateSynchronizer g_Synchronizer;