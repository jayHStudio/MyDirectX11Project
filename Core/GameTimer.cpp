// JayH

#include "EngineMinimal.h"
#include "GameTimer.h"


GameTimer::GameTimer()
{
	// 1�ʴ� ���� ������ ���ؿ´�.
	__int64 countsPerSec;
	QueryPerformanceFrequency(OUT (LARGE_INTEGER*)&countsPerSec);

	// ������ ���ؼ� ���� ������ �ð�(��)�� ���Ѵ�.
	mSecondsPerCount = 1.0 / (double)countsPerSec;
}


float GameTimer::GetTotalTimeSeconds() const
{
	// ���� ������ �����ȴٸ� ���� �ð��� ����� �Ѵ�.
	// ���� ������ ������� �ð���ŭ�� ���� ���ؼ� ���� �ð����� �� ���� ������Ѵ� 
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
	// ���� �ð��� ���ؼ� ��� �ʱ�ȭ �Ѵ�.

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

	// ���� �����Ӱ� ���� �������� �ð����� ���Ѵ�.
	mDeltaTime = (mCurrTime - mPrevTime)*mSecondsPerCount;

	mPrevTime = mCurrTime;

	if ( mDeltaTime < 0.0 )
	{
		mDeltaTime = 0.0f;
	}
}