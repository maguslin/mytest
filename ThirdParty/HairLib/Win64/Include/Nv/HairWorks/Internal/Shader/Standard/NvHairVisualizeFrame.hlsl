/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#include <Nv/HairWorks/Internal/Shader/NvHairVisualize.h>
#include <Nv/HairWorks/Internal/Shader/NvHairShaderMath.h>

//////////////////////////////////////////////////////////////////////////////////
StructuredBuffer<float4>	g_masterStrandVertices	: register(t0);
StructuredBuffer<float4>	g_masterStrandFrames	: register(t1);
Buffer<int>					g_vertexToHair			: register(t2);

//////////////////////////////////////////////////////////////////////////////////
struct VSOut
{
    float4	position	: SV_Position;
	int		hairId		: HairID;
};

//////////////////////////////////////////////////////////////////////////////////
// Hair rendering for debugging
VSOut vs_main(uint vertexId : SV_VertexID)
{
	VSOut vertex;

	uint cvId = vertexId / 4;
	uint axisId = vertexId % 4;

	float3 vpos = g_masterStrandVertices[cvId].xyz;

	float3 delta = float3(0,0,0);

	float scale = g_buffer.color.r;

	switch (axisId)
	{
	case 0:
		delta = float3(0,0,0);
		break;
	case 1:
		delta = float3(1,0,0);
		break;
	case 2:
		delta = float3(0,1,0);
		break;
	case 3:
		delta = float3(0,0,1);
		break;
	}

	float4 q = g_masterStrandFrames[cvId];

	float3 axis = scale * gfsdk_rotate(q, delta) + vpos;

	float3 wPos =  mul(float4(axis.xyz, 1), g_buffer.modelToWorld).xyz; 

    vertex.position = mul(float4(wPos, 1), g_buffer.viewProjection);
	vertex.hairId = g_vertexToHair[cvId];

    return vertex;
}

float4 ps_main(
	VSOut vertex,
	uint primitiveId : SV_PrimitiveID 
	) : SV_Target
{
	if (false == NvHair_CheckHairToShow(g_buffer, vertex.hairId))
		discard;

	uint colorId = primitiveId % 3;
	float4 color = float4(0,0,0,1);

	switch (colorId)
	{
	case 0:
		color.xyz = float3(1,0,0);
		break;
	case 1:
		color.xyz = float3(0,1,0);
		break;
	case 2:
		color.xyz = float3(0,0,1);
		break;
	}
	return color;
}

[maxvertexcount(1)]
void gs_main(point VSOut vertex[1], inout TriangleStream<VSOut> stream)
{
	stream.RestartStrip();
}