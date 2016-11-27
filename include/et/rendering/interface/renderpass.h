/*
 * This file is part of `et engine`
 * Copyright 2009-2016 by Sergey Reznik
 * Please, modify content only if you know what are you doing.
 *
 */

#pragma once

#include <et/camera/camera.h>
#include <et/rendering/rendering.h>
#include <et/rendering/constantbuffer.h>
#include <et/rendering/base/renderbatch.h>

namespace et
{

struct RenderTarget
{
	Texture::Pointer texture;
	FramebufferOperation loadOperation = FramebufferOperation::Clear;
	FramebufferOperation storeOperation = FramebufferOperation::Store;
	vec4 clearValue = vec4(1.0f);
	bool useDefaultRenderTarget = true;
	bool enabled = false;
};

class RenderInterface;
class RenderPass : public Shared
{
public:
	ET_DECLARE_POINTER(RenderPass);

	struct ConstructionInfo
	{
		RenderTarget color[MaxRenderTargets];
		RenderTarget depth;
		Camera::Pointer camera;
		Camera::Pointer light;
		RenderPassClass renderPassClass = RenderPassClass::Forward;
		uint32_t priority = 0;
	};

	struct Variables
	{
		mat4 viewProjection;
		mat4 projection;
		mat4 view;
		vec4 cameraPosition;
		vec4 cameraDirection;
		vec4 cameraUp;
		vec4 lightPosition;
	};

public:
	RenderPass(RenderInterface* renderer, const ConstructionInfo& info);
	virtual ~RenderPass();

	virtual void begin() = 0;
	virtual void pushRenderBatch(const RenderBatch::Pointer&) = 0;
	virtual void end() = 0;

	const ConstructionInfo& info() const;
	ConstantBuffer& dynamicConstantBuffer();

	void setCamera(const Camera::Pointer& cam);
	void setLightCamera(const Camera::Pointer& cam);

	Camera::Pointer& camera();
	const Camera::Pointer& camera() const;

private:
	ConstructionInfo _info;
	ConstantBuffer _dynamicConstantBuffer;
};

inline RenderPass::RenderPass(RenderInterface* renderer, const ConstructionInfo& info) :
	_info(info)
{
	_dynamicConstantBuffer.init(renderer);
}

inline RenderPass::~RenderPass()
{
	_dynamicConstantBuffer.shutdown();
}

inline const RenderPass::ConstructionInfo& RenderPass::info() const
{
	return _info;
}

inline ConstantBuffer& RenderPass::dynamicConstantBuffer()
{
	return _dynamicConstantBuffer;
}

inline void RenderPass::setCamera(const Camera::Pointer& cam)
{
	_info.camera = cam;
}

inline void RenderPass::setLightCamera(const Camera::Pointer& cam)
{
	_info.light = cam;
}

inline Camera::Pointer& RenderPass::camera() 
{
	return _info.camera; 
}

inline const Camera::Pointer& RenderPass::camera() const
{
	return _info.camera; 
}

}
