#include "framework.h"
#include "Specific/clock.h"

LARGE_INTEGER PerformanceCount;
double LdFreq;
double LdSync;

bool TimeReset()
{
	LARGE_INTEGER fq;
	QueryPerformanceCounter(&fq);
	LdSync = (double)fq.LowPart + (double)fq.HighPart * (double)0xffffffff;
	LdSync /= LdFreq;
	return true;
}

bool TimeInit()
{
	LARGE_INTEGER fq;
	if (!QueryPerformanceFrequency(&fq))
		return false;
	LdFreq = (double)fq.LowPart + (double)fq.HighPart * (double)0xFFFFFFFF;
	LdFreq /= 60.0;
	TimeReset();
	return true;
}

int Sync()
{
	LARGE_INTEGER ct;
	double dCounter;
	QueryPerformanceCounter(&ct);
	dCounter = (double)ct.LowPart + (double)ct.HighPart * (double)0xFFFFFFFF;
	dCounter /= LdFreq;
	long nFrames = long(dCounter) - long(LdSync);
	LdSync = dCounter;
	return nFrames;
}
