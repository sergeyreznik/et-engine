/*
 * This file is part of `et engine`
 * Copyright 2009-2016 by Sergey Reznik
 * Please, modify content only if you know what are you doing.
 *
 */

#include <et/core/tools.h>
#include <et/opengl/opengl.h>
#include <et/opengl/openglcaps.h>
#include <et/rendering/renderingcaps.h>

using namespace et;

bool OpenGLCapabilities::hasExtension(const std::string& e)
{
	return _extensions.count(lowercase(e)) > 0;
}

void OpenGLCapabilities::checkCaps()
{
	const char* glv = reinterpret_cast<const char*>(glGetString(GL_VERSION));
	_versionShortString = emptyString;
	_versionString = ((glv == nullptr) || strlen(glv) == 0) ? "<Unknown OpenGL version>" : glv;
	if (glv != nullptr)
	{
		do
		{
			if ((_versionShortString.size() > 0) && (*glv == ET_SPACE)) break;
			
			if ((*glv >= '0') && (*glv <= '9'))
				_versionShortString.push_back(*glv);
		}
		while (*glv++);
	}
	while (_versionShortString.size() < 3)
		_versionShortString.push_back('0');
	checkOpenGLError("OpenGLCapabilities::checkCaps");
	
	const char* glslv = reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION));
	_glslVersionShortString = emptyString;
	_glslVersionString = ((glslv == nullptr) || (strlen(glslv) == 0)) ? "<Unknown GLSL version>" : glslv;
	if (glslv != nullptr)
	{
		do
		{
			if ((_glslVersionShortString.size() > 0) && (*glslv == ET_SPACE)) break;

			if ((*glslv >= '0') && (*glslv <= '9'))
				_glslVersionShortString.push_back(*glslv);
		}
		while (*glslv++);
	}

	while (_glslVersionShortString.size() < 3)
		_glslVersionShortString.push_back('0');

	std::string lowercaseVersion = _versionString;
	lowercase(lowercaseVersion);

	int intVersion = strToInt(_glslVersionShortString);
	
	_isOpenGLES = lowercaseVersion.find("es") != std::string::npos;
	
	_version = (intVersion < 130) || (_isOpenGLES && (intVersion < 300)) ?
		OpenGLVersion::Version_2x : OpenGLVersion::Version_3x;
	
	const char* ext = nullptr;
	
#if defined(GL_NUM_EXTENSIONS)
	if (_version == OpenGLVersion::Version_2x)
#endif
	{
		ext = reinterpret_cast<const char*>(glGetString(GL_EXTENSIONS));
		checkOpenGLError("glGetString(GL_EXTENSIONS)");
		
		if (ext != nullptr)
		{
			std::string extString(ext);
			
			size_t spacePosition = 0;
			do
			{
				spacePosition = extString.find(ET_SPACE);
				if (spacePosition == std::string::npos)
					break;
				
				std::string extension = lowercase(extString.substr(0, spacePosition));
				_extensions[extension] = 1;
				extString.erase(0, spacePosition + 1);
			}
			while ((extString.size() > 0) && (spacePosition != std::string::npos));
			
			trim(extString);
			
			if (extString.size() > 0)
			{
				lowercase(extString);
				_extensions[extString] = 1;
			}
		}
	}
#if defined(GL_NUM_EXTENSIONS)
	else
	{
		int numExtensions = 0;
		glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);
		checkOpenGLError("glGetIntegerv(GL_NUM_EXTENSIONS, ...");
		for (int i = 0; i < numExtensions; ++i)
		{
			ext = reinterpret_cast<const char*>(glGetStringi(GL_EXTENSIONS, i));
			_extensions[lowercase(ext)] = 1;
		}
	}
#endif
	
	int maxUnits = 0;
	glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, &maxUnits);
	checkOpenGLError("glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, ...");
	if (maxUnits > 0)
		setFlag(OpenGLFeature_VertexTextureFetch);

#if defined(GL_ARB_draw_elements_base_vertex)
	setFlag(OpenGLFeature_DrawElementsBaseVertex);
#endif
	
#if (ET_PLATFORM_MAC)
	setFlag(OpenGLFeature_MipMapGeneration);
	setFlag(OpenGLFeature_VertexArrayObjects);
#else
	
#if (ET_PLATFROM_WIN)
	if (glGenerateMipmap != nullptr)
		setFlag(OpenGLFeature_MipMapGeneration);
#endif
	
	bool supportVertexArrays = (glGenVertexArrays != nullptr) && (glDeleteVertexArrays != nullptr)
		&& (glBindVertexArray != nullptr) && (glIsVertexArray != nullptr);
	
	if (supportVertexArrays)
	{
		uint32_t testArray = 0;
		glGenVertexArrays(1, &testArray);
		if ((glGetError() == GL_NO_ERROR) && (testArray != 0))
		{
			glDeleteVertexArrays(1, &testArray);
			setFlag(OpenGLFeature_VertexArrayObjects);
		}
	}
#endif
	
	log::info("[OpenGLCapabilities] Version: %s (%s), GLSL version: %s (%s)", _versionString.c_str(),
		_versionShortString.c_str(), _glslVersionString.c_str(), _glslVersionShortString.c_str());
	
	RenderingCapabilities::instance().checkCaps();
};

void RenderingCapabilities::checkCaps()
{
#if (GL_IMG_texture_compression_pvrtc)
	_textureFormatSupport[TextureFormat::PVR_2bpp_RGB] = 1;
	_textureFormatSupport[TextureFormat::PVR_2bpp_RGBA] = 1;
	_textureFormatSupport[TextureFormat::PVR_4bpp_RGB] = 1;
	_textureFormatSupport[TextureFormat::PVR_4bpp_RGBA] = 1;
#endif
	
#if (GL_EXT_pvrtc_sRGB)
	_textureFormatSupport[TextureFormat::PVR_2bpp_sRGB] = 1;
	_textureFormatSupport[TextureFormat::PVR_2bpp_sRGBA] = 1;
	_textureFormatSupport[TextureFormat::PVR_4bpp_sRGB] = 1;
	_textureFormatSupport[TextureFormat::PVR_4bpp_sRGBA] = 1;
#endif
	
#if (GL_EXT_texture_compression_s3tc)
	_textureFormatSupport[TextureFormat::DXT1_RGB] = 1;
	_textureFormatSupport[TextureFormat::DXT1_RGBA] = 1;
	_textureFormatSupport[TextureFormat::DXT3] = 1;
	_textureFormatSupport[TextureFormat::DXT5] = 1;
#endif
	
#if (GL_ARB_texture_compression_rgtc)
	_textureFormatSupport[TextureFormat::RGTC2] = 1;
#endif
	
	int maxSize = 0;
	glGetIntegerv(GL_MAX_CUBE_MAP_TEXTURE_SIZE, &maxSize);
	checkOpenGLError("glGetIntegerv(GL_MAX_CUBE_MAP_TEXTURE_SIZE, ...");
	_maxCubemapTextureSize = static_cast<uint32_t>(maxSize);
	
	maxSize = 0;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxSize);
	checkOpenGLError("glGetIntegerv(GL_MAX_TEXTURE_SIZE, ...");
	_maxTextureSize = static_cast<uint32_t>(maxSize);
	
	int maxSamples = 0;
	glGetIntegerv(GL_MAX_SAMPLES, &maxSamples);
	checkOpenGLError("glGetIntegerv(GL_MAX_SAMPLES, ...");
	_maxSamples = static_cast<uint32_t>(maxSamples);
	
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &_maxAnisotropyLevel);
	checkOpenGLError("glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, ...)", _maxAnisotropyLevel);
}
