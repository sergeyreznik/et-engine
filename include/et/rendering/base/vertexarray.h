/*
 * This file is part of `et engine`
 * Copyright 2009-2016 by Sergey Reznik
 * Please, modify content only if you know what are you doing.
 *
 */

#pragma once

#include <et/rendering/base/vertexdeclaration.h>
#include <et/rendering/base/vertexdatachunk.h>

namespace et
{ 
	class VertexArray : public Object
	{
	public:
		ET_DECLARE_POINTER(VertexArray);
		
		struct Description
		{
			VertexDeclaration declaration;
			BinaryDataStorage data;
		};

	public:
		VertexArray() = default;
		VertexArray(const VertexDeclaration& decl, uint32_t size);
		
		VertexDataChunk chunk(VertexAttributeUsage usage);
		
		const VertexDataChunk chunk(VertexAttributeUsage usage) const;
		
		const VertexDataChunkList& chunks() const
			{ return _chunks; }
		
		Description generateDescription() const;
		
		void resize(uint32_t size);
		void increase(uint32_t size);
		void fitToSize(uint32_t size);
		
		uint32_t size() const
			{ return _size; }

		const VertexDeclaration& decl() const
			{ return _decl; }

		VertexArray* duplicate();
		
	private:
		uint32_t _size = 0;
		VertexDeclaration _decl = VertexDeclaration(true);
		VertexDataChunkList _chunks;
	};
}
