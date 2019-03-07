// JayH

#pragma once


class GameTimer
{

public:
	GameTimer();

	// ������ ���۵ǰ� ����� �ð��� �����Ѵ�. ������ ������ �ð��� �������� �ʴ´�.
	// @ return	float	����� �ð��� �� ������ �����Ѵ�.
	float GetTotalTimeSeconds() const;

	// ���� �����Ӱ� ������������ �ð����� �����Ѵ�.
	// @ return	float	�ð����� �� ������ �����Ѵ�.
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

