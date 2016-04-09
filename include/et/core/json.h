/*
 * This file is part of `et engine`
 * Copyright 2009-2016 by Sergey Reznik
 * Please, modify content only if you know what are you doing.
 *
 */

#pragma once

#include <et/core/et.h>

namespace et
{
	namespace json
	{
		enum SerializationFlags
		{
			SerializationFlag_ReadableFormat = 0x01,
			SerializationFlag_ConvertUnicode = 0x02
		};
		
		std::string serialize(const et::Dictionary&, size_t = 0);
		std::string serialize(const et::ArrayValue&, size_t = 0);
		
		et::VariantBase::Pointer deserialize(const char*, size_t, et::VariantClass&, bool printErrors = true);
		et::VariantBase::Pointer deserialize(const char*, et::VariantClass&, bool printErrors = true);
		et::VariantBase::Pointer deserialize(const std::string&, et::VariantClass&, bool printErrors = true);
	}
}
