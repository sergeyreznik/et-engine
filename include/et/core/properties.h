/*
 * This file is part of `et engine`
 * Copyright 2009-2013 by Sergey Reznik
 * Please, do not modify content without approval.
 *
 */

#pragma once

#define ET_DECLARE_PROPERTY_GET_REF_SET_REF(TYPE, NAME, SETTER) public: \
	const TYPE& NAME() const { return _##NAME; } \
	void SETTER(const TYPE& new##NAME) { _##NAME = new##NAME; } \
private: \
	TYPE _##NAME;

#define ET_DECLARE_PROPERTY_GET_COPY_SET_COPY(TYPE, NAME, SETTER) public: \
	TYPE NAME() const { return _##NAME; } \
	void SETTER(TYPE new##NAME) { _##NAME = new##NAME; } \
private: \
	TYPE _##NAME;

#define ET_DECLARE_PROPERTY_GET_REF(TYPE, NAME) public: \
	const TYPE& NAME() const { return _##NAME; } \
	private: \
		TYPE _##NAME;
