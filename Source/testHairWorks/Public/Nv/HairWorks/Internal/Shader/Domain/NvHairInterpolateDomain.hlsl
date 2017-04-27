/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#include <Nv/HairWorks/Internal/Shader/NvHairInternalShaderCommon.h>
#include <Nv/HairWorks/Internal/Shader/NvHairInterpolate.h>

#include <Nv/HairWorks/Shader/NvHairShaderCommon.h>

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Constant parameter definition helps optimized shader performance by removing branching
////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef PARAM_CLUMP_SCALE		
#define PARAM_CLUMP_SCALE		material.clumpScale
#endif

#ifndef PARAM_CLUMP_NOISE
#define PARAM_CLUMP_NOISE		material.clumpNoise
#endif

#ifndef PARAM_CLUMP_ROUNDNESS
#define PARAM_CLUMP_ROUNDNESS	material.clumpRoundness
#endif

#ifndef PARAM_WAVE_SCALE
#define PARAM_WAVE_SCALE		material.waveScale
#endif

#ifndef PARAM_WAVE_STRAND
#define PARAM_WAVE_STRAND		material.waveScaleStrand
#endif

#ifndef PARAM_WAVE_CLUMP
#define PARAM_WAVE_CLUMP		material.waveScaleClump
#endif

#ifndef PARAM_WAVE_FREQ
#define PARAM_WAVE_FREQ			material.waveFreq
#endif

#ifndef PARAM_WAVE_SCALE_NOISE
#define PARAM_WAVE_SCALE_NOISE	material.waveScaleNoise
#endif

#ifndef PARAM_WAVE_FREQ_NOISE
#define PARAM_WAVE_FREQ_NOISE	material.waveFreqNoise
#endif

#ifndef PARAM_WAVE_CUTOFF
#define PARAM_WAVE_CUTOFF		material.waveCutoff
#endif

#ifndef PARAM_WIDTH
#define PARAM_WIDTH				material.width
#endif

#ifndef PARAM_WIDTH_NOISE_SCALE
#define PARAM_WIDTH_NOISE_SCALE	material.widthNoiseScale
#endif

#ifndef PARAM_ROOT_WIDTH_SCALE
#define PARAM_ROOT_WIDTH_SCALE	material.rootWidthScale
#endif

#ifndef PARAM_TIP_WIDTH_SCALE
#define PARAM_TIP_WIDTH_SCALE	material.tipWidthScale
#endif

#ifndef PARAM_LENGTH_SCALE
#define PARAM_LENGTH_SCALE		material.lengthScale
#endif

#ifndef PARAM_LENGTH_NOISE
#define PARAM_LENGTH_NOISE		material.lengthNoise
#endif

#ifndef OPTIMIZE_RUNTIME

#define	USE_WAVE_SCALE_TEXTURE	1
#define	USE_WAVE_FREQ_TEXTURE	1
#define	USE_CLUMP_SCALE_TEXTURE	1
#define	USE_CLUMP_ROUNDNESS_TEXTURE	1
#define	USE_WIDTH_TEXTURE		1
#define	USE_LENGTH_TEXTURE		1

#define USE_PIXEL_DENSITY		1
#define USE_CULL_SPHERE			1
#define USE_CLUMPING			1
#define USE_WAVINESS			1
#define USE_WAVE_CLUMP			1

#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// compute delta position for waviness
////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline float3 NvHair_computeWaveDelta(
	float	lengthToRoot,
	float	normalizedNoise,
	float2	texcoords,
	float3	hairTangent,
	float3	hairNormal,
	in const	NvHair_TessellationMaterial material,
	float	scale = 1.0f
	)
{
	float3 biTangent1 = normalize(cross(hairTangent, hairNormal));
	float3 biTangent2 = normalize(cross(hairTangent, biTangent1));

	float waveFreq			= PARAM_WAVE_FREQ;
	float waveScaleNoise	= PARAM_WAVE_SCALE_NOISE;
	float waveFreqNoise		= PARAM_WAVE_FREQ_NOISE;
	float waveCutoff		= PARAM_WAVE_CUTOFF;

	waveScaleNoise = 1.0f + waveScaleNoise * (normalizedNoise - 1.0f);
	waveFreqNoise = 1.0f + waveFreqNoise* (normalizedNoise - 1.0f);
	waveFreq = lengthToRoot * TWO_PI * waveFreq * waveFreqNoise;

	float waveCutoffScale = min(1.0f, lengthToRoot / waveCutoff);
	float waveScale = scale * PARAM_WAVE_SCALE * waveScaleNoise * waveCutoffScale;

#ifndef OPTIMIZE_RUNTIME
	if (g_buffer.useWaveScaleTexture)
		waveScale *= NvHair_SampleChannel(g_waveScaleTexture, samLinear, texcoords.xy, g_buffer.waveScaleTextureChan.x);
#else
	#if USE_WAVE_SCALE_TEXTURE
		waveScale *= SAMPLE_WAVE_SCALE(g_waveScaleTexture, samLinear, texcoords, 0);
	#endif
#endif

#ifndef OPTIMIZE_RUNTIME
	if (g_buffer.useWaveFreqTexture)
		waveFreq *= NvHair_SampleChannel(g_waveFreqTexture, samLinear, texcoords, g_buffer.waveFreqTextureChan.x);
#else
	#if USE_WAVE_FREQ_TEXTURE
	waveFreq *= SAMPLE_WAVE_FREQ(g_waveFreqTexture, samLinear, texcoords, 0);
	#endif
#endif

	float3 wave1 = biTangent1 * sin(waveFreq);
	float3 wave2 = biTangent2 * cos(waveFreq);
	
	return waveScale * (wave1 + wave2);
}

//////////////////////////////////////////////////////////////////////////////
// get largest value from barycentric coords
//////////////////////////////////////////////////////////////////////////////
inline float3 gfsdk_pickClump(const float3 coords)
{
	if ((coords.x >= coords.y) && (coords.x >= coords.z))
		return float3(1.0f, 0.0, 0.0);
	else if ((coords.y >= coords.x) && (coords.y >= coords.z))
		return float3(0.0f, 1.0, 0.0);

	return float3(0.0f, 0.0, 1.0);
}

////////////////////////////////////////////////////////////////////////////////////////////////
// Domain shader for interpolated hair rendering 
////////////////////////////////////////////////////////////////////////////////////////////////
[domain("isoline")]
DSOut ds_main(
	OutputPatch<VSOut, 1> inputPatch, 
	HSOut		input, 
	float2		uv : SV_DomainLocation, 
	uint		primitiveId : SV_PrimitiveID)
{
	// how many hairs did we render before this master strand
	uint iHairInsideCurMasterStrand = (int)(uv.y * input.edges[0] + 0.5) + primitiveId * NHAIRS_PER_PATCH;
	uint vertexId					= (int)(uv.x * input.edges[1] + 0.5);
	float lengthToRoot				= uv.x;
	uint instanceId					= iHairInsideCurMasterStrand + floor(g_buffer.densityPass * NHAIRS_PER_PATCH);

	// hair cv id
	int3 rootIndices	= floor(input.rootIndices);
	int3 vertexIndices  = rootIndices + int3(vertexId, vertexId, vertexId);

	// barycentric coordinates
	float3 coords;
	coords.xy = g_strandCoordinatesLut.Load(instanceId & NUM_INTERPOLATED_ATTRIBUTES_MINUS_ONE);
	coords.z = (1 - coords.x - coords.y);

	// texcoords ----------------------------------------------------------------------------------
	float2 texcoords = 
		coords.x * input.texcoords[0].xy + 
		coords.y * input.texcoords[1].xy + 
		coords.z * input.texcoords[2].xy;

	NvHair_TessellationMaterial material = g_buffer.defaultMaterial;

	DSOut	output = (DSOut)0;

	// get common noise variables
	float normalizedNoise = g_noiseLut.Load(instanceId & NUM_INTERPOLATED_ATTRIBUTES_MINUS_ONE);
	float signedNoise = 2.0f * (normalizedNoise - 0.5f);

	/////////////////////////////////////////////////////////////////////////////////
	// early out on per pixel density
	/////////////////////////////////////////////////////////////////////////////////
#ifndef OPTIMIZE_RUNTIME
	if (g_buffer.usePixelDensity && g_buffer.useDensityTexture)
	{
		float densitySample = normalizedNoise + 0.005; // add small bias to avoid dark spot having hair (1/200)
		float density = NvHair_SampleChannel(g_densityTexture, samLinear, texcoords, g_buffer.densityTextureChan.x);

		if (densitySample > density)
			return output;
	}
#else // optimized version

#if USE_PIXEL_DENSITY && USE_DENSITY_TEXTURE
	float densitySample = normalizedNoise + 0.005; // add small bias to avoid dark spot having hair (1/200)
	float density = SAMPLE_DENSITY(g_densityTexture, samLinear, texcoords, 0);

	if (densitySample > density)
		return output;
#endif
#endif

	///////////////////////////////////////////////////////////////////////////////
	// interpolate position, normal, tangents
	///////////////////////////////////////////////////////////////////////////////
	
	float4 p0 = g_tessellatedMasterStrand.Load(vertexIndices[0]);
	float4 p1 = g_tessellatedMasterStrand.Load(vertexIndices[1]);
	float4 p2 = g_tessellatedMasterStrand.Load(vertexIndices[2]);
	
	float4 positionAndLength = NvHair_InterpolateBary(p0, p1, p2, coords);
	float3 position = positionAndLength.xyz;
	float3 barycentricPosition = position;

	float3 hairTangent = 0;
	float3 t0 = 0, t1 = 0, t2 = 0;
	{
		t0 = g_tessellatedTangents.Load( vertexIndices[0] ).xyz;
		t1 = g_tessellatedTangents.Load( vertexIndices[1] ).xyz;
		t2 = g_tessellatedTangents.Load( vertexIndices[2] ).xyz;

		hairTangent = NvHair_InterpolateBary(t0, t1, t2, coords);
	}

	float3 hairNormal = 0;
	{
		float3 n0 = g_tessellatedNormals.Load( vertexIndices[0] ).xyz;
		float3 n1 = g_tessellatedNormals.Load( vertexIndices[1] ).xyz;
		float3 n2 = g_tessellatedNormals.Load( vertexIndices[2] ).xyz; 

		hairNormal = NvHair_InterpolateBary(n0, n1, n2, coords);
	}

	///////////////////////////////////////////////////////////////////////////////
	// strand waviness
	///////////////////////////////////////////////////////////////////////////////
#if USE_WAVINESS
	float waveScale		= PARAM_WAVE_SCALE;
	position.xyz += NvHair_computeWaveDelta(lengthToRoot, normalizedNoise, texcoords, hairTangent, hairNormal, material, PARAM_WAVE_STRAND);
#endif

	////////////////////////////////////////////////////////////////////////////////
	// Clumping
	///////////////////////////////////////////////////////////////////////////////
#if USE_CLUMPING

	float clumpScale		= PARAM_CLUMP_SCALE;
	float clumpNoise		= PARAM_CLUMP_NOISE;
	float clumpRoundness	= PARAM_CLUMP_ROUNDNESS;
	float3 clumpDelta		= float3(0,0,0);

	if (clumpScale > 0)
	{
		float3	clumpedPosition = p2.xyz;
		float3	clumpedHairTangent = t2.xyz;
		uint	clumpedVertexId = rootIndices[2]; // hash value that need to be same for vertices of same hair.
		
		{
			if ((coords.x >= coords.y) && (coords.x >= coords.z)) // pick first vertex
			{
				clumpedPosition = p0.xyz;
				clumpedHairTangent = t0.xyz;
				clumpedVertexId = rootIndices[0];
			}
			else if ((coords.y >= coords.x) && (coords.y >= coords.z))
			{
				clumpedPosition = p1.xyz;
				clumpedHairTangent = t1.xyz;
				clumpedVertexId = rootIndices[1];
			}
		}

#if USE_WAVE_CLUMP	
#ifndef OPTIMIZE_RUNTIME
		if (PARAM_WAVE_CLUMP > 0.0f)
#endif
		{
			float	clumpNoise = g_noiseLut.Load(clumpedVertexId & NUM_INTERPOLATED_ATTRIBUTES_MINUS_ONE);
			clumpedPosition += NvHair_computeWaveDelta(
				lengthToRoot, clumpNoise, texcoords, clumpedHairTangent, hairNormal, material, PARAM_WAVE_CLUMP);
		}
#endif

#ifndef OPTIMIZE_RUNTIME
		if (g_buffer.useClumpScaleTexture)
			clumpScale *= NvHair_SampleChannel(g_clumpScaleTexture, samLinear, texcoords, g_buffer.clumpScaleTextureChan.x);
#else
		#if USE_CLUMP_SCALE_TEXTURE
		clumpScale *= SAMPLE_CLUMP_SCALE(g_clumpScaleTexture, samLinear, texcoords, 0);
		#endif
#endif

#ifndef OPTIMIZE_RUNTIME
		if (g_buffer.useClumpRoundnessTexture)
			clumpRoundness *= NvHair_SampleChannel(g_clumpRoundnessTexture, samLinear, texcoords, g_buffer.clumpRoundnessTextureChan.x);
#else
		#if USE_CLUMP_ROUNDNESS_TEXTURE
			clumpRoundness *= SAMPLE_CLUMP_ROUNDNESS(g_clumpRoundnessTexture, samLinear, texcoords, 0);
		#endif
#endif

		float clumpness = pow(lengthToRoot, clumpRoundness);
		clumpness *= lerp(1.0f, normalizedNoise, clumpNoise);
		clumpness *= clumpScale;

		// displace delta
		clumpDelta = clumpness * (clumpedPosition - barycentricPosition);
	}

	position.xyz += clumpDelta;
#endif

	/////////////////////////////////////////////////////////////////////////////////
	// Width control
	///////////////////////////////////////////////////////////////////////////////
	float width = PARAM_WIDTH;

#ifndef OPTIMIZE_RUNTIME
	if (g_buffer.useWidthTexture)
		width *= NvHair_SampleChannel(g_widthTexture, samLinear, texcoords, g_buffer.widthTextureChan.x);
#else

#if USE_WIDTH_TEXTURE
	width *= SAMPLE_WIDTH(g_widthTexture, samLinear, texcoords, 0);
#endif

#endif

	float rootWidthScale	= PARAM_ROOT_WIDTH_SCALE;
	float tipWidthScale		= PARAM_TIP_WIDTH_SCALE;

	// taper hair by two width scale
	float widthScale = clamp(lerp(rootWidthScale, tipWidthScale, lengthToRoot), 0, max(rootWidthScale, tipWidthScale));
	width *= widthScale * (1.0f + PARAM_WIDTH_NOISE_SCALE * signedNoise);

	/////////////////////////////////////////////////////////////////////////////////
	// Length variation
	///////////////////////////////////////////////////////////////////////////////
	float length = PARAM_LENGTH_SCALE * (lerp(1.0f, 0.2f + 0.8f * normalizedNoise, PARAM_LENGTH_NOISE));

#ifndef OPTIMIZE_RUNTIME
	if (g_buffer.useLengthTexture)
		length *= NvHair_SampleChannel(g_lengthTexture, samLinear, texcoords, g_buffer.lengthTextureChan);
#else

#if USE_LENGTH_TEXTURE

		length *= SAMPLE_LENGTH(g_lengthTexture, samLinear, texcoords, 0);
#endif

#endif

	if (lengthToRoot > length) 
		width = 0.0f;

#if USE_CULL_SPHERE
	/////////////////////////////////////////////////////////////////////////////////
	// cull sphere for dismemberment
	/////////////////////////////////////////////////////////////////////////////////
	if (g_buffer.useCullSphere)
	{
		float3 rp0 = g_restMasterStrand.Load( rootIndices[0] ).xyz;
		float3 rp1 = g_restMasterStrand.Load( rootIndices[1] ).xyz;
		float3 rp2 = g_restMasterStrand.Load( rootIndices[2] ).xyz;

		float3 rootBary = NvHair_InterpolateBary(rp0, rp1, rp2, coords);

		float3 transformedRoot = mul(float4(rootBary, 1), g_buffer.cullSphereInvTransform).xyz;

		if (dot(transformedRoot, transformedRoot) < 1.0) // inside culling sphere
			width = 0.0f; // discard by length in GS
	}
#endif

	////////////////////////////////////////////////////////////
	// normalize interpolated vectors
	////////////////////////////////////////////////////////////
	hairTangent		= normalize(hairTangent);
	hairNormal		= normalize(hairNormal);

	/////////////////////////////////////////////////////////////////////////////////
	// DS output
	/////////////////////////////////////////////////////////////////////////////////
	output.position		= position.xyz;
	output.texcoords	= texcoords;
	output.width		= width;
	output.tex			= lengthToRoot;
	output.tangent		= hairTangent;
	output.normal		= hairNormal;

	output.primitiveId = input.primitiveId;
	output.coords = coords.xy;

	return output;
}

