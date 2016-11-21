/*
 * This file is part of `et engine`
 * Copyright 2009-2016 by Sergey Reznik
 * Please, modify content only if you know what are you doing.
 *
 */

#pragma once

#include <et/rendering/vulkan/vulkan_buffer.h>
#include <et/rendering/vulkan/vulkan_program.h>
#include <et/rendering/vulkan/vulkan_texture.h>
#include <et/rendering/vulkan/vulkan_sampler.h>
#include <et/rendering/vulkan/vulkan_renderpass.h>
#include <et/rendering/vulkan/vulkan_textureset.h>
#include <et/rendering/vulkan/vulkan_pipelinestate.h>
#include <et/rendering/vulkan/vulkan_renderer.h>
#include <et/rendering/vulkan/vulkan.h>
#include <et/app/application.h>

#define VULKAN_ENABLE_VALIDATION 0

namespace et
{
class VulkanRendererPrivate : public VulkanState
{
public:
	VulkanState& vulkan() 
		{ return *this; }

	PipelineStateCache pipelineCache;
	Vector<VulkanRenderPass::Pointer> passes;
};

VulkanRenderer::VulkanRenderer(RenderContext* rc) 
	: RenderInterface(rc)
{
	ET_PIMPL_INIT(VulkanRenderer);
	Camera::renderingOriginTransform = -1.0f;
}

VulkanRenderer::~VulkanRenderer()
{
	ET_PIMPL_FINALIZE(VulkanRenderer)
}

VkResult vkEnumerateInstanceLayerPropertiesWrapper(int, uint32_t* count, VkLayerProperties* props)
{
	return vkEnumerateInstanceLayerProperties(count, props);
}

VkBool32 vulkanDebugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t obj,
	size_t location, int32_t code, const char* layerPrefix, const char* msg, void* userData)
{
	if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
	{
		log::error("%s : %s", layerPrefix, msg);
		debug::debugBreak();
	}
	else if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
	{
		log::warning("%s : %s", layerPrefix, msg);
	}
	else
	{
		log::info("%s : %s", layerPrefix, msg);
	}
	return VK_TRUE;
}

void VulkanRenderer::init(const RenderContextParameters& params)
{
	std::vector<const char*> enabledExtensions = 
	{ 
		VK_KHR_SURFACE_EXTENSION_NAME, 
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME, 
#	if (VULKAN_ENABLE_VALIDATION)
		VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
#	endif
	};

	Vector<const char*> validationLayers;

#if (VULKAN_ENABLE_VALIDATION)
	auto layerProps = enumerateVulkanObjects<VkLayerProperties>(0, vkEnumerateInstanceLayerPropertiesWrapper);
	validationLayers.reserve(4);
	for (const auto& layerProp : layerProps)
	{
		if (strstr(layerProp.layerName, "validation"))
		{
			validationLayers.emplace_back(layerProp.layerName);
		}
	}
#endif

	VkApplicationInfo appInfo = { VK_STRUCTURE_TYPE_APPLICATION_INFO };
	appInfo.pApplicationName = application().identifier().applicationName.c_str();
	appInfo.pEngineName = "et-engine";
	appInfo.apiVersion = VK_API_VERSION_1_0;
	VkInstanceCreateInfo instanceCreateInfo = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
	instanceCreateInfo.pApplicationInfo = &appInfo;
	instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(enabledExtensions.size());
	instanceCreateInfo.ppEnabledExtensionNames = enabledExtensions.data();
	instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
	instanceCreateInfo.ppEnabledLayerNames = validationLayers.data();
	VULKAN_CALL(vkCreateInstance(&instanceCreateInfo, nullptr, &_private->instance));

#if (VULKAN_ENABLE_VALIDATION)
	PFN_vkCreateDebugReportCallbackEXT createDebugCb = 
		reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(vkGetInstanceProcAddr(_private->instance, "vkCreateDebugReportCallbackEXT"));

	if (createDebugCb)
	{
		VkDebugReportCallbackCreateInfoEXT debugInfo = { VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT };
		debugInfo.flags = VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_ERROR_BIT_EXT;
		debugInfo.pfnCallback = reinterpret_cast<PFN_vkDebugReportCallbackEXT>(vulkanDebugCallback);
		VULKAN_CALL(createDebugCb(_private->instance, &debugInfo, nullptr, &_private->debugCallback));
	}
#endif

	auto physicalDevices = enumerateVulkanObjects<VkPhysicalDevice>(_private->instance, vkEnumeratePhysicalDevices);
	ET_ASSERT(!physicalDevices.empty());

	_private->physicalDevice = physicalDevices.front();

	_private->queueProperties = enumerateVulkanObjects<VkQueueFamilyProperties>(_private->physicalDevice, vkGetPhysicalDeviceQueueFamilyProperties);
	ET_ASSERT(!_private->queueProperties.empty());

	_private->graphicsQueueIndex = static_cast<uint32_t>(-1);
	for (size_t i = 0, e = _private->queueProperties.size(); i < e; ++i)
	{
		if (_private->queueProperties.at(i).queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			_private->graphicsQueueIndex = static_cast<uint32_t>(i);
			break;
		}
	}
	ET_ASSERT(_private->graphicsQueueIndex != static_cast<uint32_t>(-1));

	float queuePriorities[] = { 0.0f };
	VkDeviceQueueCreateInfo queueCreateInfo = { VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
	queueCreateInfo.queueFamilyIndex = static_cast<uint32_t>(_private->graphicsQueueIndex);
	queueCreateInfo.queueCount = 1;
	queueCreateInfo.pQueuePriorities = queuePriorities;

	Vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

	VkDeviceCreateInfo deviceCreateInfo = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
	deviceCreateInfo.queueCreateInfoCount = 1;
	deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
	deviceCreateInfo.enabledExtensionCount = 1;
	deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
	VULKAN_CALL(vkCreateDevice(_private->physicalDevice, &deviceCreateInfo, nullptr, &_private->device));

	VkCommandPoolCreateInfo cmdPoolCreateInfo = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
	cmdPoolCreateInfo.queueFamilyIndex = _private->graphicsQueueIndex;
	cmdPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	VULKAN_CALL(vkCreateCommandPool(_private->device, &cmdPoolCreateInfo, nullptr, &_private->commandPool));

	vkGetDeviceQueue(_private->device, _private->graphicsQueueIndex, 0, &_private->queue);

	VkSemaphoreCreateInfo semaphoreInfo = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
	VULKAN_CALL(vkCreateSemaphore(_private->device, &semaphoreInfo, nullptr, &_private->semaphores.renderComplete));
	VULKAN_CALL(vkCreateSemaphore(_private->device, &semaphoreInfo, nullptr, &_private->semaphores.imageAvailable));

	HWND window = reinterpret_cast<HWND>(application().context().objects[0]);
	_private->swapchain.init(_private->vulkan(), params, window);

	VkCommandBufferAllocateInfo serviceBufferInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
	serviceBufferInfo.commandPool = _private->commandPool;
	serviceBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	serviceBufferInfo.commandBufferCount = 1;
	VULKAN_CALL(vkAllocateCommandBuffers(_private->device, &serviceBufferInfo, &_private->serviceCommandBuffer));

	uint32_t defaultPoolSize = 1024;
	VkDescriptorPoolSize poolSizes[] = {
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, defaultPoolSize },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, defaultPoolSize },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, defaultPoolSize }
	};

	VkDescriptorPoolCreateInfo poolInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
	poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	poolInfo.maxSets = defaultPoolSize;
	poolInfo.poolSizeCount = sizeof(poolSizes) / sizeof(poolSizes[0]);
	poolInfo.pPoolSizes = poolSizes;
	VULKAN_CALL(vkCreateDescriptorPool(_private->device, &poolInfo, nullptr, &_private->descriptprPool));

	_private->swapchain.create(_private->vulkan());
	
	initInternalStructures();
	defaultTexture();
	defaultSampler();
}

void VulkanRenderer::shutdown()
{
	_private->pipelineCache.clear();
	shutdownInternalStructures();
	
	vkDestroyDescriptorPool(_private->device, _private->descriptprPool, nullptr);
	// TODO : clean up Vulkan
}

Buffer::Pointer VulkanRenderer::createBuffer(const Buffer::Description& desc)
{
	return VulkanBuffer::Pointer::create(_private->vulkan(), desc);
}

Texture::Pointer VulkanRenderer::createTexture(TextureDescription::Pointer desc)
{
	return VulkanTexture::Pointer::create(_private->vulkan(), desc);
}

TextureSet::Pointer VulkanRenderer::createTextureSet(const TextureSet::Description& desc)
{
	return VulkanTextureSet::Pointer::create(this, _private->vulkan(), desc);
}

Sampler::Pointer VulkanRenderer::createSampler(const Sampler::Description& desc)
{
	return VulkanSampler::Pointer::create(_private->vulkan(), desc);
}

Program::Pointer VulkanRenderer::createProgram(const std::string& source)
{
	VulkanProgram::Pointer program = VulkanProgram::Pointer::create(_private->vulkan());
	program->build(source);
	return program;
}

PipelineState::Pointer VulkanRenderer::acquirePipelineState(const RenderPass::Pointer& pass, const Material::Pointer& mat, 
	const VertexStream::Pointer& vs)
{
	auto ps = _private->pipelineCache.find(vs->vertexDeclaration(), mat->program(), mat->depthState(),
		mat->blendState(), mat->cullMode(), TextureFormat::RGBA8, vs->primitiveType());

	if (ps.invalid())
	{
		ps = VulkanPipelineState::Pointer::create(this, _private->vulkan());
		ps->setPrimitiveType(vs->primitiveType());
		ps->setInputLayout(vs->vertexDeclaration());
		ps->setDepthState(mat->depthState());
		ps->setBlendState(mat->blendState());
		ps->setCullMode(mat->cullMode());
		ps->setProgram(mat->program());
		ps->setRenderPass(pass);
		ps->build();
		
		_private->pipelineCache.addToCache(ps);
	}

	return ps;
}

RenderPass::Pointer VulkanRenderer::allocateRenderPass(const RenderPass::ConstructionInfo& info)
{
	return VulkanRenderPass::Pointer::create(this, _private->vulkan(), info);
}

void VulkanRenderer::submitRenderPass(RenderPass::Pointer pass)
{
	_private->passes.emplace_back(pass);
}

void VulkanRenderer::begin()
{
	_private->swapchain.acquireNextImage(_private->vulkan());
}

void VulkanRenderer::present()
{
	std::sort(_private->passes.begin(), _private->passes.end(), [](const VulkanRenderPass::Pointer& l, const VulkanRenderPass::Pointer& r) {
		return l->info().priority > r->info().priority;
	});

	for (VulkanRenderPass::Pointer& pass : _private->passes)
	{
		pass->submit();
	}

	_private->passes.clear();
	_private->swapchain.present(_private->vulkan());
}


}
