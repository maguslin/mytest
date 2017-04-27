/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#ifndef NV_HAIR_INTERPOLATE_H
#define NV_HAIR_INTERPOLATE_H

#ifdef _CPP
#	error "Can only be included in HLSL code"
#endif

#include "NvHairInternalShaderCommon.h"
#include "NvHairInternalShaderTypes.h"

void NvHair_BlendMaterial(NvHair_TessellationMaterial defaultMaterial, in NvHair_TessellationMaterial targetMaterial, float weight, out NvHair_TessellationMaterial dst)
{
#define LERP_MATERIAL(PARAM) \
	dst.PARAM	= lerp(defaultMaterial.PARAM, targetMaterial.PARAM, weight);

	LERP_MATERIAL(width);
	LERP_MATERIAL(widthNoiseScale);
	LERP_MATERIAL(rootWidthScale);
	LERP_MATERIAL(tipWidthScale);

	LERP_MATERIAL(density);
	LERP_MATERIAL(clumpScale);
	LERP_MATERIAL(clumpNoise);
	LERP_MATERIAL(clumpRoundness);

	LERP_MATERIAL(lengthNoise);
	LERP_MATERIAL(lengthScale);
	LERP_MATERIAL(waveScale);
	LERP_MATERIAL(waveFreq);

	LERP_MATERIAL(waveScaleNoise);
	LERP_MATERIAL(waveFreqNoise);
	LERP_MATERIAL(waveCutoff);
	LERP_MATERIAL(waveScaleClump);
	LERP_MATERIAL(waveScaleStrand);

#undef LERP_MATERIAL
}

//////////////////////////////////////////////////////////////////////////////
// buffers and textures
//////////////////////////////////////////////////////////////////////////////
Buffer<float4>				g_tessellatedMasterStrand		: register(t0);
Buffer<float4>				g_tessellatedMasterStrandPrev	: register(t1);

Buffer<float4>				g_restMasterStrand				: register(t2);
Buffer<float2>				g_texCoords						: register(t3);

Buffer<float3>				g_faceHairIndices				: register(t4);
Buffer<float2>				g_strandCoordinatesLut			: register(t5);
Buffer<float>				g_noiseLut						: register(t6);

Texture2D					g_densityTexture				: register(t7);
Texture2D					g_widthTexture					: register(t8);

Texture2D					g_clumpScaleTexture				: register(t9);
Texture2D					g_clumpRoundnessTexture			: register(t10);

Texture2D					g_waveScaleTexture				: register(t11);
Texture2D					g_waveFreqTexture				: register(t12);
Texture2D					g_lengthTexture					: register(t13);
Texture2D					g_weightTexture					: register(t14);

Buffer<float4>				g_tessellatedTangents			: register(t15);
Buffer<float4>				g_tessellatedNormals			: register(t16);
 
//////////////////////////////////////////////////////////////////////////////
// sampler states
//////////////////////////////////////////////////////////////////////////////
SamplerState samLinear : register(s0);
SamplerState samPointClamp : register(s1);

///////////////////////////////////////////////////////////////////////////////////////////////////////////
//constants that change frame to frame
cbuffer cbPerFrame : register( b0 )
{
	NvHair_TessellationConstantBuffer g_buffer;
}

//////////////////////////////////////////////////////////////////////////////
// Dummy vertex shader output (hull shader input)
//////////////////////////////////////////////////////////////////////////////
struct VSOut
{
	float dummy : DUMMY;
};

//////////////////////////////////////////////////////////////////////////////
// Hull shader output (Domain shader input)
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
struct HSOut
{
    float		edges[2]			: SV_TessFactor;
	float3		rootIndices			: ROOT_INDICES;
	float3x2	texcoords			: TEX_COORDS;
	uint		primitiveId			: PRIMITIVE_ID;
};

////////////////////////////////////////////////////////////////////////////////////////////////
// Domain shader output (Geometry shader input)
////////////////////////////////////////////////////////////////////////////////////////////////
struct DSOut // 14 floats
{
   float3	position	: Position;
   float2	texcoords	: SCALPTEX;
   float3	normal		: Normal;
   float3	tangent		: Tangent;
   float	width		: Width;
   float	tex			: TEXALONGLENGTH;

   uint		primitiveId : PRIMITIVE_ID;
   float2	coords		: COORDS;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline float NvHair_SampleMultiChannel( Texture2D tex, float2 texcoords, int2 channels, float weight)
{
#if MULTI_CHANNEL_SUPPORT
	float sampleDefault = 1.0f;
	float sampleTarget = 1.0f;
	float sample = 1.0f;

	NV_HAIR_SAMPLE_CHANNEL(tex, samLinear, texcoords, 0, channels[0], sampleDefault);
	NV_HAIR_SAMPLE_CHANNEL(tex, samLinear, texcoords, 0, channels[1], sampleTarget); // optimize?

	if (weight == 0.0f)
		sample = sampleDefault;
	else
	{
		sample = lerp(sampleDefault, sampleTarget, weight);
	}
#else
	float sample = 1.0f;

	NV_HAIR_SAMPLE_CHANNEL(tex, samLinear, texcoords, 0, channels[0], sample);
#endif

	return sample;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline void NvHair_BlendMaterial(out NvHair_TessellationMaterial material, float weight = 0.0f)
{
	material = g_buffer.defaultMaterial;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline float3 NvHair_InterpolateBary(float3 v0, float3 v1, float3 v2, float3 coords)
{
	return coords.x  * v0.xyz + coords.y * v1.xyz + coords.z * v2.xyz; 
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline float4 NvHair_InterpolateBary(float4 v0, float4 v1, float4 v2, float3 coords)
{
	return coords.x  * v0 + coords.y * v1 + coords.z * v2; 
}

//////////////////////////////////////////////////////////////////////////////
// compression and decompression
//////////////////////////////////////////////////////////////////////////////
float packFloat2(float x, float y)
{
	const float base = 2048;

	float basey = floor(base * y);
	float packed = basey + x;

	return packed;
}

float packSignedFloat(float x)
{
	return 0.5f + 0.5f * clamp(x, -1.0, 1.0);
}

float packSignedFloat2(float x, float y)
{
	float sx = packSignedFloat(x);
	float sy = packSignedFloat(y);

	return packFloat2(sx,sy);
}


inline float2 unpackFloat2(float packed)
{
	const float inv_base = 1.0f / 2048.0f;

	float ubase = floor(packed);
	float unpackedy = ubase * inv_base;
	float unpackedx = packed - ubase;

	return float2(unpackedx, unpackedy);
}

inline float unpackSignedFloat(float x)
{
	return clamp(2.0f * (x - 0.5f), -1.0f, 1.0f);
}

inline float2 unpackSignedFloat2(float x)
{
	float2 unpacked = unpackFloat2(x);
	float sx = unpackSignedFloat(unpacked.x);
	float sy = unpackSignedFloat(unpacked.y);

	return float2(sx, sy);
}

//////////////////////////////////////////////////////////////////////////////
float3 NvHair_GetCameraDirection()
{
	//float3 dir = gfsdk_getBasisZ(g_buffer.inverseViewMatrix);
	float3 dir = float3(
		g_buffer.inverseViewMatrix._31,
		g_buffer.inverseViewMatrix._32,
		g_buffer.inverseViewMatrix._33);

	if (g_buffer.leftHanded)
		dir *= -1;

	return dir;
}

#endif  // NV_HAIR_INTERPOLATE_H
