/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#include <Nv/HairWorks/Internal/Shader/NvHairVisualize.h>

/////////////////////////////////////////////////////////////////////////
StructuredBuffer<float4>	g_masterStrandVertices	: register(t0);
Buffer<int>					g_interactionIndices	: register(t1);
Buffer<int>					g_interactionOffset		: register(t2);
Buffer<float>				g_interactionLength		: register(t3);

//////////////////////////////////////////////////////////////////////////////////
// Hair rendering for debugging
struct VSOut
{
	int		vertexId : VertexID;
	int		start : START_VertexID;
	int		end : END_VertexID;
};

inline float4 modelProjection(float4 inpos)
{
	float3 wPos =  mul(float4(inpos.xyz, 1), g_buffer.modelToWorld).xyz; 
    float4 outPos = mul(float4(wPos, 1), g_buffer.viewProjection);

	return outPos;
}

VSOut vs_main(uint vertexId : SV_VertexID)
{
	VSOut vertex;

	vertex.vertexId = vertexId;

	vertex.start = (vertexId == 0) ? 0 : g_interactionOffset[vertexId-1];
	vertex.end = g_interactionOffset[vertexId];

    return vertex;
}

struct GSOut
{
    float4 position : SV_Position;
	float residual : RES;
};

[maxvertexcount(10)]
void gs_main(point VSOut vertex[1], inout LineStream<GSOut> stream)
{
	GSOut hairPoint;

	int vertexId = vertex[0].vertexId;
	int start = vertex[0].start;
	int end = vertex[0].end;
	int numLines = end - start;

	float4 p0 = g_masterStrandVertices[vertexId];
	
	for (int i = 0; i < numLines; i++)
	{
		int id = g_interactionIndices[start + i];
		float4 p1 = g_masterStrandVertices[id];
		
		float restLength = g_interactionLength[start + i];

		hairPoint.residual = (length(p0.xyz - p1.xyz) - restLength) / restLength;

		hairPoint.position = modelProjection(p0);
		stream.Append(hairPoint);
		
		hairPoint.position = modelProjection(p1);
		stream.Append(hairPoint);

		stream.RestartStrip();
	}
}

float4 ps_main(GSOut vertex) : SV_Target
{
	float4 color1 = float4(1,0,1,1);
	float4 color2 = float4(0,1,0,1);

	float ratio = saturate(vertex.residual / 0.5f);
	float4 psColor = lerp(color1, color2, ratio);

	return psColor;
}

