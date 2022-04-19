#include "tsr_profiling.h"

void CalculateFrameStats(TimeData& tData, FrameStats* fStats)
{
	// avg frames per second
	// avg time in ms per frame
	// update Frame stats struct

	static ui64 frameCount = 0;
	static r64 elapsedTime = 0.0;
	++frameCount;

	static ui64 frameCountAvg = 0;
	static r64 elapsedTimeAvg = 0.0;
	++frameCountAvg;

	//fetches every 1/30 of a second
	r64 currTimeInS = tData.currTime * tData.secondsPerCount;
	r64 timeDiff = currTimeInS - elapsedTime;
	if (timeDiff >= 1.0 / 30.0)
	{
		fStats->fps = static_cast<r64>((1.0 / timeDiff) * frameCount);
		fStats->ms_per_frame = (timeDiff / frameCount) * 1000.0;

		frameCount = 0;
		elapsedTime = currTimeInS;
	}

	//fetches every second
	double timeDiffAvg = tData.totalTime - elapsedTimeAvg;
	if (timeDiffAvg >= 1.0) {
		fStats->avgfps = static_cast<r64>(frameCountAvg);
		fStats->avgmspf = 1000.0f / fStats->avgfps;

		frameCountAvg = 0;
		elapsedTimeAvg += 1.0;
	}

}

//TODO(Fran): try to move the queryperformancecounter and frequency to the platform code.

void UpdateTimeInformation(TimeData* tData)
{
	// get current time
#if (defined(_WIN64) && _WIN64)
	QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&tData->currTime));
#endif
	tData->deltaTime = (tData->currTime - tData->prevTime) * tData->secondsPerCount;
	tData->prevTime = tData->currTime;

	// Force non negative
	tData->deltaTime = tData->deltaTime < 0.0 ? 0.0 : tData->deltaTime;

	// update total time
	tData->totalTime += tData->deltaTime;

}

void ResetTimeInformation(TimeData* tData)
{
#if (defined(_WIN64) && _WIN64)
	QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&tData->baseTime));
#endif
	tData->currTime = tData->baseTime;
	tData->prevTime = tData->baseTime;
#if (defined(_WIN64) && _WIN64)
	QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&tData->countsPerSec));
#endif
	tData->secondsPerCount = 1.0 / static_cast<r64>(tData->countsPerSec);
	tData->deltaTime = 0.0;
	tData->totalTime = 0.0;
}
