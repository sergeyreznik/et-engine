/*
 * This file is part of `et engine`
 * Copyright 2009-2016 by Sergey Reznik
 * Please, modify content only if you know what are you doing.
 *
 */

#pragma once

#include <et/core/timedobject.h>
#include <et/app/events.h>

namespace et
{
template <typename T>
class InertialValue : public TimedObject
{
public:
	InertialValue() :
		_velocity(0), _value(0), _deccelerationRate(1.0f), _latestDelta(0), _time(0.0f),
		_precision(std::numeric_limits<float>::epsilon())
	{
	}

	InertialValue(const T& val) :
		_velocity(0), _value(val), _deccelerationRate(1.0f), _latestDelta(0), _time(0.0f),
		_precision(std::numeric_limits<float>::epsilon())
	{
	}

	const T& value() const { return _value; }
	const T& velocity() const { return _velocity; }
	const T& latestDelta() const { return _latestDelta; }

	void setDeccelerationRate(float value) { _deccelerationRate = value; }

public:
	void addVelocity(const T& t)
	{
		_velocity += t;
	}

	void scaleVelocity(const T& t)
	{
		_velocity *= t;
	}

	void addValue(const T& v)
	{
		_value += v;
	}

	void setPrecision(float p)
	{
		_precision = p;
	}

	void run()
	{
		TimedObject::startUpdates(nullptr);
		_time = actualTime();
	}

public:
	ET_DECLARE_EVENT0(updated);
	ET_DECLARE_EVENT1(valueUpdated, const T&);

private:
	void startUpdates(TimerPool* timerPool = nullptr) override
	{
		TimedObject::startUpdates(timerPool);
	}

	void update(float t) override
	{
		float dt = _deccelerationRate * (t - _time);
		_velocity *= std::max(0.0f, 1.0f - dt);

		_latestDelta = dt * _velocity;

		if (length(_latestDelta) > _precision)
		{
			_value += _latestDelta;
			valueUpdated.invoke(_value);
			updated.invoke();
		}

		_time = t;
	}

private:
	T _value;
	T _velocity;
	T _latestDelta;
	float _time = 0.0f;
	float _precision = 0.0f;
	float _deccelerationRate = 1.0f;
};
}
