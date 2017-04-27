/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#include <Nv/HairWorks/Internal/Shader/NvHairVisualize.h>

//////////////////////////////////////////////////////////////////////////////////
StructuredBuffer<float4>	g_masterStrandVertices : register(t0);
Buffer<int>					g_vertexToHair : register(t1);

//////////////////////////////////////////////////////////////////////////////////
struct VSOut
{
    float4	position	: SV_Position;
	int		hairId		: HairID;
};

//////////////////////////////////////////////////////////////////////////////////
VSOut vs_main(uint vertexId : SV_VertexID)
{
	VSOut vertex;

	float4 vpos			= g_masterStrandVertices[vertexId];
	float3 wPos			= mul(float4(vpos.xyz, 1), g_buffer.modelToWorld).xyz; 
    vertex.position		= mul(float4(wPos, 1), g_buffer.viewProjection);

	vertex.hairId		= g_vertexToHair[vertexId];

    return vertex;
}

float4 ps_main(VSOut vertex) : SV_Target
{
	if (false == NvHair_CheckHairToShow(g_buffer, vertex.hairId))
		discard;

	return g_buffer.color;
}

[maxvertexcount(1)]
void gs_main(point VSOut vertex[1], inout TriangleStream<VSOut> stream)
{
	stream.RestartStrip();
}