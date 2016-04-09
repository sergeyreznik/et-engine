/*
 * This file is part of `et engine`
 * Copyright 2009-2016 by Sergey Reznik
 * Please, modify content only if you know what are you doing.
 *
 */

#pragma once

#include <et/rendering/apiobject.h>
#include <et/rendering/rendering.h>

namespace et
{
	class Camera;
	class RenderState;
	
	class Program : public APIObject
	{
	public:
		ET_DECLARE_POINTER(Program)

		struct Uniform
		{
			uint32_t type = 0;
			int32_t location = -1;
		};
		
		struct Attribute
		{
			std::string name;
			VertexAttributeUsage usage = VertexAttributeUsage::Position;
			uint32_t builtIn = 0;
			
			Attribute(const std::string& aName, VertexAttributeUsage aUsage, bool isBuiltIn) :
				name(aName), usage(aUsage), builtIn(isBuiltIn ? 1 : 0) { }
		};
		
		typedef std::unordered_map<std::string, Uniform> UniformMap;
		
	public:
		Program(RenderContext*);
		
		Program(RenderContext*, const std::string& vertexShader, const std::string& geometryShader,
			const std::string& fragmentShader, const std::string& objName, const std::string& origin,
			const StringList& defines);

		~Program();

		int getUniformLocation(const std::string& uniform) const;
		uint32_t getUniformType(const std::string& uniform) const;
		Program::Uniform getUniform(const std::string& uniform) const;

		bool validate() const;
		
		int viewMatrixUniformLocation() const 
			{ return _matViewLocation; }

		int mvpMatrixUniformLocation() const
			{ return _matViewProjectionLocation; }

		int cameraUniformLocation() const
			{ return _vCameraLocation; }

		int primaryLightUniformLocation() const
			{ return _vPrimaryLightLocation; }

		int lightProjectionMatrixLocation() const
			{ return _matLightViewProjectionLocation; }

		int transformMatrixLocation() const
			{ return _matWorldLocation; }

		void setViewMatrix(const mat4 &m, bool force = false);
		void setViewProjectionMatrix(const mat4 &m, bool force = false);
		void setCameraPosition(const vec3& p, bool force = false);
		void setPrimaryLightPosition(const vec3& p, bool force = false);
		void setLightProjectionMatrix(const mat4 &m, bool force = false);
		void setTransformMatrix(const mat4 &m, bool force = false);

		void setCameraProperties(const Camera& cam);
		
		bool isBuiltInUniformName(const std::string&);
		bool isSamplerUniformType(uint32_t);
		DataType uniformTypeToDataType(uint32_t);

		const Program::UniformMap& uniforms() const 
			{ return _uniforms; }

		void setUniform(int, uint32_t, const int32_t, bool force = false);
		void setUniform(int, uint32_t, const uint32_t, bool force = false);
		void setUniform(int, uint32_t, const int64_t, bool force = false);
		void setUniform(int, uint32_t, const uint64_t, bool force = false);
		
		void setUniform(int, uint32_t, const float, bool force = false);
		void setUniform(int, uint32_t, const vec2&, bool force = false);
		void setUniform(int, uint32_t, const vec3&, bool force = false);
		void setUniform(int, uint32_t, const vec4&, bool force = false);
		void setUniform(int, uint32_t, const vec2i&, bool force = false);
		void setUniform(int, uint32_t, const vec3i&, bool force = false);
		void setUniform(int, uint32_t, const vec4i&, bool force = false);

		void setUniform(int, uint32_t, const mat3&, bool force = false);
		void setUniform(int, uint32_t, const mat4&, bool force = false);

		void setUniform(int, uint32_t, const int* value, size_t amount);
		void setUniform(int, uint32_t, const float* value, size_t amount);
		void setUniform(int, uint32_t, const vec2* value, size_t amount);
		void setUniform(int, uint32_t, const vec3* value, size_t amount);
		void setUniform(int, uint32_t, const vec4* value, size_t amount);
		void setUniform(int, uint32_t, const mat4* value, size_t amount);

		void setUniformDirectly(int, uint32_t, const vec4&);
		void setUniformDirectly(int, uint32_t, const mat4& value);
		
		void setIntUniform(int location, const int* data, uint32_t amount);
		void setInt2Uniform(int location, const int* data, uint32_t amount);
		void setInt3Uniform(int location, const int* data, uint32_t amount);
		void setInt4Uniform(int location, const int* data, uint32_t amount);
		void setFloatUniform(int location, const float* data, uint32_t amount);
		void setFloat2Uniform(int location, const float* data, uint32_t amount);
		void setFloat3Uniform(int location, const float* data, uint32_t amount);
		void setFloat4Uniform(int location, const float* data, uint32_t amount);
		void setMatrix3Uniform(int location, const float* data, uint32_t amount);
		void setMatrix4Uniform(int location, const float* data, uint32_t amount);
		
		template <typename T>
		void setUniform(const std::string& name, const T& value, bool force = false)
		{
			auto i = findUniform(name);
			if (i != _uniforms.end())
				setUniform(i->second.location, i->second.type, value, force);
		}

		template <typename T>
		void setUniform(const std::string& name, const T* value, size_t amount)
		{
			auto i = findUniform(name);
			if (i != _uniforms.end())
				setUniform(i->second.location, i->second.type, value, amount);
		}
		
		template <typename T>
		void setUniform(const Program::Uniform& u, const T& value, bool force = false)
			{ setUniform(u.location, u.type, value, force); }

		template <typename T>
		void setUniformDirectly(const Program::Uniform& u, const T& value)
			{ setUniformDirectly(u.location, u.type, value); }
		
		template <typename T>
		void setUniform(const Program::Uniform& u, const T* value, size_t amount)
			{ setUniform(u.location, u.type, value, amount); }
		
		void buildProgram(const std::string& vertex_source, const std::string& geom_source,
			const std::string& frag_source);
		
		const StringList& defines() const
			{ return _defines; }
		
	private:
		Program::UniformMap::const_iterator findUniform(const std::string& name) const;
		
		int link(bool);
		void printShaderLog(uint32_t, size_t, const char*);
		void printShaderSource(uint32_t, size_t, const char*);
		void initBuiltInUniforms();

	private:
		RenderContext* _rc = nullptr;
		
		Program::UniformMap _uniforms;
		std::vector<Attribute> _attributes;
		std::map<int, float> _floatCache;
		std::map<int, vec2> _vec2Cache;
		std::map<int, vec3> _vec3Cache;
		std::map<int, vec4> _vec4Cache;
		std::map<int, vec2i> _vec2iCache;
		std::map<int, vec3i> _vec3iCache;
		std::map<int, vec4i> _vec4iCache;
		std::map<int, mat3> _mat3Cache;
		std::map<int, mat4> _mat4Cache;

		int _matViewLocation = -1;
		int _matViewProjectionLocation = -1;
		int _vCameraLocation = -1;
		int _vPrimaryLightLocation = -1;
		int _matLightViewProjectionLocation = -1;
		int _matWorldLocation = -1;

		std::unordered_map<std::string, int*> _builtInUniforms;
		StringList _defines;
	};
}
