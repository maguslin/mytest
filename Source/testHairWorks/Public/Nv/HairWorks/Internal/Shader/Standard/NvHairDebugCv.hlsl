/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#include <Nv/HairWorks/Internal/Shader/NvHairVisualize.h>

//////////////////////////////////////////////////////////////////////////////////
StructuredBuffer<float4>	g_masterStrandVertices	: register(t0);
StructuredBuffer<float>		g_MasterStrandLuminances	: register(t1);
Buffer<int>					g_VertexToHair			: register(t2);

//////////////////////////////////////////////////////////////////////////////////
struct VSOut
{
    float4	position	: SV_Position;
	float	luminance	: Luminance;
	int		hairId		: HairID;
};

//////////////////////////////////////////////////////////////////////////////////

VSOut vs_main(uint vertexId : SV_VertexID)
{
	VSOut vertex;

	float4 vpos = g_masterStrandVertices[vertexId];
	float3 wPos =  mul(float4(vpos.xyz, 1), g_buffer.modelToWorld).xyz; 
    vertex.position = mul(float4(wPos, 1), g_buffer.viewProjection);

	vertex.luminance = g_MasterStrandLuminances[vertexId];
	vertex.hairId = g_VertexToHair[vertexId];

    return vertex;
}

[maxvertexcount(4)]
void gs_main(point VSOut vertex[1], inout TriangleStream<VSOut> stream)
{
	VSOut hairPoint;

	float4 center = vertex[0].position;

	float width = g_buffer.hairWidth;

	float widthX = width / g_buffer.aspect;
	float widthY = width;

	hairPoint.luminance = vertex[0].luminance;
	hairPoint.hairId = vertex[0].hairId;

	hairPoint.position = center;
	hairPoint.position.xyz += widthX * float3(-1,0,0) + widthY * float3(0,1,0);
	stream.Append(hairPoint);

	hairPoint.position = center;
	hairPoint.position.xyz += widthX * float3(1,0,0) + widthY * float3(0,1,0);
	stream.Append(hairPoint);

	hairPoint.position = center;
	hairPoint.position.xyz += widthX * float3(-1,0,0) + widthY * float3(0,-1,0);
	stream.Append(hairPoint);

	hairPoint.position = center;
	hairPoint.position.xyz += widthX * float3(1,0,0) + widthY * float3(0,-1,0);
	stream.Append(hairPoint);

	stream.RestartStrip();
}

float4 ps_main(VSOut vertex) : SV_Target
{
	if (false == NvHair_CheckHairToShow(g_buffer, vertex.hairId))
		discard;

	// If has pins the luminance will be set in the simulation step. If not, the buffer holds junk.
	float4 color;
	float luminance = vertex.luminance;
	if (luminance < 0)
	{
		color = g_buffer.color;
	} 
	else
	{
		color = float4(luminance, luminance, luminance, 1.0);
	}
	return color;
}
