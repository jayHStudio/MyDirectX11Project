// JayH

#pragma once


class GameTimer
{

public:
	GameTimer();

	// 게임이 시작되고 경과된 시간을 리턴한다. 게임이 중지된 시간은 포함하지 않는다.
	// @ return	float	경과된 시간을 초 단위로 리턴한다.
	float GetTotalTimeSeconds() const;

	// 현재 프레임과 이전프레임의 시간차를 리턴한다.
	// @ return	float	시간차를 초 단위로 리턴한다.
	float GetDeltaTimeSeconds() const;

	void Reset();
	void Start();
	void Stop();
	void Tick();

private:
	double mSecondsPerCount = 0.0;
	double mDeltaTime = -1.0;

	__int64 mBaseTime = 0;
	__int64 mPausedTime = 0;
	__int64 mStopTime = 0;
	__int64 mPrevTime = 0;
	__int64 mCurrTime = 0;

	bool bStopped = false;

};

