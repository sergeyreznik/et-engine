#include <et>
#include <inputdefines>
#include "importance-sampling.h"
#include "bsdf.h"

struct VSOutput 
{
	float4 position : SV_Position;
	float2 texCoord0 : TEXCOORD0;
};

#include "vertexprogram-2d-triangle.h"

float4 fragmentMain(VSOutput fsIn) : SV_Target0
{
	#define samples 2048

	float roughness = fsIn.texCoord0.x * fsIn.texCoord0.x;
	float roughnessSquared = roughness * roughness;
	float NdotV = fsIn.texCoord0.y;

	float3 n = float3(0.0, 0.0, 1.0);
	float3 tX = float3(1.0, 0.0, 0.0);
	float3 tY = float3(0.0, 1.0, 0.0);

	float3 v = float3(sqrt(1.0 - NdotV * NdotV), 0.0, NdotV);
	
	float3 result = 0.0;

	for (uint i = 0; i < samples; ++i)
	{
		float2 Xi = hammersley(i, samples);
		float3 h = importanceSampleGGX(Xi, roughnessSquared);
		h = tX * h.x + tY * h.y + n * h.z;
		float3 l = 2.0 * dot(v, h) * h - v;
		float LdotN = saturate(dot(l, n));
		float NdotH = saturate(dot(n, h));
		float LdotH = saturate(dot(l, h));
			
		float d = ggxDistribution(NdotH, roughnessSquared);
		float g = ggxMasking(NdotV, LdotN, roughnessSquared);
		float f = fresnel(0.04, 1.0, LdotH);
		float brdf_over_pdf = g * LdotH / (NdotV * NdotH);

		result.x += (1.0 - f) * brdf_over_pdf;
		result.y += f * brdf_over_pdf;

		h = importanceSampleCosine(Xi);
		l = tX * h.x + tY * h.y + n * h.z;
		l = 2.0 * dot(v, l) * l - v;
		LdotN = saturate(dot(l, n));
		LdotH = saturate(dot(l, h));
		result.z += diffuseBurley(LdotN, NdotV, LdotH, roughnessSquared);
	}
	
	return float4(result / samples, 1.0); 
}
