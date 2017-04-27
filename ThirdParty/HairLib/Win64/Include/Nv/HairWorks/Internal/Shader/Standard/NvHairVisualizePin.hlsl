/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#include <Nv/HairWorks/Internal/Shader/NvHairVisualize.h>
#include <Nv/HairWorks/Internal/Shader/NvHairSimulate.h>

StructuredBuffer<float4>	g_globalScratchMem : register(t0);
StructuredBuffer<NvHair_Pin> g_pinBuffer : register(t1);

struct VSIn
{
    float3 position : POSITION;
};

struct VSOut
{
    float4 position : SV_Position;
};

VSOut vs_main(VSIn vertexIn)
{
	VSOut vertex;

	NvHair_Pin pc = g_pinBuffer[g_buffer.pinId];
	float3 wPos = mul(mul(float4(vertexIn.position.xyz * g_buffer.scale, 1),pc.shapeMatrix), pc.currentPinMatrix).xyz;

	wPos = mul(float4(wPos.xyz, 1), g_buffer.modelToWorld).xyz;

	vertex.position = mul(float4(wPos, 1), g_buffer.viewProjection);

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