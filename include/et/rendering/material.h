/*
 * This file is part of `et engine`
 * Copyright 2009-2016 by Sergey Reznik
 * Please, modify content only if you know what are you doing.
 *
 */

#pragma once

#include <et/core/datastorage.h>
#include <et/rendering/program.h>
#include <et/rendering/texture.h>

namespace et
{
	class RenderState;
	class MaterialFactory;
	class Material : public LoadableObject
	{
	public:
		ET_DECLARE_POINTER(Material)
		
	public:
		Material(MaterialFactory*);
		
		void loadFromJson(const std::string&, const std::string& baseFolder);
		
		void enableInRenderState(RenderState&);
		void enableSnapshotInRenderState(RenderState&, uint64_t);
		
		et::Program::Pointer& program()
			{ return _program; }
		
		const et::Program::Pointer& program() const
			{ return _program; }
		
		const DepthState& depthState() const
			{ return _depth; }
		
		const BlendState& blendState() const
			{ return _blend; }
		
		CullMode cullMode() const
			{ return _cullMode; }
		
		uint64_t makeSnapshot();
		void clearSnapshots();
		
		void setProperty(const String& name, const float value);
		void setProperty(const String& name, const vec2& value);
		void setProperty(const String& name, const vec3& value);
		void setProperty(const String& name, const vec4& value);
		void setProperty(const String& name, const int value);
		void setProperty(const String& name, const vec2i& value);
		void setProperty(const String& name, const vec3i& value);
		void setProperty(const String& name, const vec4i& value);
		void setProperty(const String& name, const mat3& value);
		void setProperty(const String& name, const mat4& value);
		
		void setTexutre(const String& name, const Texture::Pointer&);
		
		uint32_t sortingKey() const;

	private:
		struct DataProperty
		{
			DataType type = DataType::max;
			int32_t locationInProgram = -1;
			uint32_t offset = 0;
			uint32_t length = 0;
			bool requireUpdate = true;
			
			DataProperty(const DataProperty&) = default;
			DataProperty(DataType dt, int32_t loc, uint32_t o, uint32_t len) :
				type(dt), locationInProgram(loc), offset(o), length(len) { }
		};
		
		struct TextureProperty
		{
			int32_t locationInProgram = -1;
			uint32_t unit = 0;
			Texture::Pointer texture;
			
			TextureProperty(const TextureProperty&) = default;
			TextureProperty(int32_t loc, uint32_t u) :
				locationInProgram(loc), unit(u) { }
		};
		
		struct Snapshot
		{
			Vector<DataProperty> properties;
			Vector<TextureProperty> textures;
			DepthState depth;
			BlendState blend;
			CullMode cullMode;
		};
		
		using ProgramSetIntFunction = void (Program::*)(int, const int*, uint32_t);
		using ProgramSetFloatFunction = void (Program::*)(int, const float*, uint32_t);
		
		void loadProperties();
		void addDataProperty(const String&, DataType, int32_t location);
		void addTexture(const String&, int32_t location, uint32_t unit);
		void updateDataProperty(DataProperty&, const void*);
		
		void applyProperty(const DataProperty&);
		
	public:
		MaterialFactory* _factory = nullptr;
		Program::Pointer _program;
		
		Vector<Snapshot> _snapshots;
		UnorderedMap<String, DataProperty> _properties;
		UnorderedMap<String, TextureProperty> _textures;
		DepthState _depth;
		BlendState _blend;
		CullMode _cullMode = CullMode::Disabled;
		
		BinaryDataStorage _propertiesData;
		ProgramSetIntFunction _setIntFunctions[DataType_max];
		ProgramSetFloatFunction _setFloatFunctions[DataType_max];
		uint32_t _additionalPriority = 0;
		uint64_t _lastShapshotIndex = uint64_t(-1);
		bool _shouldUpdateSnapshot = true;
	};
	
	class MaterialProvider
	{
	public:
		virtual Material::Pointer materialWithName(const std::string&) = 0;
	};
}
