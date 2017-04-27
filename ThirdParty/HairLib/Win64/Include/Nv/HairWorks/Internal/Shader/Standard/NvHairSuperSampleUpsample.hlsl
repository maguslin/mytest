/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#include <Nv/HairWorks/Internal/Shader/NvHairSuperSample.h>

///////////////////////////////////////////////////////////////////////////////////
// Textures
///////////////////////////////////////////////////////////////////////////////////
Texture2DMS<float>	g_depthTextureMs : register(t0);


struct VSOut
{
    float4 pos : SV_Position;
	float2 tex : TEXCOORD;
};

/////////////////////////////////////////////////////////////////////////////////////
// vertex shader
/////////////////////////////////////////////////////////////////////////////////////
VSOut vs_main( uint id : SV_VertexID )
{
    VSOut output;
    output.tex = float2( (id << 1) & 2, id & 2 );
    output.pos = float4( output.tex * float2( 2.0f, -2.0f ) + float2( -1.0f, 1.0f), 0.0f, 1.0f );

    return output;
}
	
/////////////////////////////////////////////////////////////////////////////////////
// pixel shader
/////////////////////////////////////////////////////////////////////////////////////
struct PSOut
{
	float4	color : SV_Target;	// DX12 requires this to be declared even if no render target is bound.
	float	depth : SV_Depth;	// Otherwise, the depth data will not be set correctly
};

PSOut ps_main(VSOut input) 
{   
	PSOut output;

	int2 location = getUserBufferLocation(input.tex);

	if (g_depthComparisonLess)
	{
		output.depth = sampleMinDepth(g_sampleCountUser, location, g_depthTextureMs);
	}
	else
	{
		output.depth = sampleMaxDepth(g_sampleCountUser, location, g_depthTextureMs);
	}

	return output;
}

[maxvertexcount(1)]
void gs_main(point VSOut vertex[1], inout TriangleStream<VSOut> stream)
{
	stream.RestartStrip();
}
