/*
 * This file is part of `et engine`
 * Copyright 2009-2016 by Sergey Reznik
 * Please, modify content only if you know what are you doing.
 *
 */

#pragma once

#include <et/rendering/vertexbuffer.h>
#include <et/rendering/indexbuffer.h>

namespace et
{
	class RenderState;
	class VertexArrayObject : public APIObject
	{
	public:
		ET_DECLARE_POINTER(VertexArrayObject)
		
	public:
		VertexArrayObject(RenderContext*, VertexBuffer::Pointer, IndexBuffer::Pointer,
			const std::string& name = emptyString);
		
		VertexArrayObject(RenderContext*, const std::string& name = emptyString);
		
		~VertexArrayObject();

		VertexBuffer::Pointer& vertexBuffer()
			{ return _vb; };

		const VertexBuffer::Pointer& vertexBuffer() const
			{ return _vb; };

		IndexBuffer::Pointer& indexBuffer()
			{ return _ib; };

		const IndexBuffer::Pointer& indexBuffer() const
			{ return _ib; };

		void setVertexBuffer(VertexBuffer::Pointer ib);
		void setIndexBuffer(IndexBuffer::Pointer ib);
		void setBuffers(VertexBuffer::Pointer vb, IndexBuffer::Pointer ib);

	private:
		void init();

	private:
		RenderContext* _rc = nullptr;
		VertexBuffer::Pointer _vb;
		IndexBuffer::Pointer _ib;
	};
}
