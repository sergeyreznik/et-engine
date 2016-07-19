/*
 * This file is part of `et engine`
 * Copyright 2009-2016 by Sergey Reznik
 * Please, modify content only if you know what are you doing.
 *
 */

#pragma once

#include <et/rendering/renderbatch.h>

namespace et
{
	class RenderContext;
	namespace renderhelper
	{
		void init(RenderContext*);
		void release();

		RenderBatch::Pointer createFullscreenRenderBatch(Texture::Pointer);
	};
}