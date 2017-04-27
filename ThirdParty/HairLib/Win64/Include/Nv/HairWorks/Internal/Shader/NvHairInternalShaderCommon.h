/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#ifndef NV_HAIR_INTERNAL_SHADER_COMMON_H
#define NV_HAIR_INTERNAL_SHADER_COMMON_H

#define NUM_INTERPOLATED_ATTRIBUTES_MINUS_ONE 1023
#define NUM_INTERPOLATED_ATTRIBUTES 1024
#define LUT_SIZE_MINUS_ONE 1023

#define NHAIRS_PER_PATCH 64
#define NSEGMENTS_PER_PATCH 256 

#define TWO_PI 3.141592 * 2.0f

#ifndef FLT_EPSILON
#define FLT_EPSILON	0.0000001f
#endif

#ifdef _CPP // C++ code

#define float4			gfsdk_float4
#define float3			gfsdk_float3
#define float2			gfsdk_float2
#define float4x4		gfsdk_float4x4
#define matrix4			gfsdk_float4x4
#define DualQuaternion	gfsdk_dualquaternion

typedef int				int2[2];
typedef int				int4[4];

#define lerp			gfsdk_lerp

#else // shader

struct DualQuaternion
{
	float4 q0;
	float4 q1;
};

#define matrix4			row_major float4x4
#define MATERIAL_CHANNELS	float2

inline float NvHair_LerpChannel(float2 channel, float s)
{
	return channel.x;
}

#define SAMPLE_RED(TEX, SAMPLER, TEXCOORD, MIPLEVEL) TEX.SampleLevel(SAMPLER, TEXCOORD, MIPLEVEL).r;
#define SAMPLE_GREEN(TEX, SAMPLER, TEXCOORD, MIPLEVEL) TEX.SampleLevel(SAMPLER, TEXCOORD, MIPLEVEL).g;
#define SAMPLE_BLUE(TEX, SAMPLER, TEXCOORD, MIPLEVEL) TEX.SampleLevel(SAMPLER, TEXCOORD, MIPLEVEL).b;
#define SAMPLE_ALPHA(TEX, SAMPLER, TEXCOORD, MIPLEVEL) TEX.SampleLevel(SAMPLER, TEXCOORD, MIPLEVEL).a;

#define NV_HAIR_SAMPLE_CHANNEL(TEX, SAMPLER, TEXCOORD, MIPLEVEL, CHANNEL, SAMPLE) \
	if (CHANNEL == 0) \
		SAMPLE = TEX.SampleLevel(SAMPLER, TEXCOORD, MIPLEVEL).r; \
	else if (CHANNEL == 1) \
		SAMPLE = TEX.SampleLevel(SAMPLER, TEXCOORD, MIPLEVEL).g; \
	else if (CHANNEL == 2) \
		SAMPLE = TEX.SampleLevel(SAMPLER, TEXCOORD, MIPLEVEL).b; \
	else if (CHANNEL == 3) \
		SAMPLE = TEX.SampleLevel(SAMPLER, TEXCOORD, MIPLEVEL).a;

inline float NvHair_SampleChannel( Texture2D tex, SamplerState texSampler, float2 texcoords, int channel, float weight = 1.0f)
{
	float sample = 1.0f;
	NV_HAIR_SAMPLE_CHANNEL(tex, texSampler, texcoords, 0, channel, sample);
	return sample;
}

#endif // _CPP 

#endif  // NV_HAIR_INTERNAL_SHADER_COMMON_H

