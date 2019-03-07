// JayH

#include "EngineMinimal.h"
#include "GameTimer.h"


GameTimer::GameTimer()
{
	// 1초당 성능 개수를 구해온다.
	__int64 countsPerSec;
	QueryPerformanceFrequency(OUT (LARGE_INTEGER*)&countsPerSec);

	// 역수를 취해서 성능 개수당 시간(초)을 구한다.
	mSecondsPerCount = 1.0 / (double)countsPerSec;
}


float GameTimer::GetTotalTimeSeconds() const
{
	// 만약 게임이 정지된다면 게임 시간은 멈춰야 한다.
	// 따라서 게임이 정지됬던 시간만큼의 양을 구해서 실제 시간에서 그 양을 빼줘야한다 
	//
	//                     |<--paused time-->|
	// ----*---------------*-----------------*------------*------------*------> time
	//  mBaseTime       mStopTime        startTime     mStopTime    mCurrTime

	if ( bStopped == true )
	{
		return (float)(((mStopTime - mPausedTime) - mBaseTime) * mSecondsPerCount);
	}
	else
	{
		return (float)(((mCurrTime - mPausedTime) - mBaseTime) * mSecondsPerCount);
	}
}


float GameTimer::GetDeltaTimeSeconds() const
{
	return (float)mDeltaTime;
}


void GameTimer::Reset()
{
	// 현재 시간을 구해서 모두 초기화 한다.

	__int64 currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);

	mBaseTime = currTime;
	mPrevTime = currTime;
	mStopTime = 0;
	bStopped = false;
}


void GameTimer::Start()
{
	__int64 startTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&startTime);

	if ( bStopped == true )
	{
		mPausedTime += (startTime - mStopTime);

		mPrevTime = startTime;
		mStopTime = 0;
		bStopped = false;
	}
}


void GameTimer::Stop()
{
	if ( bStopped == false )
	{
		__int64 currTime;
		QueryPerformanceCounter((LARGE_INTEGER*)&currTime);

		mStopTime = currTime;
		bStopped = true;
	}
}


void GameTimer::Tick()
{
	if ( bStopped == true )
	{
		mDeltaTime = 0.0;
		return;
	}

	__int64 currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);
	mCurrTime = currTime;

	// 현재 프레임과 이전 프레임의 시간차를 구한다.
	mDeltaTime = (mCurrTime - mPrevTime)*mSecondsPerCount;

	mPrevTime = mCurrTime;

	if ( mDeltaTime < 0.0 )
	{
		mDeltaTime = 0.0f;
	}
}