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

void UpdateTimeInformation(TimeData* tData)
{
	// get current time
	FetchPerformanceCounter(&tData->currTime);
	tData->deltaTime = (tData->currTime - tData->prevTime) * tData->secondsPerCount;
	tData->prevTime = tData->currTime;

	// Force non negative
	tData->deltaTime = tData->deltaTime < 0.0 ? 0.0 : tData->deltaTime;

	// update total time
	tData->totalTime += tData->deltaTime;

}

void ResetTimeInformation(TimeData* tData)
{
	FetchPerformanceCounter(&tData->baseTime);
	tData->currTime = tData->baseTime;
	tData->prevTime = tData->baseTime;
	FetchPerformanceFreq(&tData->countsPerSec);
	tData->secondsPerCount = 1.0 / static_cast<r64>(tData->countsPerSec);
	tData->deltaTime = 0.0;
	tData->totalTime = 0.0;
}
