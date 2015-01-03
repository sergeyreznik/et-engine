/*
 * This file is part of `et engine`
 * Copyright 2009-2014 by Sergey Reznik
 * Please, do not modify content without approval.
 *
 */

#pragma once

#include <et/apiobjects/texturedescription.h>

namespace et
{
	class RenderContext;

	class TextureData : public LoadableObject
	{
	public:
		TextureData(RenderContext*, const TextureDescription::Pointer&, const std::string&, bool deferred);
		TextureData(RenderContext*, uint32_t texture, const vec2i& size, const std::string& name);
		~TextureData();

		void setWrap(RenderContext*, TextureWrap s, TextureWrap t,
			TextureWrap r = TextureWrap::ClampToEdge);

		void setFiltration(RenderContext*, TextureFiltration minFiltration,
			TextureFiltration magFiltration);

		void setMaxLod(RenderContext*, size_t value);
		void setAnisotropyLevel(RenderContext*, float);

		void compareRefToTexture(RenderContext*, bool enable, int32_t compareFunc);
		void generateMipMaps(RenderContext* rc);

		vec2 getTexCoord(const vec2& ivec, TextureOrigin origin = TextureOrigin::TopLeft) const;

		void updateData(RenderContext*, TextureDescription::Pointer desc);
		void updateDataDirectly(RenderContext*, const vec2i& size, const char* data, size_t dataSize);

		void updatePartialDataDirectly(RenderContext*, const vec2i& offset, const vec2i& size,
			const char* data, size_t dataSize);

		uint32_t glID() const
			{ return _glID; }

		TextureFormat internalFormat() const
			{ return _desc->internalformat; }

		TextureFormat format() const
			{ return _desc->format; }

		DataType dataType() const
			{ return _desc->type; }

		TextureTarget target() const
			{ return _desc->target; }

		int width() const
			{ return _desc->size.x; }

		int height() const
			{ return _desc->size.y; }

		const vec2i& size() const
			{ return _desc->size; }

		vec2 sizeFloat() const
			{ return vec2(static_cast<float>(_desc->size.x), static_cast<float>(_desc->size.y)); }

		const vec2& texel() const
			{ return _texel; }
		
		const TextureDescription::Pointer description() const
			{ return _desc; }

	private:
		void generateTexture(RenderContext* rc);
		void buildProperies();
		void build(RenderContext* rc);
        void buildData(const char* ptr, size_t dataSize);

	private:
		uint32_t _glID = 0;
		TextureDescription::Pointer _desc;
		vector3<TextureWrap> _wrap;
		vector2<TextureFiltration> _filtration;
		vec2 _texel;
		bool _own = false;
	};

	typedef IntrusivePtr<TextureData> Texture;
}
