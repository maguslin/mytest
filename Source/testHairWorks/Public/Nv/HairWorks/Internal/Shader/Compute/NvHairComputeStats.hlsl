/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#include <Nv/HairWorks/Internal/Shader/NvHairInternalShaderCommon.h>
#include <Nv/HairWorks/Internal/Shader/NvHairInternalShaderTypes.h>

#define NUM_INTERPOLATED_ATTRIBUTES_MINUS_ONE 1023

// TODO JS: Looks like it matches NV_NW_NUM_HAIRS_PER_FACE, so perhaps can consolidate into single definition.
#define NUM_SAMPLES_PER_FACE 64

// UAVs
RWBuffer<float4> g_stats : register(u0); 

// shared resource views
Buffer<float2>				g_faceTexCoords : register(t0);
Texture2D		            g_densityTexture: register(t1);
Buffer<float>				g_noiseLut : register(t2);
Buffer<float2>				g_strandCoordinatesLut : register(t3);

//constants that change frame to frame
cbuffer cbPerFrame : register( b0 )
{
	NvHair_StatsPerFrameConstantBuffer g_buffer;
}

SamplerState samLinear;

//since the maximum length is 40 the size of the CTA is fine at 64
//the total amount of thread local storage is limited to 32KB
#define BLOCK_SIZE 64

[numthreads(64,1,1)]
void cs_main(uint threadId        : SV_GroupIndex,
             uint3 groupId        : SV_GroupID,
             uint3 globalThreadId : SV_DispatchThreadID)
{

    uint idx = groupId.x * BLOCK_SIZE + threadId;   

    if (idx < (uint)g_buffer.numFaces)
    {
        if (!g_buffer.usePixelDensity)
        {
            float2 uv0 = g_faceTexCoords.Load(idx * 3);
            float2 uv1 = g_faceTexCoords.Load(idx * 3+1);
            float2 uv2 = g_faceTexCoords.Load(idx * 3+2);
    
            float2 uv = 1.0f / 3.0f * (uv0 + uv1 + uv2);

            float density = g_densityTexture.SampleLevel(samLinear, uv, 0).x;
            g_stats[idx] = g_buffer.density * float4(density, 0.0f, 0.0f, 0.0f);
        }
        else
        {
            float2 uv0 = g_faceTexCoords.Load(idx * 3);
            float2 uv1 = g_faceTexCoords.Load(idx * 3+1);
            float2 uv2 = g_faceTexCoords.Load(idx * 3+2);

            float hairCount = 0.0f;

            for (int i = 0; i < NUM_SAMPLES_PER_FACE * g_buffer.density; i++)
            {
                int instanceId = idx * NUM_SAMPLES_PER_FACE + i;
                float3 coords;
                coords.xy = g_strandCoordinatesLut.Load(instanceId & NUM_INTERPOLATED_ATTRIBUTES_MINUS_ONE);
        	    coords.z = (1 - coords.x - coords.y);
            	
                float2 uv = coords.x * uv0.xy + coords.y * uv1.xy + coords.z * uv2.xy;
                float density = g_densityTexture.SampleLevel(samLinear, uv, 0).x;
	            float densitySample = g_noiseLut.Load(instanceId & NUM_INTERPOLATED_ATTRIBUTES_MINUS_ONE);
	            
                if (densitySample < density)
			        hairCount += 1.0f;
            }

            g_stats[idx] = hairCount / NUM_SAMPLES_PER_FACE;
        }
    }
}