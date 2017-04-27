/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#include <Nv/HairWorks/Internal/Shader/NvHairInternalShaderCommon.h>
#include <Nv/HairWorks/Internal/Shader/NvHairInterpolate.h>

#include <Nv/HairWorks/Shader/NvHairShaderCommon.h>

//////////////////////////////////////////////////////////////////////////////////
// Geometry shader for regular rendering pass
//////////////////////////////////////////////////////////////////////////////////
// turns input line segment into a quad (strip)
//
// This is a more general version of the InterpolateGeometry that can handle both cubemap 
// and non cubemap rendering. 
// It is not used if cubemapping is not required because it is substantially slower. 
//
// maxvertex count is 24 (rather than 4), to handle if cubemap rendering is enabled (6 x 4 = 24)
[maxvertexcount(24)]
void gs_main(line DSOut vertex[2], inout TriangleStream<NvHair_CubemapPixelShaderInput> stream)
{
	float width0 = vertex[0].width;
	float width1 = vertex[1].width;

	if((width0 == 0) && (width1 == 0))
		return;

	float3 worldPos0 = vertex[0].position;
	float3 worldPos1 = vertex[1].position;

	NvHair_CubemapPixelShaderInput output;

	output.primitiveId = vertex[0].primitiveId;
	output.coords = NvHair_PackFloat2(vertex[0].coords.xy);

	float3 tangent0 = vertex[0].tangent.xyz;
	float3 tangent1 = vertex[1].tangent.xyz;
	float3 normal0 = vertex[0].normal.xyz;
	float3 normal1 = vertex[1].normal.xyz;
	float2 texcoords = vertex[0].texcoords;

	// Determine whether to output to cube map
	const int faceCount = g_buffer.cubeMapActive.x ? 6 : 1;

	[unroll]
	for(int faceIdx = 0; faceIdx < faceCount; ++faceIdx)
	{
		// Check visibility for each face
		if(g_buffer.cubeMapActive.x && !g_buffer.cubeMapVisible[faceIdx].x)
			continue;

		// Set face index
		output.cubeMapIdx = faceIdx;

		// Setup camera
		float3 eyeVec;
		matrix4 viewProjection;

		if(g_buffer.cubeMapActive.x)
		{
			matrix4 invView = g_buffer.cubeMapInvViewMatrix[faceIdx];
			eyeVec.x = invView._31;
			eyeVec.y = invView._32;
			eyeVec.z = invView._33;
			if(g_buffer.leftHanded)
				eyeVec *= -1;

			viewProjection = g_buffer.cubeMapViewProjMatrix[faceIdx];
		}
		else
		{
			eyeVec = NvHair_GetCameraDirection();

			viewProjection = g_buffer.viewProjection;
		}

		float3 sideVec0 = width0 * normalize(cross(eyeVec, tangent0));
		float3 sideVec1 = width1 * normalize(cross(eyeVec, tangent1));

		float3 pos0 = worldPos0 - sideVec0;
		float3 pos1 = worldPos0 + sideVec0;
		float3 pos2 = worldPos1 - sideVec1;
		float3 pos3 = worldPos1 + sideVec1;

		// all points common
		output.compTexcoord.x = NvHair_PackFloat2(texcoords.xy);

		// Point 0 - common
		output.hairtex = vertex[0].tex;

		// Point 0 - left
		output.position = mul(float4(pos0, 1.0), viewProjection);
		stream.Append(output);

		// Point 0 - right
		output.position = mul(float4(pos1, 1.0), viewProjection);
		stream.Append(output);

		// Point 1 - common
		output.hairtex = vertex[1].tex;

		// Point 1 - left
		output.position = mul(float4(pos2, 1.0), viewProjection);
		stream.Append(output);

		// Point 1 - right
		output.position = mul(float4(pos3, 1.0), viewProjection);
		stream.Append(output);

		stream.RestartStrip();
	}
}

