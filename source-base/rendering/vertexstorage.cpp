/*
 * This file is part of `et engine`
 * Copyright 2009-2016 by Sergey Reznik
 * Please, modify content only if you know what are you doing.
 *
 */

#include <et/core/datastorage.h>
#include <et/rendering/vertexstorage.h>

using namespace et;

class et::VertexStoragePrivate
{
public:
	VertexStoragePrivate(const VertexDeclaration&, size_t);

public:
	VertexDeclaration decl;
	BinaryDataStorage data;
	size_t capacity = 0;
};

VertexStorage::VertexStorage(const VertexDeclaration& aDecl, size_t capacity)
{
	ET_PIMPL_INIT(VertexStorage, aDecl, capacity);
}

VertexStorage::VertexStorage(const VertexArray::Pointer& va)
{
	auto desc = va->generateDescription();
	ET_ASSERT(desc.declaration.interleaved());
	ET_PIMPL_INIT(VertexStorage, desc.declaration, va->size());
	memcpy(_private->data.binary(), desc.data.binary(), desc.data.dataSize());
}

VertexStorage::~VertexStorage()
{
	ET_PIMPL_FINALIZE(VertexStorage)
}

bool VertexStorage::hasAttribute(VertexAttributeUsage usage) const
{
	return declaration().has(usage);
}

bool VertexStorage::hasAttributeWithType(VertexAttributeUsage usage, DataType type) const
{
	return hasAttribute(usage) && (_private->decl.elementForUsage(usage).type() == type);
}

DataType VertexStorage::attributeType(VertexAttributeUsage usage) const
{
	ET_ASSERT(hasAttribute(usage));
	return _private->decl.elementForUsage(usage).type();
}

BinaryDataStorage& VertexStorage::data()
{
	return _private->data;
}

const BinaryDataStorage& VertexStorage::data() const
{
	return _private->data;
}

size_t VertexStorage::offsetOfAttribute(VertexAttributeUsage usage) const
{
	ET_ASSERT(hasAttribute(usage));
	return declaration().elementForUsage(usage).offset();
}

size_t VertexStorage::sizeOfAttribute(VertexAttributeUsage usage) const
{
	ET_ASSERT(hasAttribute(usage));
	const auto& element = declaration().elementForUsage(usage);
	return sizeOfDataFormat(element.dataFormat()) * dataTypeComponents(element.type());
}

size_t VertexStorage::stride() const
{
	return declaration().dataSize();
}

const VertexDeclaration& VertexStorage::declaration() const
{
	return _private->decl;
}

size_t VertexStorage::capacity() const
{
	return _private->capacity;
}

void VertexStorage::increaseSize(size_t sz)
{
	resize(_private->capacity + sz);
}

void VertexStorage::resize(size_t sz)
{
	_private->capacity = sz;
	_private->data.resize(_private->capacity * _private->decl.dataSize());
}

/*
 * Private
 */
VertexStoragePrivate::VertexStoragePrivate(const VertexDeclaration& d, size_t cap) :
	decl(d), data(d.dataSize() * cap, 0), capacity(cap)
{

}
