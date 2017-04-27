/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#include <Nv/HairWorks/Internal/Shader/NvHairInternalShaderCommon.h>
#include <Nv/HairWorks/Internal/Shader/NvHairInternalShaderTypes.h>

cbuffer cbPerFrame : register(b0)
{
	NvHair_VisualizeConstantBuffer g_buffer;
};

//////////////////////////////////////////////////////////////////////////////////
struct VSIn
{
    float3 Position : POSITION;
	float4 Color : COLOR;
};

struct VSOut
{
    float4 Position : SV_Position;
	float4 Color : COLOR;
};

VSOut vs_main(VSIn vertexIn)
{
	VSOut vertex;
	
	float3 wPos =  mul(float4(vertexIn.Position.xyz, 1), g_buffer.modelToWorld).xyz; 
    vertex.Position = mul(float4(wPos, 1), g_buffer.viewProjection);
	vertex.Color = g_buffer.color * vertexIn.Color;
	//vertex.Color = vertexIn.Color;
    return vertex;
}

float4 ps_main(VSOut vertexIn) : SV_Target
{
	//return float4(1, 0, 0, 1);
	return vertexIn.Color;
}

[maxvertexcount(1)]
void gs_main(point VSOut vertex[1], inout TriangleStream<VSOut> stream)
{
	stream.RestartStrip();
}