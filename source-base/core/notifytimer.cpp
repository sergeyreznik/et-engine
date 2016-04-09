/*
 * This file is part of `et engine`
 * Copyright 2009-2016 by Sergey Reznik
 * Please, modify content only if you know what are you doing.
 *
 */

#include <et/core/notifytimer.h>

using namespace et;

NotifyTimer::NotifyTimer() :
	_endTime(0.0f), _period(0.0f), _repeatCount(0) { }

void NotifyTimer::start(TimerPool* tp, float period, int64_t repeatCount)
{
	ET_ASSERT(tp != nullptr);

	startUpdates(tp);

	_period = period;
	_repeatCount = repeatCount;
	_endTime = actualTime() + period;
}

void NotifyTimer::start(TimerPool::Pointer tp, float period, int64_t repeatCount)
{
	start(tp.ptr(), period, repeatCount);
}

void NotifyTimer::update(float t)
{
	if (t >= _endTime)
	{
		_repeatCount--;

		if (_repeatCount == -1)
			cancelUpdates();
		else 
			_endTime = t + _period;

		expired.invoke(this);
	}
}
