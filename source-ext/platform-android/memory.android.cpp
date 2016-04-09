/*
* This file is part of `et engine`
* Copyright 2009-2016 by Sergey Reznik
* Please, modify content only if you know what are you doing.
*
*/

#include <et/core/et.h>
#include <et/core/memory.h>

#if (ET_PLATFORM_ANDROID)

using namespace et;

size_t et::memoryUsage()
{
	return 0;
}

size_t et::availableMemory()
{
	return 0;
}

void* et::allocateVirtualMemory(size_t size)
{
	ET_FAIL("Not implemented");
	return nullptr;
}

void et::deallocateVirtualMemory(void* ptr, size_t size)
{
	ET_FAIL("Not implemented");
}

#endif // ET_PLATFORM_ANDROID
