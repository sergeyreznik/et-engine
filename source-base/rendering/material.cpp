/*
 * This file is part of `et engine`
 * Copyright 2009-2016 by Sergey Reznik
 * Please, modify content only if you know what are you doing.
 *
 */

#include <et/app/application.h>
#include <et/rendering/interface/renderer.h>

using namespace et;

namespace
{
	const std::string kProgramSourceGL = "program-source-gl";
	const std::string kProgramSourceMetal = "program-source-metal";
	const std::string kProgramSourceVulkan = "program-source-vulkan";
	const std::string kDefines = "defines";
	const std::string kRenderPriority = "render-priority";
}

Material::Material(RenderInterface* renderer) :
	_renderer(renderer), _propertiesData(2 * sizeof(mat4), 0)
{
}

void Material::loadFromJson(const std::string& jsonString, const std::string& baseFolder)
{
	Dictionary obj;
	if (obj.loadFromJson(jsonString) == false)
	{
		log::error("Unable to load material from json.");
		return;
	}
	
	application().pushSearchPath(baseFolder);
	
	auto name = obj.stringForKey(kName)->content;
	auto cullMode = obj.stringForKey(kCullMode)->content;
	auto definesArray = obj.arrayForKey(kDefines);

	bool programIsBinary = false;
	std::string programSources[2];

	if (_renderer->api() == RenderingAPI::OpenGL)
	{
		programSources[0] = application().resolveFileName(obj.stringForKey(kProgramSourceGL)->content);
	}
	else if (_renderer->api() == RenderingAPI::Metal)
	{
		programSources[0] = application().resolveFileName(obj.stringForKey(kProgramSourceMetal)->content);
	}
	else if (_renderer->api() == RenderingAPI::Vulkan)
	{
		std::string baseName = obj.stringForKey(kProgramSourceVulkan)->content;
		programSources[0] = application().resolveFileName(replaceFileExt(baseName, ".vert.spv"));
		programSources[1] = application().resolveFileName(replaceFileExt(baseName, ".frag.spv"));
		programIsBinary = true;
	}
	else
	{
		ET_FAIL("Not implemented");
	}

	application().popSearchPaths();

	if (fileExists(programSources[0]))
	{
		if (programIsBinary)
		{
			std::ifstream fIn(programSources[0]);
			uint32_t fileSize = streamSize(fIn);
			programSources[0].resize(fileSize);
			fIn.read(&programSources[0].front(), fileSize);
		}
		else
		{
			programSources[0] = loadTextFile(programSources[0]);
		}
	}

	if (fileExists(programSources[1]))
	{
		if (programIsBinary)
		{
			std::ifstream fIn(programSources[1]);
			uint32_t fileSize = streamSize(fIn);
			programSources[1].resize(fileSize);
			fIn.read(&programSources[1].front(), fileSize);
		}
		else
		{
			programSources[1] = loadTextFile(programSources[0]);
		}
	}

	setName(name);
	setOrigin(baseFolder);
	
	_additionalPriority = static_cast<uint32_t>(obj.integerForKey(kRenderPriority, 0ll)->content);

	setBlendState(deserializeBlendState(obj.dictionaryForKey(kBlendState)));
	setDepthState(deserializeDepthState(obj.dictionaryForKey(kDepthState)));

	CullMode cm = CullMode::Disabled;
	if (stringToCullMode(cullMode, cm) == false)
	{
		log::warning("Invalid or unsupported cull mode in material: %s", cullMode.c_str());
	}
	setCullMode(cm);

	StringList defines;
	defines.reserve(definesArray->content.size());

	auto addDefine = [&defines](std::string& define)
	{
		if (!define.empty())
		{
			std::transform(define.begin(), define.end(), define.begin(), [](char c)
            {
                return (c == '=') ? ' ' : c;
            });
			defines.push_back("#define " + define + "\n");
		}
	};

	for (auto def : definesArray->content)
	{
		if (def->variantClass() == VariantClass::String)
		{
			addDefine(StringValue(def)->content);
		}
	}

	setProgram(_renderer->createProgram(programSources[0], programSources[1], defines, baseFolder));
}

void Material::setBlendState(const BlendState& state)
{
	_blend = state;
}

void Material::setDepthState(const DepthState& state)
{
	_depth = state;
}

void Material::setCullMode(CullMode cm)
{
	_cull = cm;
}

void Material::setProgram(Program::Pointer program)
{
	_program = program;
	loadProperties();
}

/*
void Material::enableInRenderState(RenderState::Pointer rs)
{
	rs->setCullMode(_cull);
	rs->setBlendState(_blend);
	rs->setDepthState(_depth);

	for (auto& i : _textures)
	{
		i.second.texture->bind(i.second.unit);
	}

	_program->bind();

	for (auto& i : _properties)
	{
		if (i.second.requireUpdate)
		{
			applyProperty(i.second, _propertiesData);
			i.second.requireUpdate = false;
		}
	}
}
*/

void Material::loadProperties()
{
	_textures.clear();
	_properties.clear();
	_propertiesData.resize(0);

    /*
     * TODO : load actual properties
     *
	uint32_t textureUnitCounter = 0;
    OpenGLProgram::Pointer openglProgram = ;
	for (const auto& u : openglProgram->shaderConstants())
	{
		String name(u.first.c_str());
		if (openglProgram->isSamplerUniformType(u.second.type))
		{
			addTexture(name, u.second.location, textureUnitCounter);
			openglProgram->setUniform(u.second.location, u.second.type, textureUnitCounter);
			++textureUnitCounter;
		}
		else if (!openglProgram->isBuiltInUniformName(u.first))
		{
			addDataProperty(name, openglProgram->uniformTypeToDataType(u.second.type), u.second.location);
		}
	}
    // */
}

void Material::addTexture(const String& name, int32_t location, uint32_t unit)
{
	_textures.emplace(name, TextureProperty(location, unit));
	_shouldUpdateSnapshot = true;
}

void Material::addDataProperty(const String& name, DataType type, int32_t location)
{
	uint32_t requiredSpace = dataTypeSize(type);
	uint32_t currentSize = _propertiesData.lastElementIndex();
	if (currentSize + requiredSpace > _propertiesData.size())
	{
		_propertiesData.resize(std::max(2 * _propertiesData.size(), currentSize + requiredSpace));
	}
	_properties.emplace(name, DataProperty(type, location, currentSize, requiredSpace));
	_propertiesData.applyOffset(requiredSpace);
	_shouldUpdateSnapshot = true;
}

void Material::updateDataProperty(DataProperty& prop, const void* src)
{
	auto dst = _propertiesData.element_ptr(prop.offset);
	if (memcmp(src, dst, prop.length) != 0)
	{
		_shouldUpdateSnapshot = true;
		memcpy(dst, src, prop.length);
		prop.requireUpdate = true;
	}
}

#define ET_SET_PROPERTY_IMPL(TYPE) 	{ auto i = _properties.find(name); \
									if ((i != _properties.end()) && (i->second.type == DataType::TYPE)) { \
										updateDataProperty(i->second, &value); \
									}}

void Material::setProperty(const String& name, const float value) ET_SET_PROPERTY_IMPL(Float)
void Material::setProperty(const String& name, const vec2& value) ET_SET_PROPERTY_IMPL(Vec2)
void Material::setProperty(const String& name, const vec3& value) ET_SET_PROPERTY_IMPL(Vec3)
void Material::setProperty(const String& name, const vec4& value) ET_SET_PROPERTY_IMPL(Vec4)
void Material::setProperty(const String& name, const int value) ET_SET_PROPERTY_IMPL(Int)
void Material::setProperty(const String& name, const vec2i& value) ET_SET_PROPERTY_IMPL(IntVec2)
void Material::setProperty(const String& name, const vec3i& value) ET_SET_PROPERTY_IMPL(IntVec3)
void Material::setProperty(const String& name, const vec4i& value) ET_SET_PROPERTY_IMPL(IntVec4)
void Material::setProperty(const String& name, const mat3& value) ET_SET_PROPERTY_IMPL(Mat3)
void Material::setProperty(const String& name, const mat4& value) ET_SET_PROPERTY_IMPL(Mat4)

void Material::setTexutre(const String& name, const Texture::Pointer& tex)
{
	auto i = _textures.find(name);
    if (i == _textures.end())
    {
        _textures[name].texture = tex;
    }
	else if (i->second.texture != tex)
	{
		i->second.texture = tex;
		_shouldUpdateSnapshot = true;
	}
}

Texture::Pointer Material::texture(const String& name)
{
    auto i = _textures.find(name);
    return (i == _textures.end()) ? Texture::Pointer() : i->second.texture;
}

uint32_t Material::sortingKey() const
{
	return _depth.sortingKey() | _blend.sortingKey() << 8 | _additionalPriority << 16;
}

uint64_t Material::makeSnapshot()
{
	return 0;
	/*
	if (_shouldUpdateSnapshot == false)
	{
		return _lastShapshotIndex;
	}
	
	if (_snapshots.empty())
		_snapshots.reserve(8);

	_snapshots.emplace_back();
	
	auto& snapshot = _snapshots.back();
	snapshot.pipelineState = _pipelineState;

	snapshot.textures.reserve(_textures.size());
	for (const auto& t : _textures)
	{
		snapshot.textures.emplace_back(t.second);
	}
	
	snapshot.properties.reserve(_properties.size());
	for (const auto& p : _properties)
	{
		snapshot.properties.emplace_back(p.second);
	}
    
    snapshot.propertiesData = _propertiesData;
	
	_shouldUpdateSnapshot = false;
	_lastShapshotIndex = _snapshots.size() - 1;
	return _lastShapshotIndex;
	*/
}

/*
 * TODO
 *
void Material::enableSnapshotInRenderState(RenderState::Pointer rs, uint64_t index)
{
	ET_ASSERT(index < _snapshots.size());
	
	Snapshot& snapshot = _snapshots.at(static_cast<size_t>(index));

	rs->setCullMode(snapshot.pipelineState.cull);
	rs->setBlendState(snapshot.pipelineState.blend);
	rs->setDepthState(snapshot.pipelineState.depth);

	for (auto& i : snapshot.textures)
	{
		i.texture->bind(i.unit);
	}

	_program->bind();
	for (const auto& i : snapshot.properties)
	{
		applyProperty(i, snapshot.propertiesData);
	}
}
*/

void Material::applyProperty(const DataProperty& prop, const BinaryDataStorage& data)
{
	/*
	auto format = dataTypeDataFormat(prop.type);
	auto ptr = data.element_ptr(prop.offset);
    OpenGLProgram* programPtr = static_cast<OpenGLProgram*>(_program.ptr());
	if (format == DataFormat::Int)
	{
		auto& fn = _setIntFunctions[uint32_t(prop.type)];
		(programPtr->*fn)(prop.locationInProgram, reinterpret_cast<const int*>(ptr), 1);
	}
	else
	{
		auto& fn = _setFloatFunctions[uint32_t(prop.type)];
		(programPtr->*fn)(prop.locationInProgram, reinterpret_cast<const float*>(ptr), 1);
	}
	*/ 
}

void Material::clearSnapshots()
{
	_snapshots.clear();
	_lastShapshotIndex = uint64_t(-1);
	_shouldUpdateSnapshot = true;
}
