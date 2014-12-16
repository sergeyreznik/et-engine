/*
 * This file is part of `et engine`
 * Copyright 2009-2014 by Sergey Reznik
 * Please, do not modify content without approval.
 *
 */

#include <et/core/tools.h>
#include <et/rendering/rendercontext.h>
#include <et/opengl/openglcaps.h>

using namespace et;

#if !defined(ET_CONSOLE_APPLICATION)
	static const int defaultBindingUnit = 7;
#endif

TextureData::TextureData(RenderContext* rc, const TextureDescription::Pointer& desc, const std::string& id,
	bool deferred) : LoadableObject(id, desc->origin()), _glID(0), _desc(desc), _own(true)
{
#if defined(ET_CONSOLE_APPLICATION)

	ET_FAIL("Attempt to create Texture in console application.")

#else

#	if (ET_OPENGLES)
	if (!(isPowerOfTwo(desc->size.x) && isPowerOfTwo(desc->size.y)))
		_wrap = vector3<TextureWrap>(TextureWrap_ClampToEdge);
#	endif
	
	if (deferred)
	{
		buildProperies();
	}
	else
	{
		generateTexture(rc);
		build(rc);
	}
	
#endif
}

TextureData::TextureData(RenderContext*, uint32_t texture, const vec2i& size, const std::string& name) :
	LoadableObject(name), _glID(texture), _own(false), _desc(sharedObjectFactory().createObject<TextureDescription>())
{
#if !defined(ET_CONSOLE_APPLICATION)
	if (glIsTexture(texture))
	{
		_desc->setOrigin(name);
		_desc->target = GL_TEXTURE_2D;
		_desc->size = size;
		_desc->mipMapCount = 1;
		
		buildProperies();
	}
	else
	{
		_glID = 0;
	}
#endif
}

TextureData::~TextureData()
{
#if !defined(ET_CONSOLE_APPLICATION)
	if (_own && (_glID != 0) && glIsTexture(_glID))
		glDeleteTextures(1, &_glID);
#endif
}

void TextureData::setWrap(RenderContext* rc, TextureWrap s, TextureWrap t, TextureWrap r)
{
#if !defined(ET_CONSOLE_APPLICATION)
	_wrap = vector3<TextureWrap>(s, t, r);

	rc->renderState().bindTexture(defaultBindingUnit, _glID, _desc->target);
	
	glTexParameteri(_desc->target, GL_TEXTURE_WRAP_S, textureWrapValue(_wrap.x));
	checkOpenGLError("glTexParameteri<WRAP_S> - %s", name().c_str());
	
	glTexParameteri(_desc->target, GL_TEXTURE_WRAP_T, textureWrapValue(_wrap.y));
	checkOpenGLError("glTexParameteri<WRAP_T> - %s", name().c_str());
	
#	if defined(GL_TEXTURE_WRAP_R)
#	if (ET_OPENGLES)
	if (openGLCapabilites().versionShortString() > "200")
#	endif
	{
		glTexParameteri(_desc->target, GL_TEXTURE_WRAP_R, textureWrapValue(_wrap.z));
		checkOpenGLError("glTexParameteri<WRAP_R> - %s", name().c_str());
	}
#	endif
#endif
}

void TextureData::setFiltration(RenderContext* rc, TextureFiltration minFiltration,
	TextureFiltration magFiltration)
{
#if !defined(ET_CONSOLE_APPLICATION)
	rc->renderState().bindTexture(defaultBindingUnit, _glID, _desc->target);

	_filtration = vector2<TextureFiltration>(minFiltration, magFiltration);

	if ((_desc->mipMapCount < 2) && (minFiltration > TextureFiltration_Linear))
		_filtration.x = TextureFiltration_Linear;

	if (magFiltration > TextureFiltration_Linear)
		_filtration.y = TextureFiltration_Linear;

	glTexParameteri(_desc->target, GL_TEXTURE_MIN_FILTER, textureFiltrationValue(_filtration.x)); 
	checkOpenGLError("glTexParameteri<GL_TEXTURE_MIN_FILTER> - %s", name().c_str());
	
	glTexParameteri(_desc->target, GL_TEXTURE_MAG_FILTER, textureFiltrationValue(_filtration.y)); 
	checkOpenGLError("glTexParameteri<GL_TEXTURE_MAG_FILTER> - %s", name().c_str());
#endif
}

#if defined(GL_TEXTURE_COMPARE_MODE) && defined(GL_TEXTURE_COMPARE_FUNC)
void TextureData::compareRefToTexture(RenderContext* rc, bool enable, int32_t compareFunc)
{
#if !defined(ET_CONSOLE_APPLICATION)
	rc->renderState().bindTexture(defaultBindingUnit, _glID, _desc->target);
	if (enable)
	{
		glTexParameteri(_desc->target, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
		checkOpenGLError("glTexParameteri(_desc->target, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE)");
		
		glTexParameteri(_desc->target, GL_TEXTURE_COMPARE_FUNC, compareFunc);
		checkOpenGLError("glTexParameteri(_desc->target, GL_TEXTURE_COMPARE_FUNC, compareFunc)");
	}
	else
	{
		glTexParameteri(_desc->target, GL_TEXTURE_COMPARE_MODE, GL_NONE);
		checkOpenGLError("glTexParameteri(_target, GL_TEXTURE_COMPARE_MODE, GL_NONE) - %s", name().c_str());
	}
#endif
}
#else
void TextureData::compareRefToTexture(RenderContext*, bool, int32_t)
	{ ET_FAIL("GL_TEXTURE_COMPARE_MODE and GL_TEXTURE_COMPARE_FUNC are not defined."); }
#endif

void TextureData::generateTexture(RenderContext*)
{
#if !defined(ET_CONSOLE_APPLICATION)
	if (!glIsTexture(_glID))
		glGenTextures(1, &_glID);

	checkOpenGLError("TextureData::generateTexture - %s", name().c_str());
#endif
}

void TextureData::buildData(const char* aDataPtr, size_t aDataSize)
{
#if !defined(ET_CONSOLE_APPLICATION)
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	checkOpenGLError("glPixelStorei");

#if defined(GL_TEXTURE_1D)
	if (_desc->target == GL_TEXTURE_1D)
	{
		if (_desc->compressed && (aDataSize > 0))
		{
			etCompressedTexImage1D(_desc->target, 0, static_cast<uint32_t>(_desc->internalformat),
				_desc->size.x, 0, static_cast<GLsizei>(aDataSize), aDataPtr);
		}
		else
		{
			etTexImage1D(_desc->target, 0, _desc->internalformat, _desc->size.x, 0,
				_desc->format, _desc->type, aDataPtr);
		}
	}
	else
#endif
	if (_desc->target == GL_TEXTURE_2D)
	{
		for (size_t level = 0; level < _desc->mipMapCount; ++level)
		{
			vec2i t_mipSize = _desc->sizeForMipLevel(level);
			GLsizei t_dataSize = static_cast<GLsizei>(_desc->dataSizeForMipLevel(level));
			size_t t_offset = _desc->dataOffsetForMipLevel(level, 0);
			
			const char* ptr = (aDataPtr && (t_offset < aDataSize)) ? &aDataPtr[t_offset] : 0;
			if (_desc->compressed && ptr)
			{
				etCompressedTexImage2D(_desc->target, static_cast<int>(level),
					static_cast<uint32_t>(_desc->internalformat), t_mipSize.x, t_mipSize.y, 0, t_dataSize, ptr);
			}
			else
			{
				etTexImage2D(_desc->target, static_cast<int>(level), _desc->internalformat,
					t_mipSize.x, t_mipSize.y, 0, _desc->format, _desc->type, ptr);
			}
		}
	}
	else if (_desc->target == GL_TEXTURE_CUBE_MAP)
	{
		uint32_t target = GL_TEXTURE_CUBE_MAP_POSITIVE_X;
		
		for (size_t layer = 0; layer < _desc->layersCount; ++layer, ++target)
		{
			for (size_t level = 0; level < _desc->mipMapCount; ++level)
			{
				vec2i t_mipSize = _desc->sizeForMipLevel(level);
				size_t t_offset = _desc->dataOffsetForMipLevel(level, layer);
				
				const char* ptr = (aDataPtr && (t_offset < aDataSize)) ? &aDataPtr[t_offset] : nullptr;
				if (_desc->compressed && (ptr != nullptr))
				{
					etCompressedTexImage2D(target, static_cast<int>(level), static_cast<uint32_t>(_desc->internalformat),
						t_mipSize.x, t_mipSize.y, 0, static_cast<GLsizei>(_desc->dataSizeForMipLevel(level)), ptr);
				}
				else
				{
					etTexImage2D(target, static_cast<int>(level), _desc->internalformat, t_mipSize.x, t_mipSize.y,
						0, _desc->format, _desc->type, ptr);
				}
			}
		}
	}
	else
	{
		log::error("Unsupported texture target specified: glTexTargetToString(_target)");
	}
#endif
}

void TextureData::buildProperies()
{
	setOrigin(_desc->origin());
	
	_texel = vec2(1.0f / static_cast<float>(_desc->size.x), 1.0f / static_cast<float>(_desc->size.y) );
	_filtration.x = (_desc->mipMapCount > 1) ? TextureFiltration_LinearMipMapLinear : TextureFiltration_Linear;
	_filtration.y = TextureFiltration_Linear;
}

void TextureData::build(RenderContext* rc)
{
#if !defined(ET_CONSOLE_APPLICATION)
	ET_ASSERT(_desc.valid());
	
	if (_desc->size.square() <= 0)
	{
		log::warning("Texture %s has invalid dimensions.", _desc->origin().c_str());
		return;
	}
	
	if (_desc->internalformat == 0)
	{
		log::warning("Texture %s has invalid format.", _desc->origin().c_str());
		return;
	}

	if (_desc->type == 0)
	{
		log::warning("Texture %s has invalid type.", _desc->origin().c_str());
		return;
	}

	buildProperies();

	rc->renderState().bindTexture(defaultBindingUnit, _glID, _desc->target, true);

	setFiltration(rc, _filtration.x, _filtration.y);
	setWrap(rc, _wrap.x, _wrap.y, _wrap.z);

	if (_desc->mipMapCount > 1)
		setMaxLod(rc, _desc->mipMapCount - 1);

    buildData(_desc->data.constBinaryData(), _desc->data.dataSize());
	checkOpenGLError("buildData");
	
	_desc->data.resize(0);
	ET_ASSERT(glIsTexture(_glID));
#endif
}

vec2 TextureData::getTexCoord(const vec2& vec, TextureOrigin origin) const
{
	float ax = vec.x * _texel.x;
	float ay = vec.y * _texel.y;
	return vec2(ax, (origin == TextureOrigin_TopLeft) ? 1.0f - ay : ay);
}

void TextureData::updateData(RenderContext* rc, TextureDescription::Pointer desc)
{
#if !defined(ET_CONSOLE_APPLICATION)
	_desc = desc;
	generateTexture(rc);
	build(rc);
#endif
}

void TextureData::updateDataDirectly(RenderContext* rc, const vec2i& size, const char* data, size_t dataSize)
{
#if !defined(ET_CONSOLE_APPLICATION)
	if (_glID == 0)
		generateTexture(rc);

    _desc->size = size;
    rc->renderState().bindTexture(defaultBindingUnit, _glID, _desc->target);
	buildData(data, dataSize);
#endif
}

void TextureData::updatePartialDataDirectly(RenderContext* rc, const vec2i& offset,
	const vec2i& aSize, const char* data, size_t)
{
#if !defined(ET_CONSOLE_APPLICATION)
	ET_ASSERT((_desc->target == GL_TEXTURE_2D) && !_desc->compressed);
	ET_ASSERT((offset.x >= 0) && (offset.y >= 0));
	ET_ASSERT((offset.x + aSize.x) < _desc->size.x);
	ET_ASSERT((offset.y + aSize.y) < _desc->size.y);
	
	if (_glID == 0)
		generateTexture(rc);
	
    rc->renderState().bindTexture(defaultBindingUnit, _glID, _desc->target);
	
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	checkOpenGLError("glPixelStorei");
	
	glTexSubImage2D(_desc->target, 0, offset.x, offset.y, aSize.x, aSize.y, _desc->format, _desc->type, data);
	checkOpenGLError("glTexSubImage2D(%s, 0, %d, %d, %d, %d, %s, %s, 0x%016x)",
		glTexTargetToString(_desc->target).c_str(), offset.x, offset.y, aSize.x, aSize.y,
		glInternalFormatToString(_desc->format).c_str(), glTypeToString(_desc->type).c_str(), data);
#endif
}

void TextureData::generateMipMaps(RenderContext* rc)
{
#if !defined(ET_CONSOLE_APPLICATION)
    rc->renderState().bindTexture(defaultBindingUnit, _glID, _desc->target);
	glGenerateMipmap(_desc->target);
	checkOpenGLError("glGenerateMipmap");
#endif
}

void TextureData::setMaxLod(RenderContext* rc, size_t value)
{
#if defined(GL_TEXTURE_MAX_LEVEL) && !defined(ET_CONSOLE_APPLICATION)
    rc->renderState().bindTexture(defaultBindingUnit, _glID, _desc->target);
	glTexParameteri(_desc->target, GL_TEXTURE_MAX_LEVEL, static_cast<GLint>(value));
	checkOpenGLError("TextureData::setMaxLod - %s", name().c_str());
#endif
}
