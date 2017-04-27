/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#ifndef OPTIMIZE_RUNTIME

#define USE_PIXEL_DENSITY		1
#define USE_DENSITY_TEXTURE		1
#define USE_WEIGHT_TEXTURE		1

#define SAMPLE_DENSITY			SAMPLE_RED

#endif

#include <Nv/HairWorks/Internal/Shader/NvHairInternalShaderCommon.h>
#include <Nv/HairWorks/Internal/Shader/NvHairInterpolate.h>

//////////////////////////////////////////////////////////////////////////////
inline float2 NvHair_getClipSpaceCoord(float3 p)
{
	// convert to view space (clip space)
	float4 screenPos = mul(float4(p.xyz, 1), g_buffer.viewProjection);
	screenPos.xy /= screenPos.w;

	return screenPos.xy;
}

//////////////////////////////////////////////////////////////////////////////
inline bool NvHair_checkClipping(float2 pos, float size)
{
	if ((pos.x < -size) || (pos.x > size))
		return true;

	if ((pos.y < -size) || (pos.y > size))
		return true;

	return false;
}

//////////////////////////////////////////////////////////////////////////////
// compute density based on view frustrum clipping
inline float NvHair_viewFrustrumClipDensity(float3 rootIndices, int numSegments)
{
	int rootIndex0 = floor(rootIndices.x);
	int rootIndex1 = floor(rootIndices.y);
	int rootIndex2 = floor(rootIndices.z);
	
	int tipIndex0 = rootIndex0 + numSegments;
	int tipIndex1 = rootIndex1 + numSegments;
	int tipIndex2 = rootIndex2 + numSegments;

	// root position for early culling
	float3 r0 = g_tessellatedMasterStrand.Load(rootIndex0).xyz;
	float3 r1 = g_tessellatedMasterStrand.Load(rootIndex1).xyz;
	float3 r2 = g_tessellatedMasterStrand.Load(rootIndex2).xyz;

	float2 screenPos0 = NvHair_getClipSpaceCoord(r0);
	float2 screenPos1 = NvHair_getClipSpaceCoord(r1);
	float2 screenPos2 = NvHair_getClipSpaceCoord(r2);

	float3 t0 = g_tessellatedMasterStrand.Load(tipIndex0).xyz;
	float3 t1 = g_tessellatedMasterStrand.Load(tipIndex1).xyz;
	float3 t2 = g_tessellatedMasterStrand.Load(tipIndex2).xyz;

	float threshold = 1.05f;

	bool rootHidden0 = NvHair_checkClipping(NvHair_getClipSpaceCoord(r0), threshold);
	bool rootHidden1 = NvHair_checkClipping(NvHair_getClipSpaceCoord(r1), threshold);
	bool rootHidden2 = NvHair_checkClipping(NvHair_getClipSpaceCoord(r2), threshold);

	bool tipHidden0 = NvHair_checkClipping(NvHair_getClipSpaceCoord(t0), threshold);
	bool tipHidden1 = NvHair_checkClipping(NvHair_getClipSpaceCoord(t1), threshold);
	bool tipHidden2 = NvHair_checkClipping(NvHair_getClipSpaceCoord(t2), threshold);

	if (rootHidden0 && rootHidden1 && rootHidden2 &&
		tipHidden0 && tipHidden1 && tipHidden2)
		return 0.0f;

	return 1.0f;
}

/////////////////////////////////////////////////////////////////////////////
inline float NvHair_backfaceCulling(float3 rootIndices)
{
	float3 n0 = g_tessellatedNormals.Load( rootIndices ).xyz;
	float3 n1 = g_tessellatedNormals.Load( rootIndices ).xyz;
	float3 n2 = g_tessellatedNormals.Load( rootIndices ).xyz; 
		
	float3 d =  NvHair_GetCameraDirection();

	float eps = g_buffer.backfaceCullingThreshold;
	bool cull0 = dot(n0, d) < eps;
	bool cull1 = dot(n1, d) < eps;
	bool cull2 = dot(n2, d) < eps;

	if (cull0 && cull1 && cull2)
		return 0.0f;

	return 1.0f;
}

//////////////////////////////////////////////////////////////////////////////
// Patch constant function for hull shader for rendering pass
//////////////////////////////////////////////////////////////////////////////
HSOut InterpolateHSConst(uint iWisp : SV_PrimitiveID)
{    
    HSOut output = (HSOut)0;

	int numSegments = g_buffer.strandPointCount - 1;

	float3 hairIndices = g_faceHairIndices.Load(iWisp);

	output.rootIndices = floor(
		float3(
			hairIndices.x * g_buffer.strandPointCount + 0.5f, 
			hairIndices.y * g_buffer.strandPointCount + 0.5f,
			hairIndices.z * g_buffer.strandPointCount + 0.5f
			)
		);

	output.texcoords[0] = g_texCoords.Load( iWisp * 3);
	output.texcoords[1] = g_texCoords.Load( iWisp * 3 + 1);
	output.texcoords[2] = g_texCoords.Load( iWisp * 3 + 2);

	output.primitiveId = iWisp;

	float2 texcoords = 1.0 / 3.0 * (output.texcoords[0] + output.texcoords[1] + output.texcoords[2]);

	float weight = g_buffer.materialWeight;

	NvHair_TessellationMaterial material = g_buffer.defaultMaterial;

	// density is pre-interpolated from LOD, etc.
	float density = NvHair_LerpChannel(material.density, weight);
		  density = min(1.0f, density - g_buffer.densityPass);
	
	// set density to zero for hairs completely outside view frustrum (both tip and root)
	if (g_buffer.useViewfrustrumCulling)
		density *= NvHair_viewFrustrumClipDensity(output.rootIndices, numSegments);

	// apply backface culling 
	if (g_buffer.useBackfaceCulling)
		density *= NvHair_backfaceCulling(output.rootIndices);

#ifndef OPTIMIZE_RUNTIME
	if (!g_buffer.usePixelDensity && g_buffer.useDensityTexture)
		density *= NvHair_SampleChannel(g_densityTexture, samLinear, texcoords, g_buffer.densityTextureChan);
#else

	#if USE_DENSITY_TEXTURE && !(USE_PIXEL_DENSITY)
	density *= SAMPLE_DENSITY(g_densityTexture, samLinear, texcoords, 0);
	#endif
#endif

	output.edges[0] = max(NHAIRS_PER_PATCH * density, 0);
	output.edges[1] = min(numSegments, NSEGMENTS_PER_PATCH);

    return output;
}

//////////////////////////////////////////////////////////////////////////////
[domain("isoline")]
[partitioning("integer")]
[outputtopology("line")]
[outputcontrolpoints(1)]
[patchconstantfunc("InterpolateHSConst")]
void hs_main(InputPatch<VSOut, 1> inputPatch)
{
}

