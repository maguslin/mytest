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
RWStructuredBuffer<float4>	g_particlePositions			: register(u0); // particle positions, for the  solver 

Buffer<int>					g_simulationCvOffsets		: register(t0); // start vertex for each hair
Buffer<int>					g_interactionIndices		: register(t1); // connected particle ids
Buffer<int>					g_interactionOffset			: register(t2); // start index to indices array for each particle
Buffer<float>				g_interactionLength			: register(t3); // rest length for each connection
//Buffer<float>				g_particleRadius			: register(t4); 

///////////////////////////////////////////////////////////////////////////////////////////////////////////
//constants that change frame to frame
cbuffer cbPerFrame : register( b0 )
{
	NvHair_SimulateConstantBuffer g_buffer;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// main compute shader for hair simulation
// each group computes position of cvs of a single hair strand.
// So, groupId == hairId
// each thread mostly maps to each cv within the hair
// So, in many cases (but not all), threadId = cv id, where threadId = 0 --> hair root
///////////////////////////////////////////////////////////////////////////////////////////////////////////
[numthreads(NV_HAIR_BLOCK_SIZE_SIMULATE,1,1)]
void cs_main(uint threadId        : SV_GroupIndex,
             uint3 groupId        : SV_GroupID,
             uint3 globalThreadId : SV_DispatchThreadID)
{   
	//the offset into the particle attributes buffer for this particular patch
    int readStart = 0; 
    if(groupId.x > 0) 
		readStart =  g_simulationCvOffsets.Load( groupId - 1 );

	// number of particles in this hair
    int n = g_simulationCvOffsets.Load( groupId ) - readStart;

	// classifiers for thread. Each thread maps to each simulated CV.
	int		vertexId = readStart + threadId;  // global index to hair cv attribute buffers
	bool	firstCv = (threadId == 0);	// is this first (root) cv?
	bool	lastCv  = (threadId == (n-1)); // is this last cv?
	bool	validCv = (threadId < n); // is this thread (cv id) valid?

	// particle positions to read from global memory
    float4		p0 = float4(0,0,0,0);		// current particle position 
	int			interactionStart = 0;
	int			interactionEnd = 0;

	float interactionStiffness = NV_HAIR_LERP_SIMULATION_PARAM(g_buffer, interactionStiffness);

	////////////////////////////////////////////////////////////////////////////////////////////
    // read the particle attributes into registers and shared memory (global mem load)
	////////////////////////////////////////////////////////////////////////////////////////////
    if (validCv)
    {
		p0 = g_particlePositions[vertexId];		
		
		interactionStart = (vertexId == 0) ? 0 : g_interactionOffset[ vertexId - 1 ];
		interactionEnd	= g_interactionOffset[ vertexId ];

		float3 displacement = float3(0,0,0);
		int interactionCnt = interactionEnd - interactionStart;

		for (int i = interactionStart; i < interactionEnd; i++)
		{
			int id				= g_interactionIndices[i];
			float restLength	= g_interactionLength[i];

			float4 p1 = g_particlePositions[id];

			float3 d = p1.xyz - p0.xyz;
			float l = length(d);

			float residual = (l - restLength) / (l + 1e-7f);
			float scale = interactionStiffness * residual;

			displacement += scale * d;
		}

		if (interactionCnt > 0)
			displacement *= 1.0f / (float)(interactionCnt);

		p0.xyz += p0.w * displacement;
		
	}
	
	GroupMemoryBarrierWithGroupSync();

	if (validCv)
	{
		g_particlePositions[vertexId] = p0;
    }             
}