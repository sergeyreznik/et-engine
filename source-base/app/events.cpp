/*
 * This file is part of `et engine`
 * Copyright 2009-2016 by Sergey Reznik
 * Please, modify content only if you know what are you doing.
 *
 */

#include <et/app/events.h>

using namespace et;

/*
 * EventReceiver
 */
void EventReceiver::eventConnected(Event* e)
{
	_events.push_back(e); 
}

EventReceiver::~EventReceiver() 
{
	for (auto i : _events)
		i->receiverDisconnected(this);
}

void EventReceiver::eventDisconnected(Event* e)
{
	auto i = std::find(_events.begin(), _events.end(), e);
	if (i != _events.end())
		_events.erase(i);
}

/**
 *
 * Event0
 *
 */
Event0::~Event0()
{
	for (auto& connection : _connections)
	{
		if ((connection->receiver() != nullptr) && !connection->removed())
			connection->receiver()->eventDisconnected(this);
		etDestroyObject(connection);
	}
}

void Event0::cleanup()
{
    auto i = remove_if(_connections.begin(), _connections.end(), [](EventConnectionBase* b) { return (b == nullptr) || b->removed(); });
    _connections.erase(i, _connections.end());
}

void Event0::invoke()
{
	cleanup();

	_invoking = true;
	for (auto& conn : _connections)
		conn->invoke();
	_invoking = false;
}

void Event0::invokeInMainRunLoop(float delay)
{
	cleanup();
	
	_invoking = true;
	for (auto& i : _connections)
		i->invokeInMainRunLoop(delay);
	_invoking = false;
}

void Event0::invokeInBackground(float delay)
{
	cleanup();
	
	_invoking = true;
	for (auto& i : _connections)
		i->invokeInBackground(delay);
	_invoking = false;
}

void Event0::receiverDisconnected(EventReceiver* r)
{
	auto i = _connections.begin();
	while (i != _connections.end())
	{
		if (r == (*i)->receiver())
		{
			if (_invoking)
			{
				(*i)->remove();
				++i;
			}
			else
			{
				etDestroyObject(*i);
				i = _connections.erase(i);
			}
		}
		else 
		{
			++i;
		}
	}
}
