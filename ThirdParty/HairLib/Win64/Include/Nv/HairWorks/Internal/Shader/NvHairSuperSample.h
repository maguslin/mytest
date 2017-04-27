/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#ifndef NV_HAIR_SUPER_SAMPLE_H
#define NV_HAIR_SUPER_SAMPLE_H

#ifdef _CPP
#	error "Can only be included in HLSL code"
#endif

///////////////////////////////////////////////////////////////////////////////////
// Constant bufffers
///////////////////////////////////////////////////////////////////////////////////
cbuffer cbPerFrame : register(b0)
{
	float	g_msaaWidth;
	float	g_msaaHeight;
	float	g_msaaTopLeftX;
	float	g_msaaTopLeftY;

	float	g_userWidth;
	float	g_userHeight;
	float	g_userTopLeftX;
	float	g_userTopLeftY;

	int		g_sampleCountUser;
	int		g_sampleCountMsaa;
	int		g_depthComparisonLess;
	int		g_emitPartialFragment;
}

/////////////////////////////////////////////////////////////////////////////////////
int2 getUserBufferLocation(float2 tex)
{
	int ix = g_userTopLeftX + g_userWidth * tex.x;
	int iy = g_userTopLeftY + g_userHeight * tex.y;
	int2 location = int2(ix,iy);

	return location;
}

/////////////////////////////////////////////////////////////////////////////////////
int2 getMsaaBufferLocation(float2 tex)
{
	int ix = g_msaaTopLeftX + g_msaaWidth * tex.x;
	int iy = g_msaaTopLeftY + g_msaaHeight * tex.y;
	int2 location = int2(ix,iy);

	return location;
}

/////////////////////////////////////////////////////////////////////////////////////
inline float sampleMinDepth1(int2 location, Texture2DMS<float,1> tex)
{
	float minDepth = 1.0f;

	minDepth = tex.Load(location, 0).r;

	return minDepth;
}

inline float sampleMinDepth2(int2 location, Texture2DMS<float,2> tex)
{
	float minDepth = 1.0f;

	[unroll]
	for (int i = 0; i < 2; i++)
	{
		float depth = tex.Load(location, i).r;
		minDepth = min(minDepth, depth);
	}

	return minDepth;
}

inline float sampleMinDepth4(int2 location, Texture2DMS<float,4> tex)
{
	float minDepth = 1.0f;

	[unroll]
	for (int i = 0; i < 4; i++)
	{
		float depth = tex.Load(location, i).r;
		minDepth = min(minDepth, depth);
	}

	return minDepth;
}

inline float sampleMinDepth8(int2 location, Texture2DMS<float,8> tex)
{
	float minDepth = 1.0f;

	[unroll]
	for (int i = 0; i < 8; i++)
	{
		float depth = tex.Load(location, i).r;
		minDepth = min(minDepth, depth);
	}

	return minDepth;
}

/////////////////////////////////////////////////////////////////////////////////////
inline float sampleMaxDepth1(int2 location, Texture2DMS<float,1> tex)
{
	float maxDepth = 0.0f;

	maxDepth = tex.Load(location, 0).r;

	return maxDepth;
}

inline float sampleMaxDepth2(int2 location, Texture2DMS<float,2> tex)
{
	float maxDepth = 0.0f;

	[unroll]
	for (int i = 0; i < 2; i++)
	{
		float depth = tex.Load(location, i).r;
		maxDepth = max(maxDepth, depth);
	}

	return maxDepth;
}

inline float sampleMaxDepth4(int2 location, Texture2DMS<float,4> tex)
{
	float maxDepth = 0.0f;

	[unroll]
	for (int i = 0; i < 4; i++)
	{
		float depth = tex.Load(location, i).r;
		maxDepth = max(maxDepth, depth);
	}

	return maxDepth;
}

inline float sampleMaxDepth8(int2 location, Texture2DMS<float,8> tex)
{
	float maxDepth = 0.0f;

	[unroll]
	for (int i = 0; i < 8; i++)
	{
		float depth = tex.Load(location, i).r;
		maxDepth = max(maxDepth, depth);
	}

	return maxDepth;
}

/////////////////////////////////////////////////////////////////////////////////////
inline float sampleMinDepth(int sampleCount, int2 location, Texture2DMS<float> tex)
{
	float minDepth = 1.0f;

	switch (sampleCount)
	{
	case 1:
		minDepth = sampleMinDepth1(location, tex);
		break;

	case 2:
		minDepth = sampleMinDepth2(location ,tex);
		break;

	case 4:
		minDepth = sampleMinDepth4(location, tex);
		break;

	case 8:
		minDepth = sampleMinDepth8(location, tex);
		break;
	}

	return minDepth;
}

/////////////////////////////////////////////////////////////////////////////////////
inline float sampleMaxDepth(int sampleCount, int2 location, Texture2DMS<float> tex)
{
	float maxDepth = 1.0f;

	switch (sampleCount)
	{
	case 1:
		maxDepth = sampleMaxDepth1(location, tex);
		break;

	case 2:
		maxDepth = sampleMaxDepth2(location ,tex);
		break;

	case 4:
		maxDepth = sampleMaxDepth4(location, tex);
		break;

	case 8:
		maxDepth = sampleMaxDepth8(location, tex);
		break;
	}

	return maxDepth;
}


/////////////////////////////////////////////////////////////////////////////////////
inline float4 sampleColor1(int2 location, Texture2DMS<float4> tex)
{
	return tex.Load(location, 0).rgba;
}

inline float4 sampleColor2(int2 location, Texture2DMS<float4> tex)
{
	float4 color = 0;

	[unroll]
	for (int i = 0; i < 2; i++)
	{
		color += tex.Load(location, i).rgba;
	}

	return color / 2.0f;
}

inline float4 sampleColor4(int2 location, Texture2DMS<float4> tex)
{
	float4 color = 0;

	[unroll]
	for (int i = 0; i < 4; i++)
	{
		color += tex.Load(location, i).rgba;
	}

	return color / 4.0f;
}

inline float4 sampleColor8(int2 location, Texture2DMS<float4> tex)
{
	float4 color = 0;

	[unroll]
	for (int i = 0; i < 8; i++)
	{
		color += tex.Load(location, i).rgba;
	}

	return color / 8.0f;
}

/////////////////////////////////////////////////////////////////////////////////////
inline float4 sampleColor(int sampleCount, int2 location, Texture2DMS<float4> tex)
{
	float4 sample = 0;

	switch (sampleCount)
	{
	case 1:
		sample = sampleColor1(location, tex);
		break;

	case 2:
		sample = sampleColor2(location ,tex);
		break;

	case 4:
		sample = sampleColor4(location, tex);
		break;

	case 8:
		sample = sampleColor8(location, tex);
		break;
	}

	return sample;
}

#endif // NV_HAIR_SUPER_SAMPLE_H

