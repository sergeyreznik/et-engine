#include <et>
#include <inputdefines>
#include "importance-sampling.h"
#include "atmosphere.h"
#include "environment.h"
#include "bsdf.h"

#if (WRAP_EQ_TO_CUBEMAP)

	Texture2D<float4> baseColorTexture : DECL_TEXTURE(BaseColor);

#elif (VISUALIZE_CUBEMAP || SPECULAR_CONVOLUTION || DOWNSAMPLE_CUBEMAP)

	TextureCube<float4> baseColorTexture : DECL_TEXTURE(BaseColor);

#endif

SamplerState baseColorSampler : DECL_SAMPLER(BaseColor);

cbuffer ObjectVariables : DECL_BUFFER(Object)
{
	row_major float4x4 worldTransform;
	float3 lightColor;
	float4 lightDirection;
};

cbuffer MaterialVariables : DECL_BUFFER(Material)
{
	float4 extraParameters;
};

struct VSOutput 
{
	float4 position : SV_Position;
	float2 texCoord0 : TEXCOORD0;
	float3 direction : TEXCOORD1;
};

VSOutput vertexMain(uint vertexIndex : SV_VertexID)
{
	float2 pos = float2((vertexIndex << 1) & 2, vertexIndex & 2) * 2.0 - 1.0;

	VSOutput vsOut;
	vsOut.texCoord0 = pos * 0.5 + 0.5;
	vsOut.position = float4(pos, 0.0, 1.0);
	vsOut.direction = mul(vsOut.position, worldTransform).xyz;

#if (VISUALIZE_CUBEMAP)
	vsOut.position = mul(vsOut.position, worldTransform);
#endif

	return vsOut;
}

float4 fragmentMain(VSOutput fsIn) : SV_Target0
{
#if (VISUALIZE_CUBEMAP)	

	float phi = fsIn.texCoord0.x * 2.0 * PI - PI;
	float theta = fsIn.texCoord0.y * PI - 0.5 * PI;
	float sinTheta = sin(theta);
	float cosTheta = cos(theta);
	float sinPhi = sin(phi);
	float cosPhi = cos(phi);
	float3 sampleDirection = float3(cosPhi * cosTheta, sinTheta, sinPhi * cosTheta);
	return baseColorTexture.SampleLevel(baseColorSampler, sampleDirection, extraParameters.x);

#elif (WRAP_EQ_TO_CUBEMAP)

	float3 d = normalize(fsIn.direction);        	
	float u = atan2(d.z, d.x) * 0.5 / PI + 0.5;
	float v = asin(d.y) / PI + 0.5;
	return baseColorTexture.SampleLevel(baseColorSampler, float2(u, v), 0.0);

#elif (ATMOSPHERE)

	float3 d = normalize(fsIn.direction);        	
	return float4(sampleAtmosphere(d, lightDirection.xyz, lightColor), 1.0);

#elif (DOWNSAMPLE_CUBEMAP)
	
	return baseColorTexture.SampleLevel(baseColorSampler, fsIn.direction, extraParameters.x - 1.0);

#elif (SPECULAR_CONVOLUTION)

	float3 n = normalize(fsIn.direction);
	float3 v = n;
	
	float3 up = abs(n.y) < 0.999 ? float3(0.0, 1.0, 0.0) : float3(1.0, 0.0, 0.0);
	float3 tX = normalize(cross(up, n));
	float3 tY = cross(n, tX);

	#define samples 2048
	float roughness = clamp(extraParameters.x / 8.0, MIN_ROUGHNESS, 1.0);
	roughness = roughness * roughness;

	float cubemapSolidAngle = 4.0 * PI / (6.0 * (extraParameters.y * extraParameters.z));

	float3 result = 0.0;
	float weight = 0.0;
	for (uint i = 0; i < samples; ++i)
	{
		float2 Xi = hammersley(i, samples);
		float3 h = importanceSampleGGX(Xi, roughness);	

		h = tX * h.x + tY * h.y + n * h.z;
		float3 l = 2.0 * dot(v, h) * h - v;
		float LdotN = saturate(dot(l, n));	
		float NdotH = dot(n, h);
		float LdotH = saturate(dot(l, h));
		float pdf = ggxDistribution(NdotH, roughness) * NdotH / (4.0 * LdotH);
		float w = LdotN;
		float sampleSolidAngle = 1.0 / ((float)(samples) * pdf);
		float level = clamp(2.0 + 0.5 * log2(1.0 + sampleSolidAngle / cubemapSolidAngle), 0.0, 8.0);
		result += w * baseColorTexture.SampleLevel(baseColorSampler, l, level).xyz;
		weight += w;
	}
	result /= max(0.0001, weight);
	return float4(result, 1.0);

#else

	return 1.0;

#endif
}
