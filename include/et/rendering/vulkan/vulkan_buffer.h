/*
 * This file is part of `et engine`
 * Copyright 2009-2016 by Sergey Reznik
 * Please, modify content only if you know what are you doing.
 *
 */

#pragma once

#include <et/rendering/interface/buffer.h>

namespace et
{
class VulkanState;
class VulkanNativeBuffer;
class VulkanBufferPrivate;
class VulkanBuffer : public Buffer
{
public:
	ET_DECLARE_POINTER(VulkanBuffer);
		
public:
	VulkanBuffer(VulkanState& vulkan, const Description&);
	~VulkanBuffer();

	uint8_t* map(uint64_t offset, uint64_t size) override;
	void modifyRange(uint64_t begin, uint64_t length) override;
	void unmap() override;
	bool mapped() const override;
	
	void updateData(uint64_t offset, const BinaryDataStorage&) override;
	void transferData(Buffer::Pointer destination, uint64_t srcOffset, uint64_t dstOffset, uint64_t size) override;

	const VulkanNativeBuffer& nativeBuffer() const;

	uint64_t size() const override;

private:
	ET_DECLARE_PIMPL(VulkanBuffer, 256);
};
}
