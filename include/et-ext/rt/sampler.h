/*
 * This file is part of `et engine`
 * Copyright 2009-2016 by Sergey Reznik
 * Please, modify content only if you know what are you doing.
 *
 */

#pragma once

#include <et-ext/rt/raytraceobjects.h>

namespace et
{
namespace rt
{

struct Sampler : public Object
{
	ET_DECLARE_POINTER(Sampler);

	enum class Type : uint32_t
	{
		Random,
		Uniform,
		Stratified,
	};

	virtual bool next(vec2& sample) = 0;
};

class RandomSampler : public Sampler
{
public:
	ET_DECLARE_POINTER(RandomSampler);

public:
	RandomSampler(uint32_t maxSamples);
	bool next(vec2& sample) override;

	float4 sample(uint32_t, uint32_t);

private:
	uint32_t _maxSamples = 1;
	uint32_t _samples = 0;
};

class UniformSampler : public Sampler
{
public:
	ET_DECLARE_POINTER(UniformSampler);

public:
	UniformSampler(uint32_t maxSamples);
	bool next(vec2& sample) override;

protected:
	uint32_t _maxSamples = 1;
	uint32_t _gridSubdivisions = 1;
	uint32_t _samples = 0;
	float _cellSize = 1.0f;
};

class StratifiedSampler : public UniformSampler
{
public:
	ET_DECLARE_POINTER(StratifiedSampler);

public:
	StratifiedSampler(uint32_t maxSamples, float a, float b);
	bool next(vec2& sample) override;

private:
	float _a = 0.0f;
	float _b = 1.0f;
};

struct HammersleyQMCSampler
{
	void setSamplesCount(uint32_t s) 
		{ samplesCount = s; }

	float4 sample(uint32_t i, uint32_t dim);
	float rinv(uint32_t);

private:
	uint32_t samplesCount = 1;
	uint32_t index = 0;
};


}

}
