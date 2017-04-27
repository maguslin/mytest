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
RWStructuredBuffer<NvHair_Pin> g_pinBuffer : register(u1);
RWStructuredBuffer<float> g_particleLuminances : register(u2);

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
groupshared NvHair_Pin sharedPins[NV_HAIR_MAX_PINS];

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// apply pin constraint 
///////////////////////////////////////////////////////////////////////////////////////////////////////////
void ApplyPinConstraints(inout float4 particle, inout float luminance, float4 restPosition)
{
	for (int i = 0; i < g_buffer.numPinConstraints; i++)
	{
		NvHair_Pin pin = sharedPins[i];

		float w = NvHair_ComputeWeight(pin, restPosition);

		luminance = -1.0;
		if (pin.selected)
		{
			luminance = NvHair_ComputeWeight(pin, restPosition);
		}

		float3 target = (pin.useDynamicPin) ?
			NvHair_ComputeDynamicTarget(pin, restPosition) :
			NvHair_ComputeStaticTarget(pin, restPosition);
		float3 delta = target.xyz - particle.xyz;
		
		float stiffness = pin.stiffness;
		float influence = NvHair_ComputeInfluence(pin, restPosition);

		if (pin.stiffPin && pin.useDynamicPin) {
			// Stiff pin, ignore global stiffness and influence falls off sharply
			particle.xyz += influence * (pow(w,0.05f) * stiffness) * delta;
		} else {
			// Regular pin
			particle.xyz += influence * (w * particle.w * stiffness) * delta;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
[numthreads(NV_HAIR_BLOCK_SIZE_PIN_COM,1,1)]
void cs_main(uint threadId        : SV_GroupIndex,
             uint3 groupId        : SV_GroupID,
             uint3 globalThreadId : SV_DispatchThreadID)
{
	if (g_buffer.simulate == false)
		return;

	// number of particles in this hair
    uint n = (uint)g_buffer.numTotalCvs; // need total number of cvs

	int vid = globalThreadId.x;

	if (threadId < g_buffer.numPinConstraints)
	{
		sharedPins[threadId] = g_pinBuffer[threadId];
	}
	GroupMemoryBarrierWithGroupSync();

	if (vid < n)
	{
		float4 restPosition	= g_restMasterStrand[vid];

		float4 currentPos = g_particlePositions[vid];
		float currentLuminance = g_particleLuminances[vid];
		ApplyPinConstraints(currentPos, currentLuminance, restPosition);

		g_particlePositions[vid] = currentPos;
		g_particleLuminances[vid] = currentLuminance;
	}
}


