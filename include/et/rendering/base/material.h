/*
 * This file is part of `et engine`
 * Copyright 2009-2016 by Sergey Reznik
 * Please, modify content only if you know what are you doing.
 *
 */

#pragma once

#include <et/rendering/base/rendering.h>
#include <et/rendering/base/variableset.h>
#include <et/rendering/base/constantbuffer.h>
#include <et/rendering/interface/program.h>

namespace et
{
class RenderInterface;
class MaterialInstance;
using MaterialInstancePointer = IntrusivePtr<MaterialInstance>;
using MaterialInstanceCollection = Vector<MaterialInstancePointer>;
class Material : public LoadableObject
{
public:
	ET_DECLARE_POINTER(Material);

	struct Configuration
	{
		Program::Pointer program;
		VertexDeclaration inputLayout;
		DepthState depthState;
		BlendState blendState;
		CullMode cullMode = CullMode::Disabled;
		StringList usedFiles;
	};
	using ConfigurationMap = UnorderedMap<std::string, Configuration>;

public:
	Material(RenderInterface*);

	MaterialInstancePointer instance();
	const MaterialInstanceCollection& instances() const;
	void flushInstances();
	void releaseInstances();

	void setTexture(MaterialTexture, const Texture::Pointer&);
	void setSampler(MaterialTexture, const Sampler::Pointer&);
	void setTextureWithSampler(MaterialTexture, const Texture::Pointer&, const Sampler::Pointer&);
	void setImage(StorageBuffer, const Texture::Pointer&);

	Texture::Pointer texture(MaterialTexture);
	Sampler::Pointer sampler(MaterialTexture);
	Texture::Pointer image(StorageBuffer);

	void setVector(MaterialVariable, const vec4&);
	vec4 getVector(MaterialVariable) const;

	void setFloat(MaterialVariable, float);
	float getFloat(MaterialVariable) const;

	uint64_t sortingKey() const;

	virtual const Configuration& configuration(const std::string&) const;
	const ConfigurationMap& configurations() const { return _configurations; }

	void loadFromJson(const std::string& json, const std::string& baseFolder);

	PipelineClass pipelineClass() const { return _pipelineClass; }

private:
	friend class MaterialInstance;

	template <class T>
	T getParameter(MaterialVariable) const;

	VertexDeclaration loadInputLayout(Dictionary);
	Program::Pointer loadCode(const std::string&, const std::string& baseFolder, 
		const Dictionary& defines, const VertexDeclaration&, StringList& fileNames);
	std::string generateInputLayout(const VertexDeclaration& decl);

	void setProgram(const Program::Pointer&, const std::string&);
	void setDepthState(const DepthState&, const std::string&);
	void setBlendState(const BlendState&, const std::string&);
	void setCullMode(CullMode, const std::string&);

	void loadRenderPass(const std::string&, const Dictionary&, const std::string& baseFolder);
	void initDefaultHeader();

	virtual void invalidateImageSet();
	virtual void invalidateTextureSet();
	virtual void invalidateConstantBuffer();
	virtual bool isInstance() const { return false; }

protected: // overrided / read by instanaces
	TexturesHolder textures;
	SamplersHolder samplers;
	ImagesHolder images;
	VariablesHolder properties;

private: // permanent private data
	static std::string _shaderDefaultHeader;
	RenderInterface* _renderer = nullptr;
	MaterialInstanceCollection _instances;
	ConfigurationMap _configurations;
	PipelineClass _pipelineClass = PipelineClass::Graphics;
};

class MaterialInstance : public Material
{
public:
	ET_DECLARE_POINTER(MaterialInstance);
	using Collection = Vector<MaterialInstance::Pointer>;
	using Map = UnorderedMap<std::string, MaterialInstance::Pointer>;

public:
	Material::Pointer& base();
	const Material::Pointer& base() const;

	const TextureSet::Pointer& imageSet(const std::string&);
	const TextureSet::Pointer& textureSet(const std::string&);
	const ConstantBufferEntry::Pointer& constantBufferData(const std::string&);
	const Configuration& configuration(const std::string&) const override;

	void invalidateImageSet() override;
	void invalidateTextureSet() override;
	void invalidateConstantBuffer() override;
	
	bool isInstance() const override { return true; }

private:
	friend class Material;
	friend class ObjectFactory;
	MaterialInstance(Material::Pointer base);

	void buildImageSet(const std::string&);
	void buildTextureSet(const std::string&);
	void buildConstantBuffer(const std::string&);

private:
	template <class T>
	struct Holder
	{
		T obj;
		bool valid = false;
	};

private:
	Material::Pointer _base;
	UnorderedMap<std::string, Holder<TextureSet::Pointer>> _imageSets;
	UnorderedMap<std::string, Holder<TextureSet::Pointer>> _textureSets;
	UnorderedMap<std::string, Holder<ConstantBufferEntry::Pointer>> _constBuffers;
};

template <class T>
inline T Material::getParameter(MaterialVariable p) const
{
	uint32_t pIndex = static_cast<uint32_t>(p);
	auto i = properties.find(pIndex);
	return ((i != properties.end()) && i->second.is<T>()) ? i->second.as<T>() : T();
}

}
