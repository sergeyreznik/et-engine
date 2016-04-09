/*
 * This file is part of `et engine`
 * Copyright 2009-2016 by Sergey Reznik
 * Please, modify content only if you know what are you doing.
 *
 */

#pragma once

namespace et
{
	template <typename T>
	class Derived
	{
	public:
		typedef T AncestorClass;
	};

	template <typename T, typename BASE>
	class Hierarchy : public BASE
	{
	public:
		using BasePointer = IntrusivePtr<T>;
		using List = Vector<BasePointer>;
		
	public:
		Hierarchy(T* parent);
		virtual ~Hierarchy();

		virtual void setParent(T* p);

		T* parent();
		const T* parent() const;

		List& children();
		const List& children() const;

		virtual void bringToFront(T*);
		virtual void sendToBack(T*);

	protected:
		virtual void childRemoved(T*) { }
		void removeChildren();

	private:
		bool addChild(T* c);
		void pushChild(T* c);
		
		bool removeChild(T* c);
		void removeParent();

	private:
		T* _parent = nullptr;
		List _children;
	};

	template <typename T, typename BASE>
	Hierarchy<T, BASE>::Hierarchy(T* parent) : _parent(parent)
	{
		if (_parent)
			_parent->pushChild(static_cast<T*>(this));
	}

	template <typename T, typename BASE>
	Hierarchy<T, BASE>::~Hierarchy()
	{
		removeChildren();
	}

	template <typename T, typename BASE>
	void Hierarchy<T, BASE>::setParent(T* p)
	{
		BASE::retain();

		if (_parent)
			_parent->removeChild(static_cast<T*>(this));

		_parent = p;

		if (_parent)
			_parent->addChild(static_cast<T*>(this));

		BASE::release();
	}

	template <typename T, typename BASE>
	void Hierarchy<T, BASE>::pushChild(T* c)
		{ _children.push_back(typename Hierarchy<T, BASE>::BasePointer(c)); }

	template <typename T, typename BASE>
	bool Hierarchy<T, BASE>::addChild(T* c)
	{
		if (c == this) return false;

		bool notFound = true;
		for (auto i = _children.begin(), e = _children.end(); i != e; ++i)
		{
			if (i->ptr() == c)
			{
				notFound = false;
				break;
			}
		}

		if (notFound)
			pushChild(c);

		return notFound;
	}

	template <typename T, typename BASE>
	bool Hierarchy<T, BASE>::removeChild(T* c)
	{
		bool found = false;
		
		for (auto i = _children.begin(), e = _children.end(); i != e; ++i)
		{
			if (i->ptr() == c)
			{
				childRemoved(c);
				
				_children.erase(i);
				found = true;
				break;
			}
		}

		return found;
	}

	template <typename T, typename BASE>
	void Hierarchy<T, BASE>::bringToFront(T* c)
	{
		for (auto i = _children.begin(), e = _children.end(); i != e; ++i)
		{
			if (i->ptr() == c)
			{
				auto pointer = *i;

				_children.erase(i);
				_children.push_back(pointer);

				break;
			}
		}
	}

	template <typename T, typename BASE>
	void Hierarchy<T, BASE>::sendToBack(T* c)
	{
		for (auto i = _children.begin(), e = _children.end(); i != e; ++i)
		{
			if (i->ptr() == c)
			{
				auto pointer = *i;

				_children.erase(i);
				_children.insert(_children.begin(), pointer);

				break;
			}
		}
	}

	template <typename T, typename BASE>
	void Hierarchy<T, BASE>::removeChildren()
	{
		for (auto c : _children)
			c->removeParent();
		
		_children.clear();
	}

	template <typename T, typename BASE>
	inline T* Hierarchy<T, BASE>::parent()
		{ return _parent; }

	template <typename T, typename BASE>
	inline const T* Hierarchy<T, BASE>::parent() const
		{  return _parent; }

	template <typename T, typename BASE>
	inline const typename Hierarchy<T, BASE>::List& Hierarchy<T, BASE>::children() const
		{ return _children; }

	template <typename T, typename BASE>
	inline typename Hierarchy<T, BASE>::List& Hierarchy<T, BASE>::children()
		{ return _children; }

	template <typename T, typename BASE>
	void Hierarchy<T, BASE>::removeParent()
		{ _parent = nullptr; }

}
