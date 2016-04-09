/*
 * This file is part of `et engine`
 * Copyright 2009-2016 by Sergey Reznik
 * Please, modify content only if you know what are you doing.
 *
 */

#pragma once

#include <et/core/timerpool.h>
#include <et/core/taskpool.h>

namespace et
{
	class RunLoop : public Shared
	{
	public:
		ET_DECLARE_POINTER(RunLoop)
		
	public:
		RunLoop();
		virtual ~RunLoop();
		
		float time() const
			{ return _time; }
		
		uint64_t timeMSec() const
			{ return _actualTimeMSec; }

		TimerPool::Pointer& firstTimerPool()
			{ return _timerPools.front(); }
		
		void updateTime(uint64_t t);
		void update(uint64_t t);

		void pause();
		void resume();

		void attachTimerPool(const TimerPool::Pointer&);
		void detachTimerPool(const TimerPool::Pointer&);
		void detachAllTimerPools();

		virtual void addTask(Task*, float);
		
		bool hasTasks()
			{ return _taskPool.hasTasks(); }

	private:
		std::vector<TimerPool::Pointer> _timerPools;
		TaskPool _taskPool;
		uint64_t _actualTimeMSec = 0;
		uint64_t _activityTimeMSec = 0;
		float _time = 0.0f;
		bool _started = false;
		bool _active = true;
	};
}
