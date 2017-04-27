/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#include <Nv/HairWorks/Internal/Shader/NvHairVisualize.h>

//////////////////////////////////////////////////////////////////////////////////
StructuredBuffer<float4>	g_growthMeshVertices : register(t0);

struct VSOut
{
    float4 Position : SV_Position;
};

VSOut vs_main(uint vertexId : SV_VertexID)
{
	VSOut vertex;
	float4 pos = g_growthMeshVertices[vertexId];

	float3 wPos =  mul(float4(pos.xyz, 1), g_buffer.modelToWorld).xyz; 
    vertex.Position = mul(float4(wPos, 1), g_buffer.viewProjection);
    return vertex;
}

float4 ps_main(VSOut vertex) : SV_Target
{
	return g_buffer.color;
}

[maxvertexcount(1)]
void gs_main(point VSOut vertex[1], inout TriangleStream<VSOut> stream)
{
	stream.RestartStrip();
}