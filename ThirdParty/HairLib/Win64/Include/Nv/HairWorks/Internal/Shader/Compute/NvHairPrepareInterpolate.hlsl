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
RWStructuredBuffer<float4>	g_particlePositions			: register(u0); //particle positions, for the  solver 
RWStructuredBuffer<float4>	g_previousParticlePositions	: register(u1); //the partice positions from the last frame
RWStructuredBuffer<float4>	g_deltaParticlePos			: register(u2); // Delta particle pos

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// shared resource views
Buffer<int>		g_simulationStrandOffsets			: register(t0); 
Buffer<float4>	g_restMasterStrand					: register(t1);
Buffer<float4>	g_boneIndices						: register(t2);
Buffer<float4>	g_boneWeights						: register(t3);

///////////////////////////////////////////////////////////////////////////////////////////////////////////
//constants that change frame to frame
cbuffer cbPerFrame : register( b0 )
{
	NvHair_SimulateConstantBuffer g_buffer;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// apply dual quaternion skinning
float3 computeSkinningDq(uint hairId,  float4 restPosition)
{
	float4 boneIndex  = g_boneIndices.Load(hairId);
	float4 boneWeight = g_boneWeights.Load(hairId);

	DualQuaternion dq;
	dq.q0 = float4(0,0,0,0);
	dq.q1 = float4(0,0,0,0);

	[unroll(4)]
	for (int b = 0; b < 4; b++)
	{
		float w = boneWeight[b];
		DualQuaternion skinDq = g_buffer.skinDqs[boneIndex[b]];

		skinDq.q0 *= w;
		skinDq.q1 *= w;

		// hemispherization
		float sign = (dot(dq.q0, skinDq.q0) < -FLT_EPSILON) ? -1.0f: 1.0f;

		dq.q0 += sign * skinDq.q0;
		dq.q1 += sign * skinDq.q1;
	}

	// normalize
	float mag = dot(dq.q0, dq.q0);
	float deLen = 1.0f / sqrt(mag+FLT_EPSILON);
	dq.q0 *= deLen;
	dq.q1 *= deLen;

	// transform
	float3 d0 = dq.q0.xyz;
	float3 de = dq.q1.xyz;
	float a0 = dq.q0.w;
	float ae = dq.q1.w;

	float3 tempPos		= cross(d0, restPosition.xyz) + a0 * restPosition.xyz;
	float3 tempPos2		= 2.0f * (de * a0 - d0 * ae + cross(d0, de));

	return restPosition.xyz + tempPos2 + cross(2.0f * d0, tempPos);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// apply linear blending based skinning
float3 computeSkinningLinear(uint hairId, float4 restPosition)
{
	float4 boneIndex  = g_boneIndices.Load(hairId);
	float4 boneWeight = g_boneWeights.Load(hairId);

	float weightSum = boneWeight.x + boneWeight.y + boneWeight.z + boneWeight.w ;
	float invWeightSum = 1.0f / (weightSum + FLT_EPSILON);

	float3 skinnedPosition = float3(0,0,0);
	
	[unroll(4)]
	for (int b = 0; b < 4; b++)
	{
		row_major float4x4 skin = g_buffer.skinMatrices[boneIndex[b]];
		float3 p = boneWeight[b] * invWeightSum * (mul(float4(restPosition.xyz,1), skin)).xyz;
		skinnedPosition += p;
	}
	return skinnedPosition;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// shared memory
//groupshared float4 sharedPos[NV_HAIR_BLOCK_SIZE_SIMULATE];				// particle positions
///////////////////////////////////////////////////////////////////////////////////////////////////////////
// main compute shader for hair simulation
// each group computes position of cvs of a single hair strand.
// each thread mostly maps to each cv within the hair
// So, in many cases (but not all), threadId = cv id, where threadId = 0 --> hair root
///////////////////////////////////////////////////////////////////////////////////////////////////////////
[numthreads(NV_HAIR_BLOCK_SIZE_SIMULATE,1,1)]
void cs_main(uint threadId        : SV_GroupIndex,
             uint3 groupId        : SV_GroupID,
             uint3 globalThreadId : SV_DispatchThreadID)
{   
	// each thread group = each hair
	uint hairId = groupId.x;

	// global index for this hair's root
    uint globalRootIndex = (hairId == 0) ? 0 :g_simulationStrandOffsets.Load( hairId - 1 );
	
	// number of particles in this hair
    int n = g_simulationStrandOffsets.Load( hairId ) - globalRootIndex;

	// classifiers for thread. Each thread maps to each simulated Cv.
	int		globalCvIndex = globalRootIndex + threadId;  // global index to per hair cv attribute buffers
	bool	validCv = (threadId < n); // is this thread (cv id) valid?
	
	// we compute the nextPos at frame n+1 using info from current (n) and previous (n-1) frames.
	// prevPos (n-1) -> currentPos (n) -> nextPos(n+1)
    float4		currentPos	= float4(0,0,0,0);		// current particle position 
	float4		prevPos		= float4(0,0,0,0);		// previous particle position stored in previous frame

	float4		restPos		= float4(0,0,0,0);		// rest position in bind pose
	float4		skinnedPos  = float4(0,0,0,0);		// skinned position accounting for bone based skinning

	////////////////////////////////////////////////////////////////////////////////////////////
    // read the particle attributes into registers and shared memory (global mem load)
	////////////////////////////////////////////////////////////////////////////////////////////
    if (validCv)
    {
		prevPos								= g_previousParticlePositions[globalCvIndex];
		currentPos							= g_particlePositions[globalCvIndex];
		restPos								= g_restMasterStrand.Load(globalCvIndex);    
		// restPos.w holds the hair root vertex index
		skinnedPos							= restPos;
    }

	// copy previous root position for inerita computation
	//GroupMemoryBarrierWithGroupSync();

	////////////////////////////////////////////////////////////////////////////////////////////
	// apply GPU skinning
	////////////////////////////////////////////////////////////////////////////////////////////
	if (g_buffer.numBones > 0)
	{
		if (g_buffer.useDualQuaterinon)
			skinnedPos.xyz = computeSkinningDq(hairId, restPos);
		else
			skinnedPos.xyz = computeSkinningLinear(hairId, restPos);
	}

	if (validCv)
	{ 
		float3 interpPos = lerp(prevPos, currentPos, g_buffer.simulationInterp).xyz;
		g_deltaParticlePos[globalCvIndex] = float4(skinnedPos - interpPos, 0); 
	}	
}
