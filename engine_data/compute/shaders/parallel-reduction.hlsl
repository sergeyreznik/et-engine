#include <et>
#include <inputdefines>
#include "../../materials/shaders/srgb.h"

Texture2D<float> inputImage : DECLARE_TEXTURE;
RWTexture2D<float> outputImage : DECLARE_STORAGE;

#if (LUMINOCITY_ADAPTATION)
cbuffer ObjectVariables : DECL_OBJECT_BUFFER {
	float deltaTime;
};
#endif

struct CSInput
{
	uint3 groupIndex : SV_GroupID;
	uint3 groupThreadIndex : SV_GroupThreadID;
};

static const uint ThreadGroupSize = 16;
static const uint numThreads = ThreadGroupSize * ThreadGroupSize;

groupshared float4 sharedMemory[numThreads];

[numthreads(ThreadGroupSize, ThreadGroupSize, 1)]
void computeMain(CSInput input) 
{
	const uint2 sampleIndex = input.groupIndex.xy * 32 + input.groupThreadIndex.xy * 2;
	const uint threadIndex = input.groupThreadIndex.y * ThreadGroupSize + input.groupThreadIndex.x;

	float s0 = inputImage[sampleIndex + uint2(0, 0)];
	float s1 = inputImage[sampleIndex + uint2(1, 0)];
	float s2 = inputImage[sampleIndex + uint2(0, 1)];
	float s3 = inputImage[sampleIndex + uint2(1, 1)];
	
	sharedMemory[threadIndex] = float4(s0, s1, s2, s3);
	GroupMemoryBarrierWithGroupSync();

	for (uint s = numThreads / 2; s > 0; s >>= 1)
	{
		if (threadIndex < s)
		{
			sharedMemory[threadIndex] += sharedMemory[threadIndex + s];
		}
		GroupMemoryBarrierWithGroupSync();
	}

	if (threadIndex == 0)
	{
		float value = dot(sharedMemory[0], 0.25f / float(numThreads));

	#if (DOWNSAMPLE)
		
		outputImage[input.groupIndex.xy] = value;
	
	#elif (LUMINOCITY_ADAPTATION)
		
		float lowerBound = evToLuminance(expectedEv - adaptationRange.x);
		float upperBound = evToLuminance(expectedEv + adaptationRange.y);
		float currentValue = clamp(exp(value), lowerBound, upperBound);

		float previousValue = outputImage.Load(uint2(0, 0));
		float adaptationSpeed = lerp(3.0, 5.0, step(currentValue - previousValue, 0.0));
		outputImage[uint2(0, 0)] = lerp(previousValue, currentValue, 1.0f - exp(-deltaTime * adaptationSpeed));
   	
   	#else
		
		outputImage[input.groupIndex.xy] = 1.0;
	
	#endif
	}
}
