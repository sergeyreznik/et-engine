/*
 * This file is part of `et engine`
 * Copyright 2009-2016 by Sergey Reznik
 * Please, modify content only if you know what are you doing.
 *
 */

#include <et/app/application.h>
#include <et/rendering/rendercontext.h>
#include <et/rendering/interface/renderer.h>
#include <et/rendering/base/helpers.h>

namespace et
{
namespace renderhelper
{

namespace rh_local
{
RenderInterface::Pointer renderer;
Material::Pointer texturedMaterial;
VertexStream::Pointer vertexStream;
VertexStream::Pointer emptyStream;
}

void init(RenderInterface::Pointer& rc)
{
	ET_ASSERT(rh_local::vertexStream.invalid());

	rh_local::renderer = rc;
	
	VertexDeclaration vd(false, VertexAttributeUsage::Position, DataType::Vec3);
	VertexStorage::Pointer vs = VertexStorage::Pointer::create(vd, 4);

	VertexDataAccessor<DataType::Vec3> pos = vs->accessData<DataType::Vec3>(VertexAttributeUsage::Position, 0);
	pos[0] = vec3(-1.0f, -1.0f, 0.0f); 
	pos[1] = vec3(-1.0f, +1.0f, 0.0f); 
	pos[2] = vec3(+1.0f, -1.0f, 0.0f); 
	pos[3] = vec3(+1.0f, +1.0f, 0.0f); 

	Buffer::Pointer vb = rh_local::renderer->createVertexBuffer("rh_local::vb", vs, Buffer::Location::Device);
	rh_local::vertexStream = VertexStream::Pointer::create();
	rh_local::vertexStream->setVertexBuffer(vb, vs->declaration());
	rh_local::vertexStream->setPrimitiveType(PrimitiveType::TriangleStrips);

	rh_local::emptyStream = VertexStream::Pointer::create();
	rh_local::emptyStream->setPrimitiveType(PrimitiveType::TriangleStrips);

	rh_local::texturedMaterial = rh_local::renderer->sharedMaterialLibrary().loadDefaultMaterial(DefaultMaterial::Textured2D);
}

void release()
{
	rh_local::vertexStream.reset(nullptr);
	rh_local::texturedMaterial.reset(nullptr);
	rh_local::renderer.release();
}
RenderBatch::Pointer createQuadBatch(Material::Pointer mat, QuadType type) {
	ET_ASSERT(rh_local::vertexStream.valid());
	ET_ASSERT(rh_local::texturedMaterial.valid());

	if (mat.invalid())
		mat = rh_local::texturedMaterial;

	RenderBatch::Pointer batch = rh_local::renderer->allocateRenderBatch();
	{
		MaterialInstance::Pointer materialInstance = mat->instance();
		switch (type)
		{
		case QuadType::Fullscreen:
			batch->construct(materialInstance, rh_local::emptyStream, 0, 3);
			break;

		default:
			batch->construct(materialInstance, rh_local::vertexStream, 0, 4);
		}

	}
	return batch;
}

RenderBatch::Pointer createQuadBatch(const std::string& textureId, const Texture::Pointer& texture, const Material::Pointer& mat, QuadType type) {
	RenderBatch::Pointer batch = createQuadBatch(mat, type);
	batch->material()->setTexture(textureId, texture);
	return batch;
}
	
RenderBatch::Pointer createQuadBatch(const std::string& textureId, const Texture::Pointer& tex, const Material::Pointer& mat, const ResourceRange& rng, QuadType type)
{
	RenderBatch::Pointer rb = createQuadBatch(mat, type);
	rb->material()->setTexture(textureId, tex, rng);
	return rb;
}

}
}
