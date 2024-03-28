#include "framework.h"
#include "Specific/clock.h"

// Globals
LARGE_INTEGER PerformanceCount = {};
double		  LdFreq		   = 0.0;
double		  LdSync		   = 0.0;

int Sync()
{
	auto ct = LARGE_INTEGER{};
	QueryPerformanceCounter(&ct);

	double dCounter = (double)ct.LowPart + (double)ct.HighPart * (double)0xffffffff;
	dCounter /= LdFreq;

	long ticks = (long)dCounter - (long)LdSync;
	LdSync = dCounter;
	return ticks;
}

bool TimeReset()
{
	auto fq = LARGE_INTEGER{};
	QueryPerformanceCounter(&fq);

	LdSync = (double)fq.LowPart + ((double)fq.HighPart * (double)0xffffffff);
	LdSync /= LdFreq;
	return true;
}

bool TimeInit()
{
	auto fq = LARGE_INTEGER{};
	if (!QueryPerformanceFrequency(&fq))
		return false;

	LdFreq = (double)fq.LowPart + ((double)fq.HighPart * (double)0xffffffff);
	LdFreq /= 60.0;
	TimeReset();
	return true;
}

GameTime GetGameTime(int ticks)
{
	auto gameTime = GameTime{};
	int seconds = ticks / FPS;

	gameTime.Days    = (seconds / (DAY_UNIT * SQUARE(TIME_UNIT)));
	gameTime.Hours   = (seconds % (DAY_UNIT * SQUARE(TIME_UNIT))) / SQUARE(TIME_UNIT);
	gameTime.Minutes = (seconds / TIME_UNIT) % TIME_UNIT;
	gameTime.Seconds = seconds % TIME_UNIT;
	return gameTime;
}

bool TestGlobalTimeInterval(float intervalSecs, float offsetSecs)
{
	int intervalTicks = (int)round(intervalSecs * FPS);
	int offsetTicks = (int)round(offsetSecs * FPS);

	if (offsetTicks >= intervalTicks)
	{
		TENLog("TestGlobalTimeInterval(): intervalSecs must be greater than offsetSecs.", LogLevel::Warning);
		return false;
	}

	return ((GlobalCounter % intervalTicks) == offsetTicks);
}
