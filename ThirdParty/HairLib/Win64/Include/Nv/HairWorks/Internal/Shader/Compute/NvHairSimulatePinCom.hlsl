/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */
#include <Nv/HairWorks/Internal/Shader/NvHairSimulate.h>
#include <Nv/HairWorks/Internal/Shader/NvHairShaderMath.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// UAVs
RWStructuredBuffer<float4>	g_particlePositions	: register(u0); // particle positions, for the  solver 
RWStructuredBuffer<NvHair_PinScratchData>	g_globalScratchMem : register(u1); // global scratch mem
RWStructuredBuffer<NvHair_Pin> g_pinBuffer : register(u2);
RWStructuredBuffer<float4>	g_masterStrandTangents: register(u3);

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// shared resource views
Buffer<float4>	g_restMasterStrand				: register(t0);

///////////////////////////////////////////////////////////////////////////////////////////////////////////
//constants that change frame to frame
cbuffer cbPerFrame : register( b0 )
{
	NvHair_SimulateConstantBuffer g_buffer;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
groupshared float4 sharedPos[NV_HAIR_BLOCK_SIZE_SIMULATE];

///////////////////////////////////////////////////////////////////////////////////////////////////////////
[numthreads(NV_HAIR_BLOCK_SIZE_PIN_COM,1,1)]
void cs_main(uint threadId        : SV_GroupIndex,
             uint3 groupId        : SV_GroupID,
             uint3 globalThreadId : SV_DispatchThreadID)
{
	// number of particles in this hair
    uint n = (uint)g_buffer.numTotalCvs; // need total number of cvs
	int numPins = g_buffer.numPinConstraints;
	int pinId = threadId;

    // read positions and weights to shared mem
	sharedPos[threadId] = float4(0,0,0,0);

	float4 com = 0;
	float4 tangentSum = 0;

	// TODO average tangent and fix shared mem version
	if (pinId < numPins)
	{
		NvHair_Pin pin = g_pinBuffer[pinId];
		if(!pin.useDynamicPin)
			return;

		for (uint i = 0; i < NV_HAIR_BLOCK_SIZE_PIN_COM; i++)
		{
			if (globalThreadId.x < n)
			{
				int vid = groupId.x * NV_HAIR_BLOCK_SIZE_PIN_COM + i;
				float4 currentPos = g_particlePositions[vid];
				float4 restPosition = g_restMasterStrand[vid];
				float w = NvHair_ComputeWeight(pin, restPosition);

				currentPos.xyz *= w;
				currentPos.w = w;
				com += currentPos;

				float4 tangent = g_masterStrandTangents[vid];
				tangentSum.xyz += w * tangent.xyz;
			}
		}

		if (groupId.x < NV_HAIR_SCRATCH_SIZE_PER_PIN)
		{
			NvHair_PinScratchData p;

			p.com = com;
			p.tangent = tangentSum;

			g_globalScratchMem[pinId * NV_HAIR_SCRATCH_SIZE_PER_PIN + groupId.x] = p;
		}
	}

#if 0
	if (globalThreadId.x < n)
	{
		float4 currentPos = g_particlePositions[globalThreadId.x];
		float4 restPosition	= g_restMasterStrand[globalThreadId.x];
		float w = ComputePinWeight(0, restPosition);

		currentPos.xyz *= w;
		currentPos.w = w;

		sharedPos[threadId] = currentPos;

		restPosition.xyz *= w;
		restPosition.w = w;

		sharedRestPos[threadId] = restPosition;
	}
	GroupMemoryBarrierWithGroupSync();


	// reduction to add all the data in shared mem
	// final sum is stored in sharedPos[0]
	uint taskSize = BLOCK_SIZE_PIN_COM / 2; 
	for ( ;  taskSize >= 1; taskSize /= 2)
	{
		float4 sum = 0;
		float4 restsum = 0;

		if (threadId < taskSize)
		{
			sum = sharedPos[threadId * 2] + sharedPos[threadId * 2 + 1];
			restsum = sharedRestPos[threadId * 2] + sharedRestPos[threadId * 2 + 1];
		}
		GroupMemoryBarrierWithGroupSync();

		if (threadId < taskSize)
		{
			sharedPos[threadId] = sum;
			sharedRestPos[threadId] = restsum;
		}
		GroupMemoryBarrierWithGroupSync();
	}

	// write block sum to global scratch mem
	uint numBlocks = n / BLOCK_SIZE_PIN_COM + 1;

	numBlocks = 6;

	if ( (threadId == 0) && (groupId.x < numBlocks))
	{
		g_globalScratchMem[groupId.x] = sharedPos[0];
		g_globalScratchMem[2047] = sharedRestPos[0];
	}
#endif
}


