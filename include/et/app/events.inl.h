/*
 * This file is part of `et engine`
 * Copyright 2009-2016 by Sergey Reznik
 * Please, modify content only if you know what are you doing.
 *
 */

/*
 * Event0Connection
 */

template <typename T>
Event0Connection<T>::Event0Connection(T* receiver, void(T::*func)()) :  
	_receiverMethod(func), _receiver(receiver)
{
}

/*
 * Event0
 */

template <typename R>
inline void Event0::connect(R* receiver, void (R::*receiverMethod)())
{
	for (auto& connection : _connections)
	{
		if (connection->receiver() == receiver)
		{
			connection->setRemoved(false);
			return;
		}
	}

	_connections.push_back(etCreateObject<Event0Connection<R>>(receiver, receiverMethod));
	receiver->eventConnected(this);
}

template <typename F>
inline void Event0::connect(F func)
{
	_connections.push_back(etCreateObject<Event0DirectConnection<F>>(func));
}

template <typename R>
void Event0::disconnect(R* receiver)
{
	receiverDisconnected(receiver);
	receiver->eventDisconnected(this);
}

/*
* Event1Connection
*/

template <typename ReceiverType, typename ArgType>
Event1Connection<ReceiverType, ArgType>::Event1Connection(ReceiverType* receiver,
	void(ReceiverType::*func)(ArgType)) : _receiverMethod(func), _receiver(receiver)
{
}

/*
* Event1
*/
template <typename ArgType>
Event1<ArgType>::~Event1()
{
	for (auto& connection : _connections)
	{
		if ((connection->receiver() != nullptr) && !connection->removed())
			connection->receiver()->eventDisconnected(this);
		etDestroyObject(connection);
	}
}

template <typename ArgType>
template <typename ReceiverType>
inline void Event1<ArgType>::connect(ReceiverType* receiver, void (ReceiverType::*receiverMethod)(ArgType))
{
	static_assert(std::is_base_of<EventReceiver, ReceiverType>::value, 
		"Receiver object should be derived from et::EventReceiver");

	for (auto& connection : _connections)
	{
		if (connection->receiver() == receiver)
		{
			connection->setRemoved(false);
			return;
		}
	}

	_connections.push_back(etCreateObject<Event1Connection<ReceiverType, ArgType>>(receiver, receiverMethod));
	receiver->eventConnected(this);
}

template <typename ArgType>
template <typename F>
inline void Event1<ArgType>::connect(F func)
{
	_connections.push_back(etCreateObject<Event1DirectConnection<F, ArgType>>(func));
}

template <typename ArgType>
template <typename ReceiverType>
inline void Event1<ArgType>::disconnect(ReceiverType* receiver)
{
	receiverDisconnected(receiver);
	receiver->eventDisconnected(this);
}

template <typename ArgType>
inline void Event1<ArgType>::receiverDisconnected(EventReceiver* r)
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

template <typename ArgType>
inline void Event1<ArgType>::cleanup()
{
    auto i = std::remove_if(_connections.begin(), _connections.end(), [](EventConnectionBase* b) { return (b == nullptr) || b->removed(); });
    _connections.erase(i, _connections.end());
}

template <typename ArgType>
inline void Event1<ArgType>::invoke(ArgType arg)
{
	cleanup();
	
	_invoking = true;
	for (auto& conn : _connections)
		conn->invoke(arg);
	_invoking = false;
}

template <typename ArgType>
inline void Event1<ArgType>::invokeInMainRunLoop(ArgType arg, float delay)
{
	cleanup();
	
	_invoking = true;
	for (auto& conn : _connections)
		conn->invokeInMainRunLoop(arg, delay);
	_invoking = false;
}

template <typename ArgType>
inline void Event1<ArgType>::invokeInCurrentRunLoop(ArgType arg, float delay)
{
	cleanup();
	
	_invoking = true;
	for (auto& conn : _connections)
		conn->invokeInCurrentRunLoop(arg, delay);
	_invoking = false;
}

/*
* Event2Connection
*/

template <typename ReceiverType, typename Arg1Type, typename Arg2Type>
Event2Connection<ReceiverType, Arg1Type, Arg2Type>::Event2Connection(ReceiverType* receiver,
	void(ReceiverType::*func)(Arg1Type, Arg2Type)) : _receiverMethod(func), _receiver(receiver)
{
}

/*
* Event2
*/
template <typename Arg1Type, typename Arg2Type>
Event2<Arg1Type, Arg2Type>::~Event2()
{
	for (auto& connection : _connections)
	{
		if ((connection->receiver() != nullptr) && !connection->removed())
			connection->receiver()->eventDisconnected(this);

		etDestroyObject(connection);
	}
}

template <typename Arg1Type, typename Arg2Type>
template <typename ReceiverType>
inline void Event2<Arg1Type, Arg2Type>::connect(ReceiverType* receiver,
	void (ReceiverType::*receiverMethod)(Arg1Type, Arg2Type))
{
	for (auto& i : _connections)
	{
		if (i->receiver() == receiver)
		{
			i->setRemoved(false);
			return;
		}
	}

	_connections.push_back(etCreateObject
		<Event2Connection<ReceiverType, Arg1Type, Arg2Type>>(receiver, receiverMethod));
	
	receiver->eventConnected(this);
}

template <typename ArgType1, typename ArgType2>
template <typename F>
inline void Event2<ArgType1, ArgType2>::connect(F func)
{
	_connections.push_back(etCreateObject<Event2DirectConnection<F, ArgType1, ArgType2>>(func));
}

template <typename Arg1Type, typename Arg2Type>
template <typename ReceiverType>
inline void Event2<Arg1Type, Arg2Type>::disconnect(ReceiverType* receiver)
{
	receiverDisconnected(receiver);
	receiver->eventDisconnected(this);
}

template <typename Arg1Type, typename Arg2Type>
inline void Event2<Arg1Type, Arg2Type>::receiverDisconnected(EventReceiver* r)
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

template <typename Arg1Type, typename Arg2Type>
inline void Event2<Arg1Type, Arg2Type>::invoke(Arg1Type a1, Arg2Type a2)
{
    auto i = remove_if(_connections.begin(), _connections.end(), [](EventConnectionBase* b) { return (b == nullptr) || b->removed(); });
    _connections.erase(i, _connections.end());

	_invoking = true;
	for (auto& conn : _connections)
		conn->invoke(a1, a2);
	_invoking = false;
}

template <typename Arg1Type, typename Arg2Type>
inline void Event2<Arg1Type, Arg2Type>::invokeInMainRunLoop(Arg1Type a1, Arg2Type a2, float delay)
{
    auto i = remove_if(_connections.begin(), _connections.end(), [](EventConnectionBase* b) { return (b == nullptr) || b->removed(); });
    _connections.erase(i, _connections.end());
	
	_invoking = true;
	for (auto& conn : _connections)
		conn->invokeInMainRunLoop(a1, a2, delay);
	_invoking = false;

}

template <typename Arg1Type, typename Arg2Type>
inline void Event2<Arg1Type, Arg2Type>::invokeInCurrentRunLoop(Arg1Type a1, Arg2Type a2, float delay)
{
    auto i = remove_if(_connections.begin(), _connections.end(), [](EventConnectionBase* b) { return (b == nullptr) || b->removed(); });
    _connections.erase(i, _connections.end());
	
	_invoking = true;
	for (auto& conn : _connections)
		conn->invokeInCurrentRunLoop(a1, a2, delay);
	_invoking = false;
	
}
