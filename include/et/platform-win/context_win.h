/*
 * This file is part of `et engine`
 * Copyright 2009-2016 by Sergey Reznik
 * Please, modify content only if you know what are you doing.
 *
 */

#pragma once

#include <et/app/context.h>

namespace et
{
    class ApplicationContextFactoryWin : public ApplicationContextFactory
    {
    public:
        PlatformDependentContext createContextWithOptions(ContextOptions&) override;
        void destroyContext(PlatformDependentContext) override;
    };
}